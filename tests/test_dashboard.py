"""Tests for the camera query in dash_common.latest_camera (DL-194 regression guard).

The camera panel crashed in production with `no such column: id`: DL-177 changed this
query to ORDER BY id, but camera_readings has no id column (its PK is ts). It shipped
because the query had no test and camera_readings' schema differs from the EAV tables.

dash_common imports streamlit/pandas/PIL at module top, which aren't needed to test the
SQL and would drag heavy deps into CI. So instead of importing the module, we extract
the actual camera query string from the source file and run it against the real
schema.sql on an in-memory DB. That still catches the real bug class -- a query naming
a column the schema lacks raises OperationalError here -- while keeping CI lean and
guaranteeing the test tracks the real query (it reads it from source, so it can't drift).
"""
import os
import re
import sqlite3

import pytest

_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
_SCHEMA = os.path.join(_ROOT, "hub", "04-listener", "schema.sql")
_DASH_COMMON = os.path.join(_ROOT, "hub", "06-dashboard", "dash_common.py")


def _camera_query():
    """Pull the exact camera SELECT out of dash_common source (so the test can't drift
    from the real query). Matches the SELECT ... FROM camera_readings ... statement,
    joining the adjacent string literals the source splits it across."""
    src = open(_DASH_COMMON).read()
    # Grab the string-literal fragments between the SELECT and the closing paren of execute().
    m = re.search(r'"(SELECT[^"]*?)"\s*\n\s*"([^"]*?camera_readings[^"]*?)"', src)
    assert m, "could not locate the camera query in dash_common.py"
    return (m.group(1) + m.group(2)).strip()


@pytest.fixture
def conn():
    c = sqlite3.connect(":memory:")
    with open(_SCHEMA) as f:
        c.executescript(f.read())
    yield c
    c.close()


def _add_cam(c, ts, path="/img/x.jpg", greenness=0.5, area=0.4, ratio=0.3):
    c.execute(
        "INSERT INTO camera_readings (ts, path, greenness, green_area, green_ratio) "
        "VALUES (?,?,?,?,?)",
        (ts, path, greenness, area, ratio))
    c.commit()


def test_camera_query_runs_against_real_schema(conn):
    # The DL-194 guard: the query must reference only columns that exist in
    # camera_readings. A stale column (e.g. ORDER BY id) raises OperationalError here.
    _add_cam(conn, "2026-08-23T12:00:00Z")
    row = conn.execute(_camera_query()).fetchone()   # must not raise
    assert row is not None


def test_camera_query_returns_newest_by_ts(conn):
    _add_cam(conn, "2026-08-23T10:00:00Z", path="/img/old.jpg")
    _add_cam(conn, "2026-08-23T12:00:00Z", path="/img/new.jpg")
    _add_cam(conn, "2026-08-23T11:00:00Z", path="/img/mid.jpg")
    row = conn.execute(_camera_query()).fetchone()
    assert row[0] == "2026-08-23T12:00:00Z"
    assert row[1] == "/img/new.jpg"


def test_camera_query_none_when_empty(conn):
    # No rows -> fetchone() returns None; the panel must handle this, not error.
    assert conn.execute(_camera_query()).fetchone() is None


def test_camera_query_selects_five_columns(conn):
    _add_cam(conn, "2026-08-23T12:00:00Z", greenness=0.61, area=0.42, ratio=0.29)
    row = conn.execute(_camera_query()).fetchone()
    # ts, path, greenness, green_area, green_ratio.
    assert len(row) == 5
    assert row[2] == 0.61 and row[3] == 0.42 and row[4] == 0.29
