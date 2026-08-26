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


# --------------------------------------------------------------------------- #
# dark capture windows (DL-199, vision phase Piece 3)                          #
# --------------------------------------------------------------------------- #
# in_dark_window() holds the grow-light OFF for the top-down camera's glare-free
# measurement captures. Windows default to centers 08:00..16:00, each running
# center-2min .. center+3min (07:58-08:03, ...). grow_light_should_be_on()
# combines the day window with these: ON only when daytime AND not in a window.

import datetime as _dt


def _at(h, m):
    """A tz-aware datetime at local h:m (date is irrelevant to the logic)."""
    return _dt.datetime(2026, 8, 25, h, m, tzinfo=photoperiod.LOCAL_TZ)


@pytest.fixture
def default_windows(monkeypatch):
    # Force the documented defaults regardless of the environment the tests run in.
    monkeypatch.setattr(photoperiod, "DARK_WINDOW_CENTERS", "08:00,10:00,12:00,14:00,16:00")
    monkeypatch.setattr(photoperiod, "DARK_WINDOW_LEAD_MIN", 2)
    monkeypatch.setattr(photoperiod, "DARK_WINDOW_TAIL_MIN", 3)
    monkeypatch.setattr(photoperiod, "DARK_CENTERS", None)   # force re-parse
    yield


def test_dark_window_center_is_dark(default_windows):
    assert photoperiod.in_dark_window(_at(8, 0)) is True
    assert photoperiod.in_dark_window(_at(12, 0)) is True
    assert photoperiod.in_dark_window(_at(16, 0)) is True


def test_dark_window_boundaries_inclusive(default_windows):
    assert photoperiod.in_dark_window(_at(7, 58)) is True    # center-2, start
    assert photoperiod.in_dark_window(_at(8, 3)) is True     # center+3, end
    assert photoperiod.in_dark_window(_at(7, 57)) is False   # just before
    assert photoperiod.in_dark_window(_at(8, 4)) is False    # just after


def test_non_window_times_are_not_dark(default_windows):
    assert photoperiod.in_dark_window(_at(9, 0)) is False    # between windows
    assert photoperiod.in_dark_window(_at(13, 30)) is False


def test_grow_light_off_during_dark_window(default_windows, window):
    window(7, 19)                                            # normal lit day
    # 08:00 is daytime, but it's a dark window -> light must be OFF.
    assert photoperiod.grow_light_should_be_on(_at(8, 0)) is False


def test_grow_light_on_daytime_outside_windows(default_windows, window):
    window(7, 19)
    assert photoperiod.grow_light_should_be_on(_at(9, 0)) is True
    assert photoperiod.grow_light_should_be_on(_at(13, 30)) is True


def test_grow_light_off_at_night_regardless(default_windows, window):
    window(7, 19)
    # 22:00 is outside the day window -> OFF (and not a dark window either).
    assert photoperiod.grow_light_should_be_on(_at(22, 0)) is False


def test_dark_window_only_matters_during_day(default_windows, window):
    # A dark-window center that fell outside the lit day would still be OFF
    # because the day window already says OFF -- no double-negative surprise.
    window(9, 19)                                            # day starts 09:00
    assert photoperiod.grow_light_should_be_on(_at(8, 0)) is False  # before day AND a window


def test_malformed_center_is_skipped(monkeypatch):
    monkeypatch.setattr(photoperiod, "DARK_WINDOW_CENTERS", "08:00,garbage,12:00")
    monkeypatch.setattr(photoperiod, "DARK_CENTERS", None)
    # Valid centers still work; the bad one is dropped, not fatal.
    assert photoperiod.in_dark_window(_at(8, 0)) is True
    assert photoperiod.in_dark_window(_at(12, 0)) is True
