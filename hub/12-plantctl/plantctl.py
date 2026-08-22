#!/usr/bin/env python3
"""
plantctl — read-only diagnostics CLI for the Plant Autonomy Testbed hub.

Encodes the operational runbook we run by hand when something looks off, so a
check is one command instead of a page of pasted SQL. STRICTLY READ-ONLY: it
queries the DB, inspects systemd, and (only where noted) does a single HTTP GET
to the Shelly. It never writes to the DB, never actuates the pump, never changes
firmware or service state. Safe to run anytime.

Timezone discipline (the thing that bit us repeatedly): the DB stores UTC ISO
("...T...Z"). All "age" math is done in UTC; only *display* is converted to
LOCAL_TZ. Never compare a naive local time against a stored UTC timestamp.

Soil % uses the SAME anchors as the firmware. Keep ANCHORS below in sync with
firmware config on each recalibration (or override via env). This is the single
place plantctl reads them; when the shared-config file lands (planned), point
_anchors() at it instead.

Usage:
    plantctl health        full hub sweep (services, WROVER, DB, soil, light, pump)
    plantctl soil          soil-moisture trend (raw + %, days-to-trigger)
    (more commands added incrementally)

Env overrides (defaults match firmware / services):
    PLANT_DB, LOCAL_TZ, SOIL_RAW_DRY, SOIL_RAW_WET, TRIGGER_PCT,
    GROW_ON_HOUR, GROW_OFF_HOUR, GROW_LUX_THRESHOLD, SHELLY_HOST
"""
import argparse
import os
import sqlite3
import sys
from datetime import datetime, timezone, timedelta
from zoneinfo import ZoneInfo

DB_PATH = os.environ.get("PLANT_DB", "/home/basilpi/plant-hub/plant.db")
LOCAL_TZ = ZoneInfo(os.environ.get("LOCAL_TZ", "America/Chicago"))
ISO = "%Y-%m-%dT%H:%M:%SZ"  # stored ts format (UTC)

# --- calibration (keep in sync with firmware config.h on each recalibration) ---
SOIL_RAW_DRY = int(os.environ.get("SOIL_RAW_DRY", "2585"))   # 0%
SOIL_RAW_WET = int(os.environ.get("SOIL_RAW_WET", "1700"))   # 100% (DL-121)
TRIGGER_PCT  = float(os.environ.get("TRIGGER_PCT", "20"))    # F6 (DL-166): match firmware config.h TRIGGER_PCT=20; was 30 (stale) — cried wolf across the normal 20-40% band

GROW_ON_HOUR = int(os.environ.get("GROW_ON_HOUR", "7"))
GROW_OFF_HOUR = int(os.environ.get("GROW_OFF_HOUR", "19"))
GROW_LUX_THRESHOLD = float(os.environ.get("GROW_LUX_THRESHOLD", "30"))
SHELLY_HOST = os.environ.get("SHELLY_HOST", "10.6.17.32")

# Freshness expectations (seconds) — how old a reading may be before "stale".
WROVER_HEARTBEAT_STALE_S = 300     # heartbeats ~ every 5 s; 5 min = offline
SENSOR_STALE_S = 600               # WROVER sensors publish frequently
SERVICES = ["plant-listener", "plant-dashboard",
            "plant-photoperiod.timer", "plant-shelly-monitor.timer"]

# ---- output helpers ----------------------------------------------------------
OK, WARN, FAIL, INFO = "\u2713", "\u26a0", "\u2717", "\u00b7"

def _c(mark, label, detail=""):
    print(f"  {mark} {label}" + (f"  {detail}" if detail else ""))

def _hdr(title):
    print(f"\n{title}")

# ---- data access (read-only) -------------------------------------------------
def _db():
    # read-only connection; fails clearly if the DB is missing
    if not os.path.exists(DB_PATH):
        print(f"{FAIL} DB not found: {DB_PATH}", file=sys.stderr)
        sys.exit(2)
    conn = sqlite3.connect(f"file:{DB_PATH}?mode=ro", uri=True, timeout=10)
    conn.execute("PRAGMA query_only = ON")
    return conn

def _now_utc():
    return datetime.now(timezone.utc)

