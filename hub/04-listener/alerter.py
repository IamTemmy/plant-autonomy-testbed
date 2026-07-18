"""Push-notification alerter for the Plant Autonomy Testbed (DL-061).

Sends ntfy push notifications for genuine problems only, driven by polling the
state already recorded in `system_status`. Design goals:

- Edge-triggered + debounced: one alert per transition, never repeated while a
  condition persists (the firmware re-publishes the same state every 30 s).
- Non-blocking: the HTTP POST runs on a short-timeout daemon thread so a slow
  network call never stalls message ingestion.
- Safe-by-default: if NTFY_TOPIC is unset, every call is a silent no-op, so the
  listener runs identically whether or not alerting is configured.

Configuration (environment, via the systemd EnvironmentFile — never the repo):
    NTFY_TOPIC              required to enable alerts; the secret topic name
    NTFY_SERVER             default https://ntfy.sh
    ALERT_OFFLINE_GRACE_S   default 300 (5 min) before an offline alert fires
    HEARTBEAT_HOUR          default 9   (local hour for the daily summary)
    LOCAL_TZ                default America/Chicago
"""

import logging
import os
import threading
import time
import urllib.request
from datetime import datetime, timezone
from zoneinfo import ZoneInfo

log = logging.getLogger("plant-listener.alerter")

NTFY_TOPIC = os.environ.get("NTFY_TOPIC")
NTFY_SERVER = os.environ.get("NTFY_SERVER", "https://ntfy.sh").rstrip("/")
OFFLINE_GRACE_S = int(os.environ.get("ALERT_OFFLINE_GRACE_S", "300"))
HEARTBEAT_HOUR = int(os.environ.get("HEARTBEAT_HOUR", "9"))
LOCAL_TZ = ZoneInfo(os.environ.get("LOCAL_TZ", "America/Chicago"))

# Grow-light photoperiod verification (DL-063): cross-check measured lux against
# the Shelly's schedule. Threshold set from measured data in the current geometry:
# room-lights-max ~18 lux vs grow light ~44-47 lux at the sensor -> a 30 lux line
# splits the gap with ~12 lux margin each side. Env-overridable; retune from data.
GROW_ON_HOUR = int(os.environ.get("GROW_ON_HOUR", "7"))
GROW_OFF_HOUR = int(os.environ.get("GROW_OFF_HOUR", "19"))
GROW_LUX_THRESHOLD = float(os.environ.get("GROW_LUX_THRESHOLD", "30"))
GROW_MISMATCH_GRACE_S = int(os.environ.get("GROW_MISMATCH_GRACE_S", "900"))  # 15 min
GROW_LUX_STALE_S = int(os.environ.get("GROW_LUX_STALE_S", "300"))  # can't verify if older

# External (manual) watering detection (DL-064): flag a soil-moisture rise the
# system didn't cause. A rise >= SOIL_RISE_THRESHOLD over SOIL_LOOKBACK_MIN, with
# no system watering in the lookback + settling window, means someone else watered.
SOIL_RISE_THRESHOLD = float(os.environ.get("SOIL_RISE_THRESHOLD", "15"))  # percent
SOIL_LOOKBACK_MIN = int(os.environ.get("SOIL_LOOKBACK_MIN", "30"))
SOIL_SETTLING_MIN = int(os.environ.get("SOIL_SETTLING_MIN", "30"))
_SOIL_RECENT_MIN = 5   # "now" = average over the last this-many minutes
_SOIL_BASE_HALF = 5    # baseline window = [LOOKBACK-HALF, LOOKBACK+HALF] min ago

# Silent-camera detection (DL-088): the camera node POSTs an hourly capture that the
# receiver records in camera_readings. If no fresh row arrives during the lit window the
# imaging pipeline is down (node power/WiFi, or the receiver service) -- a silent failure
# nothing else catches. Reuses the GROW_ON_HOUR/GROW_OFF_HOUR photoperiod window.
CAMERA_STALE_S = int(os.environ.get("CAMERA_STALE_S", "7200"))  # 2h; tolerates one missed capture

