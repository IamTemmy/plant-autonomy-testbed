"""Host-tests for hub/12-plantctl/plantctl.py pure helpers (P2-13).

Covers the small pure functions behind the CLI status readout:
  * _soil_pct  -- raw ADC -> moisture %, clamped to [0, 100].
  * _age_s     -- stored UTC ts -> seconds-ago, tolerant of two ts formats.
  * _fmt_age   -- seconds -> human "Ns/Nm/Nh/Nd ago" with correct bucket boundaries.

No DB, network, or hardware -- these are the deterministic bits of plantctl.
"""
from datetime import datetime, timedelta, timezone

import pytest

import plantctl

ISO = "%Y-%m-%dT%H:%M:%SZ"


def _iso_ago(seconds):
    return (datetime.now(timezone.utc) - timedelta(seconds=seconds)).strftime(ISO)


# --------------------------------------------------------------------------- #
# _soil_pct -- raw -> %, clamped                                              #
# --------------------------------------------------------------------------- #

def test_soil_pct_at_dry_anchor_is_zero():
    assert plantctl._soil_pct(plantctl.SOIL_RAW_DRY) == 0.0


def test_soil_pct_at_wet_anchor_is_hundred():
    assert plantctl._soil_pct(plantctl.SOIL_RAW_WET) == 100.0


def test_soil_pct_midpoint_is_fifty():
    mid = (plantctl.SOIL_RAW_DRY + plantctl.SOIL_RAW_WET) / 2
    assert plantctl._soil_pct(mid) == pytest.approx(50.0)


def test_soil_pct_clamps_below_zero():
    # raw drier than the dry anchor would be negative -> clamped to 0.
    assert plantctl._soil_pct(plantctl.SOIL_RAW_DRY + 500) == 0.0


def test_soil_pct_clamps_above_hundred():
    # raw wetter than the wet anchor would exceed 100 -> clamped to 100.
    assert plantctl._soil_pct(plantctl.SOIL_RAW_WET - 500) == 100.0


# --------------------------------------------------------------------------- #
# _age_s -- ts string -> seconds ago                                          #
# --------------------------------------------------------------------------- #

def test_age_none_for_empty():
    assert plantctl._age_s("") is None
    assert plantctl._age_s(None) is None


def test_age_none_for_garbage():
    assert plantctl._age_s("not-a-timestamp") is None


def test_age_parses_z_suffix_format():
    age = plantctl._age_s(_iso_ago(100))
    assert age is not None and 90 < age < 130     # ~100s, allow slack


def test_age_parses_isoformat_offset():
    ts = (datetime.now(timezone.utc) - timedelta(seconds=60)).isoformat()
    age = plantctl._age_s(ts)
    assert age is not None and 50 < age < 90


# --------------------------------------------------------------------------- #
# _fmt_age -- bucket boundaries                                               #
# --------------------------------------------------------------------------- #

def test_fmt_age_unknown_when_none():
    assert plantctl._fmt_age(None) == "unknown"


def test_fmt_age_seconds_bucket():
    assert plantctl._fmt_age(45) == "45s ago"
    assert plantctl._fmt_age(89) == "89s ago"      # just under the 90s cutoff


def test_fmt_age_minutes_bucket():
    assert plantctl._fmt_age(90) == "1m ago"       # at the cutoff -> minutes
    assert plantctl._fmt_age(600) == "10m ago"


def test_fmt_age_hours_bucket():
    assert plantctl._fmt_age(5400) == "1h ago"     # 90 min -> hours bucket
    assert plantctl._fmt_age(7200) == "2h ago"


def test_fmt_age_days_bucket():
    assert plantctl._fmt_age(172800) == "2d ago"   # 48h -> days bucket
    assert plantctl._fmt_age(259200) == "3d ago"
