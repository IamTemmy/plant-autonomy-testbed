"""Tests for listener.route_message -- the MQTT topic/payload parser (DL-181).

route_message is the single function that turns an inbound MQTT message into rows
in the specialized tables. It has been at the centre of several audit bugs (F11 the
dropped moist_pct, F9 the daily-mL fabrication, F5 the leak reasons), yet had no
direct test -- the alerter/plantctl suites test the readers, not the writer. This
covers the writer against synthetic messages, using the real schema.sql on an
in-memory SQLite DB. No hardware, no network (alerter no-ops without NTFY_TOPIC).
"""
import os
import sqlite3

import pytest

# Ensure alerter stays silent (no ntfy) even if the environment has it set.
os.environ.pop("NTFY_TOPIC", None)

import listener  # noqa: E402  (conftest puts hub/04-listener on the path)

_SCHEMA = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "hub", "04-listener", "schema.sql",
)


@pytest.fixture
def conn():
    c = sqlite3.connect(":memory:")
    with open(_SCHEMA) as f:
        c.executescript(f.read())
    yield c
    c.close()


def _route(conn, topic, payload, ts="2026-08-20T12:00:00Z"):
    listener.route_message(conn, 1, "run-test", ts, topic, payload)


def _rows(conn, sql, params=()):
    return conn.execute(sql, params).fetchall()


# --------------------------------------------------------------------------- #
# sensor fan-out (plant/sensors/<device>)                                      #
# --------------------------------------------------------------------------- #

def test_sensor_payload_fans_out_to_rows(conn):
    _route(conn, "plant/sensors/soil", '{"soil_raw": 2400, "moisture_pct": 42.5}')
    rows = _rows(conn,
        "SELECT device, sensor, value FROM sensor_readings ORDER BY sensor")
    assert ("soil", "moisture", 42.5) in rows
    assert ("soil", "soil_raw", 2400.0) in rows


def test_lux_stored_under_its_device(conn):
    # F13-adjacent: lux comes in on its own topic -> device='bh1750'.
    _route(conn, "plant/sensors/bh1750", '{"lux": 44.0}')
    rows = _rows(conn, "SELECT device, sensor, value FROM sensor_readings")
    assert rows == [("bh1750", "lux", 44.0)]


def test_missing_fields_are_skipped(conn):
    # Only the present keys become rows; nulls are not written.
    _route(conn, "plant/sensors/bme280", '{"temperature_c": 23.1}')
    rows = _rows(conn, "SELECT sensor, value FROM sensor_readings")
    assert rows == [("temperature", 23.1)]


def test_non_json_sensor_payload_is_ignored(conn):
    _route(conn, "plant/sensors/soil", "not-json")
    assert _rows(conn, "SELECT COUNT(*) FROM sensor_readings")[0][0] == 0


# --------------------------------------------------------------------------- #
# state message (plant/state/wrover) -- the fields the audits touched          #
# --------------------------------------------------------------------------- #

def _state(**over):
    base = {"state": "monitor", "pump": 0, "session_ml": 0, "dose_count": 0,
            "moist_pct": 63.5, "maintenance": 0, "reason": ""}
    base.update(over)
    import json
    return json.dumps(base)


def test_state_writes_fsm_state(conn):
    _route(conn, "plant/state/wrover", _state(state="dosing"))
    rows = _rows(conn,
        "SELECT status FROM system_status WHERE metric='fsm_state'")
    assert rows == [("dosing",)]


def test_state_stores_moist_ema_f11(conn):
    # F11 regression: moist_pct must be persisted as metric 'moist_ema'.
    _route(conn, "plant/state/wrover", _state(moist_pct=63.5))
    rows = _rows(conn,
        "SELECT value FROM system_status WHERE metric='moist_ema'")
    assert rows == [(63.5,)]


def test_state_skips_moist_ema_sentinel(conn):
    # F11: the firmware sends -1 before it has a valid reading; don't store it.
    _route(conn, "plant/state/wrover", _state(moist_pct=-1))
    assert _rows(conn,
        "SELECT COUNT(*) FROM system_status WHERE metric='moist_ema'")[0][0] == 0


def test_state_stores_session_ml_and_dose_count(conn):
    _route(conn, "plant/state/wrover", _state(session_ml=100, dose_count=2))
    got = dict(_rows(conn,
        "SELECT metric, value FROM system_status WHERE metric IN ('session_ml','dose_count')"))
    assert got["session_ml"] == 100.0
    assert got["dose_count"] == 2.0


def test_state_stores_maintenance_flag(conn):
    _route(conn, "plant/state/wrover", _state(maintenance=1))
    rows = _rows(conn,
        "SELECT status, value FROM system_status WHERE metric='maintenance'")
    assert rows == [("on", 1.0)]


def test_state_records_pump(conn):
    _route(conn, "plant/state/wrover", _state(pump=1))
    rows = _rows(conn,
        "SELECT status, value FROM system_status WHERE metric='pump'")
    assert rows == [("on", 1.0)]