# Latched FSM states worth a push, with (title, body, priority, tags).
FAULT_ALERTS = {
    "leak_fault": (
        "Leak detected",
        "Water detected where it shouldn't be — pump stopped. Needs attention and ACK.",
        "high", ["rotating_light"]),
    "watering_fault": (
        "Watering fault",
        "Soil isn't responding to watering — pump stopped (empty reservoir, clog, or pump issue). Needs ACK.",
        "high", ["warning"]),
    "reservoir_empty": (
        "Reservoir empty",
        "The water reservoir is empty — refill needed.",
        "default", ["droplet"]),
}

# In-memory alert state (resets on listener restart, which is acceptable).
_st = {
    "fsm_alerted": None,        # which fault state we've notified about
    "offline_since": None,      # monotonic time wrover first seen offline
    "offline_alerted": False,
    "reboot_flap_alerted": False,
    "last_heartbeat_date": None,
    "gl_mismatch_since": None,   # monotonic time the current grow-light mismatch began
    "gl_mismatch_kind": None,    # "dark_during_on" | "lit_during_off"
    "gl_alerted": None,          # mismatch kind we've alerted on (for recovery)
    "ext_water_alerted": False,  # external-watering event currently alerted (debounce)
    "cam_window_since": None,    # monotonic time we entered the lit window (re-armed daily)
    "cam_alerted": False,        # silent-camera alert currently firing (for recovery)
    "water_last_reason": None,   # last bottom-watering reason we alerted on (edge-trigger)
}


# ---- notification primitive ------------------------------------------------

def _post(title, message, priority, tags):
    url = f"{NTFY_SERVER}/{NTFY_TOPIC}"
    headers = {"Title": title, "Priority": priority}
    if tags:
        headers["Tags"] = ",".join(tags)
    req = urllib.request.Request(
        url, data=message.encode("utf-8"), headers=headers, method="POST")
    try:
        urllib.request.urlopen(req, timeout=5)
        log.info("ntfy sent: %s", title)
    except Exception as e:  # never let a notification failure surface
        log.warning("ntfy POST failed (%s): %s", title, e)


def notify(title, message, priority="default", tags=None):
    """Fire a push notification without blocking the caller. No-op if unconfigured."""
    if not NTFY_TOPIC:
        return
    threading.Thread(
        target=_post, args=(title, message, priority, tags or []), daemon=True
    ).start()


# ---- bottom-watering loop alerts (event-driven, DL-109) --------------------
# The bottom-watering harness publishes plant/state/wrover with a `reason` field
# describing session outcomes; the integrated firmware sends no reason, so only
# the harness can trigger these. Keyed on the exact reason strings the firmware
# emits. Routine states (monitor/dosing/settle/grace) carry no alert-worthy
# reason and are ignored. `abort` is user-initiated, so it is intentionally silent.
_WATERING_ALERTS = {
    "stalled: tray may be holding water": (
        "Watering stalled",
        "A dose didn't raise soil moisture — the tray may still be holding water. "
        "A one-time grace re-check is running.", "high", ["warning"]),
    "failed: not absorbing": (
        "Watering failed",
        "Soil isn't absorbing after the grace period. Watering stopped — needs a look.",
        "high", ["rotating_light"]),
    "capped: target not reached": (
        "Watering capped",
        "Hit the session volume cap before reaching target. Watering stopped.",
        "high", ["warning"]),
    "reservoir empty": (
        "Reservoir empty",
        "Watering blocked — the reservoir float reads empty. Refill to resume.",
        "high", ["warning"]),
    "leak": (
        "Leak detected",
        "Leak sensor tripped — pump cut and latched. Check for spilled water.",
        "urgent", ["rotating_light"]),
    "target reached": (
        "Watering complete",
        "Soil reached target — watering session done.", "default", ["white_check_mark"]),
}


def on_watering_state(state, reason):
    """Event-driven: called by the listener on each plant/state/wrover message.
    Fires one push per new alert-worthy reason (edge-triggered to absorb retained
    or repeated messages). No-op if alerting is unconfigured or the reason is not
    alert-worthy. In-memory dedup, so a listener restart may re-fire a still-latched
    reason once — acceptable."""
    if not NTFY_TOPIC:
        return
    if reason == _st["water_last_reason"]:
        return
    _st["water_last_reason"] = reason
    alert = _WATERING_ALERTS.get(reason or "")
    if alert:
        notify(*alert)


