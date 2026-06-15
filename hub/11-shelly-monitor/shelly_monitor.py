#!/usr/bin/env python3
"""
Shelly uptime/RSSI monitor for the plant-autonomy hub.

The grow-light Shelly Plug (10.6.17.32) sits on the flaky campus WiFi (DL-028).
A telemetry gap alone cannot distinguish a REBOOT (device restarted) from a
DROPOUT (stayed powered, lost WiFi): a reboot resets the device's uptime
counter, a dropout does not. This poller samples the Shelly's own uptime + RSSI
once a minute via a direct HTTP RPC call and logs them, which makes the
distinction unambiguous after the fact:

  * uptime resets to ~0                         -> the device rebooted
  * a gap, but uptime still climbing afterward  -> just a WiFi dropout
  * rssi series                                 -> correlate dropouts with signal

uptime/rssi go to sensor_readings (device='shelly'); a detected uptime reset is
also written to system_status as a metric='reboot' marker, mirroring the WROVER
reboot detection (DL-060). Shelly reboots are recorded for analysis and the
dashboard but deliberately NOT wired into the alerter -- the plug's WiFi is
known-flaky and the grow-light verification alert (DL-063) already covers any
real functional impact (light state wrong vs. schedule).

Run once a minute via plant-shelly-monitor.timer. Stateless: the previous uptime
is read from the DB, so reboot detection survives poller restarts.
"""
import json
import os
import sqlite3
import sys
import urllib.request
from datetime import datetime, timezone

SHELLY_HOST = os.environ.get("SHELLY_HOST", "10.6.17.32")
DB_PATH = os.environ.get("PLANT_DB", "/home/basilpi/plant-hub/plant.db")
TIMEOUT = float(os.environ.get("SHELLY_POLL_TIMEOUT", "4"))
ISO = "%Y-%m-%dT%H:%M:%SZ"  # matches the stored ts format exactly


def log(msg):
    print(f"{datetime.now(timezone.utc).strftime(ISO)} [shelly-monitor] {msg}", flush=True)


def poll():
    """Return (uptime_s, rssi) from one Shelly.GetStatus call. Raises on unreachable."""
    url = f"http://{SHELLY_HOST}/rpc/Shelly.GetStatus"
    with urllib.request.urlopen(url, timeout=TIMEOUT) as r:
        st = json.load(r)
    return int(st["sys"]["uptime"]), int(st["wifi"]["rssi"])


def record(conn, ts, uptime, rssi):
    """Log the sample; if uptime went backwards vs. the last sample, log a reboot."""
    prev = conn.execute(
        "SELECT value FROM sensor_readings WHERE device='shelly' AND sensor='uptime' "
        "ORDER BY id DESC LIMIT 1").fetchone()
    rebooted = prev is not None and uptime < prev[0]
    if rebooted:
        conn.execute(
            "INSERT INTO system_status (ts, message_id, run_id, device, status, metric, value) "
            "VALUES (?, NULL, NULL, 'shelly', 'reboot', 'reboot', ?)",
            (ts, float(prev[0])))
        log(f"reboot detected: uptime {uptime}s < previous {int(prev[0])}s")
    conn.executemany(
        "INSERT INTO sensor_readings (ts, message_id, run_id, device, sensor, value, unit) "
        "VALUES (?, NULL, NULL, 'shelly', ?, ?, ?)",
        [(ts, "uptime", float(uptime), "s"), (ts, "rssi", float(rssi), "dBm")])
    conn.commit()
    return rebooted


def main():
    if not os.path.exists(DB_PATH):
        log(f"DB not found: {DB_PATH}")
        return 1
    try:
        uptime, rssi = poll()
    except Exception as e:
        # Unreachable this minute = a dropout (or a reboot in progress). The gap in
        # the uptime series is itself the record; nothing to insert.
        log(f"unreachable: {e}")
        return 0
    ts = datetime.now(timezone.utc).strftime(ISO)
    conn = sqlite3.connect(DB_PATH, timeout=30)
    conn.execute("PRAGMA journal_mode = WAL")
    conn.execute("PRAGMA busy_timeout = 30000")
    try:
        record(conn, ts, uptime, rssi)
    finally:
        conn.close()
    log(f"uptime={uptime}s rssi={rssi}dBm")
    return 0


if __name__ == "__main__":
    sys.exit(main())
