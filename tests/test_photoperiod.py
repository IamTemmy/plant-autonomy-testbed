"""Host-tests for hub/08-grow-light/photoperiod.py (P2-13).

`desired_on(hour)` decides whether the grow light should be on at a given local
hour. It handles two cases:
  * normal window (ON_HOUR < OFF_HOUR): on for [ON, OFF).
  * overnight window (ON_HOUR > OFF_HOUR): on if hour >= ON OR hour < OFF (wraps
    midnight).

The overnight-wrap branch is the subtle one, so it gets explicit coverage. Tests
patch the module's ON_HOUR/OFF_HOUR so they don't depend on the deployed schedule.
"""
import pytest

import photoperiod


@pytest.fixture
def window(monkeypatch):
    """Set the photoperiod window for a test: window(on, off)."""
    def _set(on, off):
        monkeypatch.setattr(photoperiod, "ON_HOUR", on)
        monkeypatch.setattr(photoperiod, "OFF_HOUR", off)
    return _set


# --------------------------------------------------------------------------- #
# normal daytime window (ON < OFF), e.g. the deployed 07:00-19:00              #
# --------------------------------------------------------------------------- #

def test_normal_window_on_at_start(window):
    window(7, 19)
    assert photoperiod.desired_on(7) is True     # ON boundary is inclusive


def test_normal_window_off_at_end(window):
    window(7, 19)
    assert photoperiod.desired_on(19) is False    # OFF boundary is exclusive


def test_normal_window_on_midday(window):
    window(7, 19)
    assert photoperiod.desired_on(12) is True


def test_normal_window_off_before_start(window):
    window(7, 19)
    assert photoperiod.desired_on(6) is False


def test_normal_window_off_after_end(window):
    window(7, 19)
    assert photoperiod.desired_on(23) is False


def test_normal_window_off_at_midnight(window):
    window(7, 19)
    assert photoperiod.desired_on(0) is False


# --------------------------------------------------------------------------- #
# overnight window (ON > OFF) -- wraps midnight, e.g. 20:00-06:00              #
# --------------------------------------------------------------------------- #

def test_overnight_on_late_evening(window):
    window(20, 6)
    assert photoperiod.desired_on(22) is True     # after ON, before midnight


def test_overnight_on_at_midnight(window):
    window(20, 6)
    assert photoperiod.desired_on(0) is True       # past midnight, before OFF


def test_overnight_on_early_morning(window):
    window(20, 6)
    assert photoperiod.desired_on(5) is True


def test_overnight_off_at_off_hour(window):
    window(20, 6)
    assert photoperiod.desired_on(6) is False      # OFF boundary exclusive


def test_overnight_off_midday(window):
    window(20, 6)
    assert photoperiod.desired_on(12) is False


def test_overnight_on_at_on_hour(window):
    window(20, 6)
    assert photoperiod.desired_on(20) is True      # ON boundary inclusive


# --------------------------------------------------------------------------- #
# every hour is decided (no gaps / no double-cover across a full day)          #
# --------------------------------------------------------------------------- #

def test_full_day_normal_window_hours(window):
    window(7, 19)
    on_hours = [h for h in range(24) if photoperiod.desired_on(h)]
    assert on_hours == list(range(7, 19))          # exactly 7..18 inclusive


def test_full_day_overnight_window_hours(window):
    window(20, 6)
    on_hours = [h for h in range(24) if photoperiod.desired_on(h)]
    assert on_hours == [0, 1, 2, 3, 4, 5, 20, 21, 22, 23]
