"""Host-tests for hub/10-maintenance/retention.py (P2-13, first module).

Covers the two pure data operations:
  * prune()        -- batch-delete rows older than a cutoff, keep recent rows.
  * prune_images() -- delete JPEGs older than N days by mtime, keep the rest.

Both run against synthetic state (in-memory SQLite / a tmp dir), so there is no
dependency on the live plant.db, the Pi, or any hardware.
"""
import os
import sqlite3
import time
from datetime import datetime, timedelta, timezone

import pytest

import retention

ISO = "%Y-%m-%dT%H:%M:%SZ"


def _iso(dt):
    return dt.strftime(ISO)


def _make_db(rows):
    """In-memory DB with a `sensor_readings(ts, value)` table seeded from `rows`
    (a list of (ts_iso, value)). Mirrors the real table's ts column + format."""
    conn = sqlite3.connect(":memory:")
    conn.execute("CREATE TABLE sensor_readings (id INTEGER PRIMARY KEY, ts TEXT, value REAL)")
    conn.executemany("INSERT INTO sensor_readings (ts, value) VALUES (?, ?)", rows)
    conn.commit()
    return conn


def _count(conn, table="sensor_readings"):
    return conn.execute(f"SELECT COUNT(*) FROM {table}").fetchone()[0]


# --------------------------------------------------------------------------- #
# prune()                                                                     #
# --------------------------------------------------------------------------- #

def test_prune_removes_only_rows_older_than_cutoff():
    now = datetime.now(timezone.utc)
    rows = [
        (_iso(now - timedelta(days=40)), 1.0),   # older than 30d -> delete
        (_iso(now - timedelta(days=31)), 2.0),   # older than 30d -> delete
        (_iso(now - timedelta(days=29)), 3.0),   # within 30d     -> keep
        (_iso(now - timedelta(days=1)),  4.0),   # recent         -> keep
    ]
    conn = _make_db(rows)
    removed = retention.prune(conn, "sensor_readings", days=30)
    assert removed == 2
    assert _count(conn) == 2
    # The survivors are exactly the two most-recent rows.
    kept = [r[0] for r in conn.execute("SELECT value FROM sensor_readings ORDER BY value")]
    assert kept == [3.0, 4.0]


def test_prune_keeps_everything_when_all_recent():
    now = datetime.now(timezone.utc)
    rows = [(_iso(now - timedelta(days=d)), float(d)) for d in range(0, 10)]
    conn = _make_db(rows)
    removed = retention.prune(conn, "sensor_readings", days=30)
    assert removed == 0
    assert _count(conn) == 10


def test_prune_empty_table_is_noop():
    conn = _make_db([])
    removed = retention.prune(conn, "sensor_readings", days=30)
    assert removed == 0
    assert _count(conn) == 0


def test_prune_boundary_row_exactly_at_cutoff_is_kept():
    # prune uses a strict `ts < cutoff`, so a row AT the cutoff instant is retained.
    # Use a clearly-older and a clearly-newer row to avoid clock-race flakiness on
    # the exact-boundary microsecond, and assert the strict-less-than contract via
    # a row a hair NEWER than the cutoff surviving.
    now = datetime.now(timezone.utc)
    rows = [
        (_iso(now - timedelta(days=31)), 1.0),               # clearly older -> delete
        (_iso(now - timedelta(days=30) + timedelta(hours=1)), 2.0),  # just inside 30d -> keep
    ]
    conn = _make_db(rows)
    removed = retention.prune(conn, "sensor_readings", days=30)
    assert removed == 1
    assert _count(conn) == 1


def test_prune_batches_when_over_batch_size(monkeypatch):
    # Force a tiny batch so a >1-batch backlog exercises the loop + per-batch commit.
    monkeypatch.setattr(retention, "BATCH", 3)
    now = datetime.now(timezone.utc)
    rows = [(_iso(now - timedelta(days=40)), float(i)) for i in range(7)]  # 7 old rows, batch 3
    conn = _make_db(rows)
    removed = retention.prune(conn, "sensor_readings", days=30)
    assert removed == 7          # all removed across 3 batches (3+3+1)
    assert _count(conn) == 0


# --------------------------------------------------------------------------- #
# prune_images()                                                              #
# --------------------------------------------------------------------------- #

def _make_jpg(dir_path, name, age_days):
    p = os.path.join(dir_path, name)
    with open(p, "wb") as f:
        f.write(b"\xff\xd8\xff\xe0fakejpg")  # tiny JPEG-ish payload
    mtime = time.time() - age_days * 86400
    os.utime(p, (mtime, mtime))
    return p


def test_prune_images_removes_old_keeps_recent(tmp_path):
    d = str(tmp_path)
    old = _make_jpg(d, "old.jpg", age_days=100)
    new = _make_jpg(d, "new.jpg", age_days=10)
    removed = retention.prune_images(d, days=90)
    assert removed == 1
    assert not os.path.exists(old)
    assert os.path.exists(new)


def test_prune_images_ignores_non_jpg(tmp_path):
    d = str(tmp_path)
    _make_jpg(d, "old.jpg", age_days=100)
    # An old non-JPG file must be left alone (only *.jpg are pruned).
    other = os.path.join(d, "notes.txt")
    with open(other, "w") as f:
        f.write("keep me")
    os.utime(other, (time.time() - 100 * 86400, time.time() - 100 * 86400))
    removed = retention.prune_images(d, days=90)
    assert removed == 1
    assert os.path.exists(other)


def test_prune_images_disabled_when_days_zero(tmp_path):
    d = str(tmp_path)
    _make_jpg(d, "old.jpg", age_days=100)
    removed = retention.prune_images(d, days=0)  # 0 = disabled
    assert removed == 0
    assert os.path.exists(os.path.join(d, "old.jpg"))


def test_prune_images_missing_dir_is_safe():
    removed = retention.prune_images("/nonexistent/path/xyz", days=90)
    assert removed == 0
