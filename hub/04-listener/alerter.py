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
from datetime import datetime
from zoneinfo import ZoneInfo

log = logging.getLogger("plant-listener.alerter")

NTFY_TOPIC = os.environ.get("NTFY_TOPIC")
NTFY_SERVER = os.environ.get("NTFY_SERVER", "https://ntfy.sh").rstrip("/")
OFFLINE_GRACE_S = int(os.environ.get("ALERT_OFFLINE_GRACE_S", "300"))
HEARTBEAT_HOUR = int(os.environ.get("HEARTBEAT_HOUR", "9"))
LOCAL_TZ = ZoneInfo(os.environ.get("LOCAL_TZ", "America/Chicago"))

# Latched FSM states worth a push, with (title, body, priority, tags).
FAULT_ALERTS = {
    "leak_fault": (
        "Leak detected",
        "Water detected where it shouldn't be \u2014 pump stopped. Needs attention and ACK.",
        "high", ["rotating_light"]),
    "watering_fault": (
        "Watering fault",
        "Soil isn't responding to watering \u2014 pump stopped (empty reservoir, clog, or pump issue). Needs ACK.",
        "high", ["warning"]),
    "reservoir_empty": (
        "Reservoir empty",
        "The water reservoir is empty \u2014 refill needed.",
        "default", ["droplet"]),
}

# In-memory alert state (resets on listener restart, which is acceptable).
_st = {
    "fsm_alerted": None,        # which fault state we've notified about
    "offline_since": None,      # monotonic time wrover first seen offline
    "offline_alerted": False,
    "reboot_flap_alerted": False,
    "last_heartbeat_date": None,
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
        "AND metric='reboot' AND ts >= datetime('now','-24 hours')") or 0


def _daily_ml(conn):
    ms = _scalar(conn,
        "SELECT value FROM system_status WHERE device='wrover' "
        "AND metric='fsm_state' ORDER BY id DESC LIMIT 1")
    return (ms or 0) / 1000.0


def _soil_pct(conn):
    return _scalar(conn,
        "SELECT value FROM sensor_readings WHERE sensor='moisture' "
        "AND device='soil' ORDER BY id DESC LIMIT 1")


# ---- the periodic evaluation ----------------------------------------------

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
            notify("Resolved", f"{prev_title} cleared \u2014 back to normal.",
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
                   f"The WROVER has been offline for over {mins} min \u2014 the plant "
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
                   f"The WROVER has rebooted {flaps} times in 24h \u2014 check power "
                   f"stability (brownout when the pump runs?).", "high",
                   ["electric_plug", "warning"])
            _st["reboot_flap_alerted"] = True
    else:
        _st["reboot_flap_alerted"] = False

    # 4) Daily heartbeat summary at the configured local hour.
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