def _age_s(ts_str):
    """Seconds between a stored UTC ts and now. None if unparseable."""
    if not ts_str:
        return None
    try:
        t = datetime.strptime(ts_str, ISO).replace(tzinfo=timezone.utc)
    except ValueError:
        try:
            t = datetime.fromisoformat(ts_str.replace("Z", "+00:00"))
        except ValueError:
            return None
    return (_now_utc() - t).total_seconds()

def _fmt_age(sec):
    if sec is None:
        return "unknown"
    sec = int(sec)
    if sec < 90:
        return f"{sec}s ago"
    if sec < 5400:
        return f"{sec // 60}m ago"
    if sec < 172800:
        return f"{sec // 3600}h ago"
    return f"{sec // 86400}d ago"

def _local(ts_str):
    a = _age_s(ts_str)
    if a is None:
        return ts_str or "?"
    t = _now_utc() - timedelta(seconds=a)
    return t.astimezone(LOCAL_TZ).strftime("%Y-%m-%d %H:%M %Z")

def _latest(conn, table, where, params=()):
    # F12 (DL-177): order by id, not ts. ts is 1-second-resolution text, so rows can
    # share a ts; ORDER BY ts then picks an arbitrary member of the tie. id is a
    # unique monotonic rowid, so ORDER BY id DESC always returns the true latest row.
    row = conn.execute(
        f"SELECT ts, value FROM {table} WHERE {where} ORDER BY id DESC LIMIT 1",
        params).fetchone()
    return row  # (ts, value) or None

def _soil_pct(raw):
    span = SOIL_RAW_DRY - SOIL_RAW_WET
    pct = (SOIL_RAW_DRY - raw) / span * 100.0
    return max(0.0, min(100.0, pct))

# ---- checks ------------------------------------------------------------------
def _check_services():
    _hdr("Services")
    import subprocess
    for svc in SERVICES:
        try:
            r = subprocess.run(["systemctl", "is-active", svc],
                               capture_output=True, text=True, timeout=5)
            state = r.stdout.strip()
        except Exception as e:
            _c(FAIL, svc, f"check failed: {e}"); continue
        # oneshot timers report 'active' when armed; their .service is inactive between runs
        if state == "active":
            _c(OK, svc, state)
        elif svc.endswith(".timer") and state in ("active", "waiting"):
            _c(OK, svc, state)
        else:
            _c(FAIL, svc, state or "inactive")

def _check_wrover(conn):
    _hdr("WROVER")
    row = _latest(conn, "system_status", "device='wrover' AND metric='fsm_state'")
    st = conn.execute(
        "SELECT status, ts FROM system_status WHERE device='wrover' AND metric='fsm_state' "
        "ORDER BY id DESC LIMIT 1").fetchone()
    hb = _latest(conn, "system_status", "device='wrover' AND status='online'")
    hb_age = _age_s(hb[0]) if hb else None
    if hb_age is None:
        _c(WARN, "heartbeat", "no online record found")
    elif hb_age > WROVER_HEARTBEAT_STALE_S:
        _c(FAIL, "heartbeat", f"last seen {_fmt_age(hb_age)} — WROVER may be offline / unheard")
    else:
        _c(OK, "heartbeat", f"online, {_fmt_age(hb_age)}")
    if st:
        _c(INFO, "fsm state", f"{st[0]} (as of {_local(st[1])})")

def _check_pump(conn):
    _hdr("Pump / watering")
    st = conn.execute(
        "SELECT status, ts FROM system_status WHERE device='wrover' AND metric='fsm_state' "
        "ORDER BY id DESC LIMIT 1").fetchone()
    fsm = st[0] if st else "unknown"
    # Maintenance is a separate flag (DL-128/DL-133), not a state string — read the metric.
    mrow = conn.execute(
        "SELECT value FROM system_status WHERE device='wrover' AND metric='maintenance' "
        "ORDER BY id DESC LIMIT 1").fetchone()
    maint = bool(mrow[0]) if mrow and mrow[0] is not None else False
    if maint:
        _c(OK, "mode", f"maintenance — auto-watering disabled (state: {fsm})")
    else:
        _c(INFO, "mode", f"armed (state: {fsm})")
    on = conn.execute(
        "SELECT ts FROM system_status WHERE device='wrover' AND metric='pump' AND value>0 "
        "ORDER BY id DESC LIMIT 1").fetchone()
    if on:
        _c(INFO, "last pump-on", _local(on[0]))
    else:
        _c(INFO, "last pump-on", "none on record")

