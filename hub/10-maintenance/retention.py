#!/usr/bin/env python3
"""
Database retention for the plant-autonomy hub.

The hub appends rows continuously (per-minute power telemetry, ~30s sensor
pushes, and every MQTT message archived to mqtt_messages), so plant.db grows
without bound. Query speed is unaffected -- DL-067 made the windowed queries
ride the time indexes, so they only touch their window regardless of table
size -- but the file and the Pi's SD card are not infinite. This job trims the
high-frequency tables to a rolling window and leaves the sparse, valuable
tables (runs, actuator_events, fault_events) untouched. It also prunes old
camera image files (large JPEGs) from the images directory, while keeping their
small, valuable metric rows in camera_readings.

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

# Camera images live on disk, not in the DB. The metric rows in camera_readings
# are tiny and worth keeping long-term; the JPEG files are large, so prune the
# files on their own rolling window and leave the rows in place.
IMAGE_DIR = os.environ.get("IMAGE_DIR", "/home/basilpi/plant-hub/images")
IMAGE_RETENTION_DAYS = int(os.environ.get("IMAGE_RETENTION_DAYS", "90"))


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


def prune_images(image_dir, days):
    """Delete camera JPEGs older than `days` (by mtime). Only the image files
    are removed; the metric rows in camera_readings are kept. mtime equals the
    capture/write time in normal operation, and is timezone-agnostic."""
    if days <= 0:
        log(f"images: retention disabled (days={days}), skipping")
        return 0
    if not os.path.isdir(image_dir):
        log(f"images: dir not found ({image_dir}), skipping")
        return 0
    cutoff = time.time() - days * 86400
    removed = 0
    freed = 0
    for entry in os.scandir(image_dir):
        if not entry.is_file() or not entry.name.lower().endswith(".jpg"):
            continue
        try:
            info = entry.stat()
            if info.st_mtime < cutoff:
                freed += info.st_size
                os.remove(entry.path)
                removed += 1
        except OSError as e:
            log(f"images: could not remove {entry.name}: {e}")
    log(f"images: removed {removed} files older than {days}d "
        f"({freed / 1048576:.1f} MB freed) from {image_dir}")
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
    # Image files are on disk, not in the DB, so this runs after the DB is closed.
    images = prune_images(IMAGE_DIR, IMAGE_RETENTION_DAYS)
    log(f"done: {total} rows and {images} image files removed in {time.time() - t0:.1f}s")
    return 0


if __name__ == "__main__":
    sys.exit(main())