# ---- small read helpers ----------------------------------------------------

def _scalar(conn, sql, params=()):
    row = conn.execute(sql, params).fetchone()
    return row[0] if row else None


def _latest_fsm(conn):
    return _scalar(conn,
        "SELECT status FROM system_status WHERE device='wrover' "
        "AND metric='fsm_state' ORDER BY id DESC LIMIT 1")


def _presence(conn):
    return _scalar(conn,
        "SELECT status FROM system_status WHERE device='wrover' "
        "AND metric IS NULL ORDER BY id DESC LIMIT 1") or "unknown"


def _reboots_24h(conn):
    return _scalar(conn,
        "SELECT COUNT(*) FROM system_status WHERE device='wrover' "
        "AND metric='reboot' AND ts >= strftime('%Y-%m-%dT%H:%M:%SZ','now','-24 hours')") or 0


def _daily_ml(conn):
    ms = _scalar(conn,
        "SELECT value FROM system_status WHERE device='wrover' "
        "AND metric='fsm_state' ORDER BY id DESC LIMIT 1")
    return (ms or 0) / 1000.0


def _soil_pct(conn):
    return _scalar(conn,
        "SELECT value FROM sensor_readings WHERE sensor='moisture' "
        "AND device='soil' ORDER BY id DESC LIMIT 1")


def _latest_lux(conn):
    """Most recent lux reading as (value, age_seconds), or (None, None)."""
    row = conn.execute(
        "SELECT value, ts FROM sensor_readings WHERE sensor='lux' "
        "ORDER BY id DESC LIMIT 1").fetchone()
    if not row or row[0] is None:
        return (None, None)
    try:
        t = datetime.fromisoformat(row[1].replace("Z", "+00:00"))
        age = (datetime.now(timezone.utc) - t).total_seconds()
    except Exception:
        age = None
    return (row[0], age)


def _latest_camera_age_s(conn):
    """Seconds since the most recent camera_readings row, or None if there are none."""
    ts = _scalar(conn, "SELECT MAX(ts) FROM camera_readings")
    if not ts:
        return None
    try:
        dt = datetime.fromisoformat(ts.replace("Z", "+00:00"))
    except ValueError:
        return None
    if dt.tzinfo is None:
        dt = dt.replace(tzinfo=timezone.utc)
    return (datetime.now(timezone.utc) - dt).total_seconds()


def _check_grow_light(conn, now_mono):
    """Alert if measured lux disagrees with the photoperiod schedule for long enough."""
    # Wrap-aware photoperiod window, consistent with photoperiod.py and
    # image_receiver.py (handles an overnight window where ON > OFF).
    hour = datetime.now(LOCAL_TZ).hour
    if GROW_ON_HOUR < GROW_OFF_HOUR:
        expected_on = GROW_ON_HOUR <= hour < GROW_OFF_HOUR
    else:
        expected_on = hour >= GROW_ON_HOUR or hour < GROW_OFF_HOUR
    lux, age = _latest_lux(conn)

    # No fresh lux (e.g. controller offline) -> can't verify; stand down quietly.
    if lux is None or age is None or age > GROW_LUX_STALE_S:
        _st["gl_mismatch_since"] = None
        _st["gl_mismatch_kind"] = None
        return

    lit = lux > GROW_LUX_THRESHOLD
    if expected_on and not lit:
        kind = "dark_during_on"
    elif not expected_on and lit:
        kind = "lit_during_off"
    else:
        kind = None  # matches the schedule

    if kind is None:
        if _st["gl_alerted"] is not None:
            notify("Grow light back to normal",
                   "The grow light matches its photoperiod schedule again.",
                   "default", ["white_check_mark"])
            _st["gl_alerted"] = None
        _st["gl_mismatch_since"] = None
        _st["gl_mismatch_kind"] = None
        return

    # A mismatch is present: track how long it has persisted.
    if _st["gl_mismatch_kind"] != kind:
        _st["gl_mismatch_kind"] = kind
        _st["gl_mismatch_since"] = now_mono

    if (now_mono - _st["gl_mismatch_since"] >= GROW_MISMATCH_GRACE_S
            and _st["gl_alerted"] != kind):
        if kind == "dark_during_on":
            notify("Grow light may be off",
                   f"It's the daytime photoperiod but light at the plant is only "
                   f"{lux:.0f} lux (below {GROW_LUX_THRESHOLD:.0f}). The grow light may "
                   f"have failed to come on — check the plug, schedule, or bulb.",
                   "default", ["bulb"])
        else:
            notify("Grow light still on",
                   f"It's outside the photoperiod but light at the plant is {lux:.0f} "
                   f"lux — the grow light may be stuck on.",
                   "default", ["bulb"])
        _st["gl_alerted"] = kind


