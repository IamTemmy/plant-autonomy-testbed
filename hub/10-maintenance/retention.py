#!/usr/bin/env python3
"""
Database retention for the plant-autonomy hub.

The hub appends rows continuously (per-minute power telemetry, ~30s sensor
pushes, and every MQTT message archived to mqtt_messages), so plant.db grows
without bound. Query speed is unaffected -- DL-067 made the windowed queries
ride the time indexes, so they only touch their window regardless of table
size -- but the file and the Pi's SD card are not infinite. This job trims the
high-frequency tables to a rolling window and leaves the sparse, valuable
tables (runs, actuator_events, fault_events) untouched.

Run nightly via plant-retention.timer. Safe to run by hand.

Design notes:
  * Deletes are SARGABLE ("ts < <cutoff>", cutoff built in the stored ISO
    format) so they range-scan the per-table ts index instead of full-scanning.
  * Deletes are BATCHED with a commit per batch, so a large first-run backlog
    cannot hold a long write lock or balloon the WAL file against the live
    listener; we also briefly yield between full batches.
  * WAL + busy_timeout: this process waits for the listener's writes instead of
    erroring, and (under WAL) never blocks the listener's reads.
"""
import os
import sqlite3
import sys
import time
from datetime import datetime, timedelta, timezone

DB_PATH = os.environ.get("PLANT_DB", "/home/basilpi/plant-hub/plant.db")
BATCH = int(os.environ.get("RETENTION_BATCH", "5000"))

ISO = "%Y-%m-%dT%H:%M:%SZ"  # matches the stored ts format exactly

# table -> retention days. Tables NOT listed here are kept forever.
# Each is env-overridable so windows can be tuned without a redeploy.
TABLES = {
    "mqtt_messages":   int(os.environ.get("RETENTION_MQTT_DAYS",   "7")),
    "system_status":   int(os.environ.get("RETENTION_STATUS_DAYS", "14")),
    "sensor_readings": int(os.environ.get("RETENTION_SENSOR_DAYS", "30")),
}


def log(msg):
    print(f"{datetime.now(timezone.utc).strftime(ISO)} [retention] {msg}", flush=True)


def prune(conn, table, days):
    """Batch-delete rows older than `days`. Returns total rows removed."""
    cutoff = (datetime.now(timezone.utc) - timedelta(days=days)).strftime(ISO)
    removed = 0
    while True:
        cur = conn.execute(
            f"DELETE FROM {table} WHERE rowid IN "
            f"(SELECT rowid FROM {table} WHERE ts < ? LIMIT ?)",
            (cutoff, BATCH),
        )
        conn.commit()
        n = cur.rowcount
        if n <= 0:
            break
        removed += n
        if n == BATCH:
            time.sleep(0.05)  # yield the write lock to the listener between batches
    log(f"{table}: removed {removed} rows older than {days}d (cutoff {cutoff})")
    return removed


def main():
    if not os.path.exists(DB_PATH):
        log(f"DB not found: {DB_PATH}")
        return 1
    conn = sqlite3.connect(DB_PATH, timeout=30)
    conn.execute("PRAGMA journal_mode = WAL")
    conn.execute("PRAGMA busy_timeout = 30000")
    total = 0
    t0 = time.time()
    try:
        for table, days in TABLES.items():
            if days <= 0:
                log(f"{table}: retention disabled (days={days}), skipping")
                continue
            total += prune(conn, table, days)
        # Reset the WAL file after potentially large deletes so it doesn't sit huge.
        conn.execute("PRAGMA wal_checkpoint(TRUNCATE)")
    finally:
        conn.close()
    log(f"done: {total} rows removed in {time.time() - t0:.1f}s")
    return 0


if __name__ == "__main__":
    sys.exit(main())