def _check_soil(conn):
    _hdr("Soil moisture")
    row = _latest(conn, "sensor_readings", "sensor='soil_raw' AND device='soil'")
    if not row:
        _c(WARN, "soil", "no readings"); return
    ts, raw = row
    age = _age_s(ts)
    pct = _soil_pct(raw)
    mark = OK if (age is not None and age <= SENSOR_STALE_S) else WARN
    _c(mark, "reading", f"raw {int(raw)} = {pct:.0f}%  ({_fmt_age(age)})")
    _c(INFO, "scale", f"dry {SOIL_RAW_DRY} / wet {SOIL_RAW_WET}; trigger {TRIGGER_PCT:.0f}%")
    if pct <= TRIGGER_PCT:
        _c(WARN, "trigger", f"at/below {TRIGGER_PCT:.0f}% — would water if not in maintenance")

def _check_light(conn):
    _hdr("Grow light")
    hour = datetime.now(LOCAL_TZ).hour
    if GROW_ON_HOUR < GROW_OFF_HOUR:
        expected_on = GROW_ON_HOUR <= hour < GROW_OFF_HOUR
    else:
        expected_on = hour >= GROW_ON_HOUR or hour < GROW_OFF_HOUR
    row = _latest(conn, "sensor_readings", "sensor='lux'")
    if not row:
        _c(WARN, "lux", "no readings")
    else:
        ts, lux = row
        age = _age_s(ts)
        lit = lux > GROW_LUX_THRESHOLD
        if age is not None and age > SENSOR_STALE_S:
            # Lux is stale. If soil is fresh the WROVER is alive but not reporting light.
            # Integrated does read lux, so sustained stale lux during lit hours can mean a
            # dead BH1750 (the alerter escalates that, DL-173); plantctl is a point-in-time
            # check, so it reports INFO here rather than diagnosing a fault.
            soil = _latest(conn, "sensor_readings", "sensor='soil_raw' AND device='soil'")
            soil_age = _age_s(soil[0]) if soil else None
            if soil_age is not None and soil_age <= SENSOR_STALE_S:
                _c(INFO, "lux", f"not reported by current firmware (harness reads no light sensor; "
                                f"last lux {_fmt_age(age)})")
            else:
                _c(WARN, "lux", f"stale ({_fmt_age(age)}) — can't verify light (WROVER may be offline)")
        else:
            want = "on" if expected_on else "off"
            good = (lit == expected_on)
            _c(OK if good else FAIL, "state",
               f"{lux:.0f} lux ({'lit' if lit else 'dark'}); schedule wants {want}")
    _c(INFO, "window", f"{GROW_ON_HOUR:02d}:00–{GROW_OFF_HOUR:02d}:00 {LOCAL_TZ.key}, now hour {hour}")

def _check_db(conn):
    _hdr("Database")
    try:
        n = conn.execute("SELECT COUNT(*) FROM sensor_readings").fetchone()[0]
        latest = conn.execute("SELECT MAX(ts) FROM sensor_readings").fetchone()[0]
        _c(OK, "sensor_readings", f"{n:,} rows; newest {_fmt_age(_age_s(latest))}")
    except Exception as e:
        _c(FAIL, "query", str(e))

# ---- commands ----------------------------------------------------------------
def cmd_health(args):
    print(f"Plant Autonomy Testbed — health  ({_now_utc().astimezone(LOCAL_TZ).strftime('%Y-%m-%d %H:%M %Z')})")
    _check_services()
    conn = _db()
    try:
        _check_wrover(conn)
        _check_soil(conn)
        _check_light(conn)
        _check_pump(conn)
        _check_db(conn)
    finally:
        conn.close()
    print()

def main():
    p = argparse.ArgumentParser(prog="plantctl", description="Read-only hub diagnostics.")
    sub = p.add_subparsers(dest="cmd")
    sub.add_parser("health", help="full hub sweep")
    args = p.parse_args()
    if args.cmd == "health":
        cmd_health(args)
    else:
        p.print_help()
        sys.exit(1)

if __name__ == "__main__":
    main()
