"""Tests for the capture orchestrator's decision logic (DL-200, Piece 4).

decide(now, centers) is the pure core: given the time and the dark-window centers,
it returns "dark" | "lit" | None. The plug-state gate and MQTT publish are I/O and
tested by deploy, not here. These lock the schedule: dark centers win over the :00
lit slot, lit fires at :00/:30 in the lit day, nothing fires at night.
"""
import datetime as _dt

import pytest

import capture_scheduler as cs  # conftest puts hub/13-capture-scheduler on the path

_CENTERS = cs.parse_centers("08:00,10:00,12:00,14:00,16:00")


def _at(h, m):
    return _dt.datetime(2026, 8, 26, h, m, tzinfo=cs.LOCAL_TZ)


@pytest.fixture(autouse=True)
def _day_window(monkeypatch):
    # Pin the lit day to 07:00-19:00 regardless of the test environment.
    monkeypatch.setattr(cs, "ON_HOUR", 7)
    monkeypatch.setattr(cs, "OFF_HOUR", 19)
    yield


def test_dark_center_returns_dark():
    assert cs.decide(_at(8, 0), _CENTERS) == "dark"
    assert cs.decide(_at(12, 0), _CENTERS) == "dark"
    assert cs.decide(_at(16, 0), _CENTERS) == "dark"


def test_dark_center_wins_over_lit_slot():
    # 08:00 is both a dark center and a :00 lit slot -- dark must win.
    assert cs.decide(_at(8, 0), _CENTERS) == "dark"


def test_lit_slots_on_the_half_hour():
    assert cs.decide(_at(9, 0), _CENTERS) == "lit"     # :00, not a dark center
    assert cs.decide(_at(9, 30), _CENTERS) == "lit"    # :30
    assert cs.decide(_at(13, 30), _CENTERS) == "lit"


def test_dark_center_hour_still_lit_on_the_half_hour():
    # 08:30 is not a center -> normal lit capture even in a dark-window hour.
    assert cs.decide(_at(8, 30), _CENTERS) == "lit"


def test_non_slot_minutes_do_nothing():
    assert cs.decide(_at(9, 15), _CENTERS) is None
    assert cs.decide(_at(9, 45), _CENTERS) is None
    assert cs.decide(_at(8, 1), _CENTERS) is None      # inside the dark window but not the center


def test_night_does_nothing():
    assert cs.decide(_at(3, 0), _CENTERS) is None
    assert cs.decide(_at(22, 0), _CENTERS) is None
    assert cs.decide(_at(22, 30), _CENTERS) is None


def test_day_boundaries():
    assert cs.decide(_at(7, 0), _CENTERS) == "lit"     # ON_HOUR inclusive, :00
    assert cs.decide(_at(19, 0), _CENTERS) is None     # OFF_HOUR exclusive


def test_parse_centers_skips_malformed():
    got = cs.parse_centers("08:00,junk,10:00")
    assert got == [8 * 60, 10 * 60]