# The DB stores UTC ISO ("...T...Z"). Build the window cutoff in that same format so
# the filter stays a plain `ts >= <const>`, which the (sensor, ts) / (device, ts)
# indexes can range-scan (sargable) — wrapping ts in a function would defeat them.
_TS_CUTOFF = "strftime('%Y-%m-%dT%H:%M:%SZ','now', ?)"


def _pump_ran_recently(conn, minutes):
    """True if the pump was commanded on within the last `minutes`. This is the
    direct actuator signal (water actually dispensed), so an aborted watering that
    never ran the pump correctly does NOT suppress external-watering detection."""
    n = conn.execute(
        "SELECT COUNT(*) FROM system_status WHERE metric='pump' AND status='on' "
        f"AND ts >= {_TS_CUTOFF}",
        (f"-{minutes} minutes",)).fetchone()[0]
    return bool(n)


def _soil_avg(conn, older_min, newer_min):
    """Average soil moisture for readings between older_min and newer_min ago
    (newer_min=None means up to the present)."""
    sql = ("SELECT AVG(value) FROM sensor_readings WHERE sensor='moisture' "
           f"AND ts >= {_TS_CUTOFF}")
    params = [f"-{older_min} minutes"]
    if newer_min is not None:
        sql += f" AND ts < {_TS_CUTOFF}"
        params.append(f"-{newer_min} minutes")
    return conn.execute(sql, params).fetchone()[0]


def _check_external_watering(conn):
    """Alert on a significant soil rise the system didn't cause."""
    # Suppress while the pump's own dispensation (and its settling climb) is in range.
    if _pump_ran_recently(conn, SOIL_LOOKBACK_MIN + SOIL_SETTLING_MIN):
        _st["ext_water_alerted"] = False
        return

    recent = _soil_avg(conn, _SOIL_RECENT_MIN, None)
    baseline = _soil_avg(conn, SOIL_LOOKBACK_MIN + _SOIL_BASE_HALF,
                         SOIL_LOOKBACK_MIN - _SOIL_BASE_HALF)
    if recent is None or baseline is None:
        _st["ext_water_alerted"] = False   # not enough data to judge
        return

    rise = recent - baseline
    if rise >= SOIL_RISE_THRESHOLD:
        if not _st["ext_water_alerted"]:
            notify("Unexplained watering",
                   f"Soil moisture rose {rise:.0f}% in ~{SOIL_LOOKBACK_MIN} min with no "
                   f"system watering — did someone water the plant?",
                   "default", ["sweat_drops"])
            _st["ext_water_alerted"] = True
    else:
        _st["ext_water_alerted"] = False   # settled back; re-arm


# ---- the periodic evaluation ----------------------------------------------

