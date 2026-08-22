"""Host-tests for hub/04-listener/alerter.py query helpers (P2-13).

Covers the evidence-based board-liveness signals (DL-137/139) and the reboot
classification (DL-138) -- the logic we actually had bugs in, so regression
coverage here has real value:

  * _soil_age_s   -- board-liveness age from the latest soil reading.
  * _latest_lux   -- latest lux value + age.
  * _soil_pct     -- latest moisture %.
  * _reboots_24h / _dev_reboots_24h -- armed vs maintenance reboot counts (DL-138).
  * _presence     -- latest presence status.

All run against synthetic in-memory SQLite mirroring the real schema. No network,
no live DB, no hardware.
"""
import sqlite3
from datetime import datetime, timedelta, timezone

import pytest

import alerter

ISO = "%Y-%m-%dT%H:%M:%SZ"


def _iso(dt):
    return dt.strftime(ISO)


def _conn():
    """In-memory DB with the two tables the helpers read."""
    c = sqlite3.connect(":memory:")
    c.execute(
        "CREATE TABLE sensor_readings "
        "(id INTEGER PRIMARY KEY, ts TEXT, device TEXT, sensor TEXT, value REAL)")
    c.execute(
        "CREATE TABLE system_status "
        "(id INTEGER PRIMARY KEY, ts TEXT, device TEXT, metric TEXT, status TEXT)")
    return c


def _add_reading(c, sensor, value, age_s=0, device=None):
    ts = _iso(datetime.now(timezone.utc) - timedelta(seconds=age_s))
    c.execute("INSERT INTO sensor_readings (ts, device, sensor, value) VALUES (?,?,?,?)",
              (ts, device, sensor, value))
    c.commit()


def _add_status(c, metric, status, age_h=0, device="wrover"):
    ts = _iso(datetime.now(timezone.utc) - timedelta(hours=age_h))
    c.execute("INSERT INTO system_status (ts, device, metric, status) VALUES (?,?,?,?)",
              (ts, device, metric, status))
    c.commit()


# --------------------------------------------------------------------------- #
# _soil_age_s -- the DL-139 board-liveness signal                             #
# --------------------------------------------------------------------------- #

def test_soil_age_none_when_no_readings():
    assert alerter._soil_age_s(_conn()) is None


def test_soil_age_recent_reading_is_small():
    c = _conn()
    _add_reading(c, "soil_raw", 2400, age_s=20, device="soil")
    age = alerter._soil_age_s(c)
    assert age is not None and 0 <= age < 120   # ~20s, allow scheduling slack


def test_soil_age_old_reading_is_large():
    c = _conn()
    _add_reading(c, "soil_raw", 2400, age_s=3 * 86400, device="soil")   # 3 days
    age = alerter._soil_age_s(c)
    assert age is not None and age > 2 * 86400


def test_soil_age_requires_soil_device_and_soil_raw_sensor():
    # A lux reading must NOT satisfy the soil-liveness query.
    c = _conn()
    _add_reading(c, "lux", 5.0, age_s=10)          # no device, wrong sensor
    assert alerter._soil_age_s(c) is None


def test_board_liveness_gate_reproduces_dl139_logic():
    # The DL-139 gate: board_alive = soil fresh. Harness case (soil fresh, lux stale)
    # -> alive -> suppress "offline". Board-down case (soil also stale) -> not alive.
    stale = alerter.GROW_LUX_STALE_S
    c_alive = _conn()
    _add_reading(c_alive, "soil_raw", 2400, age_s=30, device="soil")
    sa = alerter._soil_age_s(c_alive)
    assert sa is not None and sa <= stale          # board_alive -> True

    c_down = _conn()
    _add_reading(c_down, "soil_raw", 2400, age_s=2 * 86400, device="soil")
    sd = alerter._soil_age_s(c_down)
    assert sd is not None and sd > stale            # board_alive -> False


# --------------------------------------------------------------------------- #
# _latest_lux / _soil_pct                                                     #
# --------------------------------------------------------------------------- #

def test_latest_lux_none_when_absent():
    assert alerter._latest_lux(_conn()) == (None, None)


def test_latest_lux_returns_value_and_age():
    c = _conn()
    _add_reading(c, "lux", 42.0, age_s=15, device="bh1750")
    val, age = alerter._latest_lux(c)
    assert val == 42.0
    assert age is not None and 0 <= age < 120


def test_latest_lux_takes_most_recent():
    c = _conn()
    _add_reading(c, "lux", 10.0, age_s=600, device="bh1750")
    _add_reading(c, "lux", 99.0, age_s=5, device="bh1750")
    val, _ = alerter._latest_lux(c)
    assert val == 99.0


def test_soil_pct_latest_value():
    c = _conn()
    _add_reading(c, "moisture", 41.0, age_s=60, device="soil")
    _add_reading(c, "moisture", 44.5, age_s=5, device="soil")
    assert alerter._soil_pct(c) == 44.5


# --------------------------------------------------------------------------- #
# reboot classification -- DL-138                                             #
# --------------------------------------------------------------------------- #

def test_reboots_24h_counts_only_armed():
    c = _conn()
    _add_status(c, "reboot", "reboot", age_h=1)         # armed -> counts
    _add_status(c, "reboot", "reboot", age_h=5)         # armed -> counts
    _add_status(c, "reboot", "reboot_maint", age_h=2)   # dev flash -> excluded
    assert alerter._reboots_24h(c) == 2


def test_dev_reboots_24h_counts_only_maint():
    c = _conn()
    _add_status(c, "reboot", "reboot", age_h=1)         # armed -> excluded here
    _add_status(c, "reboot", "reboot_maint", age_h=2)   # dev -> counts
    _add_status(c, "reboot", "reboot_maint", age_h=3)   # dev -> counts
    assert alerter._dev_reboots_24h(c) == 2


def test_reboots_24h_excludes_older_than_24h():
    c = _conn()
    _add_status(c, "reboot", "reboot", age_h=1)         # in window
    _add_status(c, "reboot", "reboot", age_h=30)        # older than 24h -> excluded
    assert alerter._reboots_24h(c) == 1


def test_reboots_24h_zero_when_none():
    assert alerter._reboots_24h(_conn()) == 0
    assert alerter._dev_reboots_24h(_conn()) == 0


# --------------------------------------------------------------------------- #
# _presence                                                                   #
# --------------------------------------------------------------------------- #

def test_presence_unknown_when_empty():
    assert alerter._presence(_conn()) == "unknown"


def test_presence_returns_latest():
    c = _conn()
    _add_status(c, None, "online", age_h=2)
    _add_status(c, None, "offline", age_h=1)
    assert alerter._presence(c) == "offline"


