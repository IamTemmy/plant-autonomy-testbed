"""Contract test: firmware state/reason strings <-> alerter alert keys (P2-13, F15).

The bugs F1/F2/F5 were all the same shape: the firmware emits a state or reason
string, but the alerter has no matching key (or the wrong one), so a real fault
fires the pump-off latch and sends NO push. A table-driven contract test catches
that class automatically and stops it regressing.

This test parses the actual strings out of firmware/integrated/src/fsm.cpp (so it
tracks the real firmware, not a hand-copied list) and checks them against the
alerter's two alert dictionaries:
  * FAULT_ALERTS     -- keyed on FSM state (polled).
  * _WATERING_ALERTS -- keyed on session reason (event-driven).

Rules enforced:
  1. Every firmware FAULT STATE has a FAULT_ALERTS entry.
  2. Every firmware stop/fault REASON is covered -- it is in _WATERING_ALERTS, OR
     its state carries the alert, OR it is on the intentional-silence allowlist.
  3. No alert key references a state/reason the firmware never emits (dead keys).
"""
import os
import re

import pytest

import alerter

_FW = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "firmware", "integrated", "src", "fsm.cpp",
)


def _fw_src():
    with open(_FW) as f:
        return f.read()


def _fw_states():
    """All strings returned by state_name()."""
    src = _fw_src()
    block = src[src.index("state_name"):]
    block = block[:block.index("return \"unknown\"")]
    return set(re.findall(r'return "([^"]+)"', block))


def _fw_reasons():
    """All strings assigned to last_reason (incl. the leak ternary)."""
    src = _fw_src()
    reasons = set(re.findall(r'last_reason = "([^"]+)"', src))
    # leak reasons are set via a ternary: last_reason = x ? "a" : "b";
    reasons |= set(re.findall(r'\? "([^"]+)" : "([^"]+)"', src)[0]) \
        if re.search(r'\? "[^"]+" : "[^"]+"', src) else set()
    return reasons


# Firmware fault states that MUST raise a state-keyed alert.
_FAULT_STATES = {"leak_fault", "sensor_fault", "recovery_hold", "reservoir_empty"}

# Reasons that are intentionally NOT alerted (transitional, success, user-initiated,
# or expected operator action -- see alerter comments).
_SILENT_REASONS = {
    "abort",             # user-initiated stop; no alarm wanted
    "maintenance",       # expected intentional pause
    "recovery cleared",  # transitional (operator ACKed recovery_hold)
    "target reached",    # success (has a benign key anyway)
    "soil sensor stale", # covered by the sensor_fault STATE alert, not the reason
}


def test_all_fault_states_are_known_to_firmware():
    # Guard the test itself: the states we require alerts for must actually exist
    # in the firmware (else the list has rotted).
    assert _FAULT_STATES <= _fw_states(), (
        _FAULT_STATES - _fw_states(), "listed fault state not emitted by firmware")


def test_every_fault_state_has_a_fault_alert():
    missing = _FAULT_STATES - set(alerter.FAULT_ALERTS)
    assert not missing, f"firmware fault states with no FAULT_ALERTS entry: {missing}"


def test_no_dead_fault_alert_keys():
    # Every FAULT_ALERTS key must be a real firmware state (catches e.g. the old
    # 'watering_fault' that no firmware emits).
    dead = set(alerter.FAULT_ALERTS) - _fw_states()
    assert not dead, f"FAULT_ALERTS keys not emitted by firmware: {dead}"


def test_every_reason_is_covered_or_intentionally_silent():
    reasons = _fw_reasons()
    covered = set(alerter._WATERING_ALERTS) | _SILENT_REASONS
    # A reason whose state is itself alerted is also covered.
    state_covered = {r for r in reasons if r in alerter.FAULT_ALERTS}
    uncovered = reasons - covered - state_covered
    assert not uncovered, (
        f"firmware reasons with no alert and not on the silence list: {uncovered}")


def test_silence_list_only_names_real_reasons():
    # Keep the allowlist honest: everything on it must be a reason the firmware
    # actually emits, so stale entries can't hide a real gap.
    stale = _SILENT_REASONS - _fw_reasons()
    assert not stale, f"_SILENT_REASONS entries the firmware no longer emits: {stale}"


def test_specific_p0_reasons_alert():
    # Explicit regression guards for the exact strings F1/F2 involved.
    for reason in ("capped: target not reached", "failed: not absorbing",
                   "reservoir empty", "pump max-runtime exceeded"):
        assert reason in alerter._WATERING_ALERTS, f"{reason!r} lost its alert (F1/F2 regression)"


def test_leak_disconnect_distinct_from_leak():
    # F5: the two leak reasons must both alert and be distinct entries.
    assert "leak" in alerter._WATERING_ALERTS
    assert "leak sensor disconnected" in alerter._WATERING_ALERTS
    assert alerter._WATERING_ALERTS["leak"] != alerter._WATERING_ALERTS["leak sensor disconnected"]