def _check_camera(conn, now_mono):
    """Alert if no camera image arrives during the lit window (silent node or receiver)."""
    hour = datetime.now(LOCAL_TZ).hour
    if GROW_ON_HOUR < GROW_OFF_HOUR:
        in_window = GROW_ON_HOUR <= hour < GROW_OFF_HOUR
    else:
        in_window = hour >= GROW_ON_HOUR or hour < GROW_OFF_HOUR

    # Outside the lit window the camera is meant to be silent: stand down and re-arm.
    if not in_window:
        _st["cam_window_since"] = None
        _st["cam_alerted"] = False
        return

    # On entering the window (or after a restart) start the clock, and hold off judging
    # until a capture could plausibly have arrived -- so dawn, when the newest row is
    # still yesterday's, does not false-alarm.
    if _st["cam_window_since"] is None:
        _st["cam_window_since"] = now_mono
    if now_mono - _st["cam_window_since"] < CAMERA_STALE_S:
        return

    age = _latest_camera_age_s(conn)
    silent = age is None or age > CAMERA_STALE_S

    if silent:
        if not _st["cam_alerted"]:
            notify("Camera may be down",
                   f"No camera image in over {CAMERA_STALE_S // 3600}h during the lit "
                   f"window. Check the camera node (power/WiFi) and the image-receiver "
                   f"service.", "default", ["camera"])
            _st["cam_alerted"] = True
    elif _st["cam_alerted"]:
        notify("Camera back", "Camera images are arriving again.",
               "default", ["white_check_mark"])
        _st["cam_alerted"] = False


def evaluate(conn):
    """Poll current state and fire any newly-warranted alerts. Called on a timer."""
    if not NTFY_TOPIC:
        return  # alerting disabled; skip all work

    now_mono = time.monotonic()

    # 1) Latched FSM faults (leak / watering / reservoir) + recovery.
    fsm = _latest_fsm(conn)
    if fsm in FAULT_ALERTS:
        if _st["fsm_alerted"] != fsm:
            title, body, prio, tags = FAULT_ALERTS[fsm]
            notify(title, body, prio, tags)
            _st["fsm_alerted"] = fsm
    else:
        if _st["fsm_alerted"] is not None:
            prev_title = FAULT_ALERTS[_st["fsm_alerted"]][0]
            notify("Resolved", f"{prev_title} cleared — back to normal.",
                   "default", ["white_check_mark"])
            _st["fsm_alerted"] = None

    # 2) Device offline, after a grace period, + recovery.
    if _presence(conn) == "offline":
        if _st["offline_since"] is None:
            _st["offline_since"] = now_mono
        elif (not _st["offline_alerted"]
              and now_mono - _st["offline_since"] >= OFFLINE_GRACE_S):
            mins = int(OFFLINE_GRACE_S / 60)
            notify("Controller offline",
                   f"The WROVER has been offline for over {mins} min — the plant "
                   f"is currently unmanaged.", "default", ["red_circle"])
            _st["offline_alerted"] = True
    else:
        if _st["offline_alerted"]:
            notify("Controller back online",
                   "The WROVER has reconnected and is managing the plant again.",
                   "default", ["white_check_mark"])
        _st["offline_since"] = None
        _st["offline_alerted"] = False

    # 3) Flapping reboots (>= 2 in 24h). One alert until it settles.
    flaps = _reboots_24h(conn)
    if flaps >= 2:
        if not _st["reboot_flap_alerted"]:
            notify("Controller rebooting repeatedly",
                   f"The WROVER has rebooted {flaps} times in 24h — check power "
                   f"stability (brownout when the pump runs?).", "high",
                   ["electric_plug", "warning"])
            _st["reboot_flap_alerted"] = True
    else:
        _st["reboot_flap_alerted"] = False

    # 4) Grow-light photoperiod verification.
    _check_grow_light(conn, now_mono)

    # 5) Silent-camera detection (no image arriving during the lit window).
    _check_camera(conn, now_mono)

    # 6) External (manual) watering detection.
    _check_external_watering(conn)

    # 7) Daily heartbeat summary at the configured local hour.
    now_local = datetime.now(LOCAL_TZ)
    today = now_local.date()
    if now_local.hour >= HEARTBEAT_HOUR and _st["last_heartbeat_date"] != today:
        soil = _soil_pct(conn)
        soil_s = f"{soil:.0f}%" if soil is not None else "n/a"
        state = _latest_fsm(conn) or "unknown"
        body = (f"Soil {soil_s} \u00b7 state {state} \u00b7 ~{_daily_ml(conn):.0f} mL "
                f"watered today \u00b7 {_reboots_24h(conn)} reboot(s)/24h.")
        notify("Daily plant summary", body, "low", ["seedling"])
        _st["last_heartbeat_date"] = today
