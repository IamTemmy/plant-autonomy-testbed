#!/usr/bin/env python3
"""
Hub-side photoperiod enforcement for the grow light.

The Shelly Plug runs an onboard 07:00-on / 19:00-off schedule (DL-054), but the
plug sits on flaky campus WiFi (DL-028) and reboots frequently (DL-070) -- and a
reboot at the wrong moment makes its onboard scheduler miss the trigger (observed
2026-06-16: the light never came on at 07:00). The Shelly's clock and scheduler
are only as reliable as the plug. The Pi is always-on with an NTP-correct clock
and reaches the plug over HTTP RPC even while its MQTT/WiFi is flapping.

This controller asserts the correct state from the Pi every couple of minutes:
inside the photoperiod window -> ensure ON, outside -> ensure OFF. It is
idempotent (calls Switch.Set only when the plug is actually in the wrong state)
and symmetric (hard-enforces OFF at night too). Because it re-asserts
continuously, any reboot-induced wrong state self-heals within one tick -- which
turns the DL-063 "grow light may be off" alert from a call to action into a
notice of a self-correcting event. The Shelly's onboard schedule (DL-054) is
kept as a fast-path fallback; the two agree on intent and the hub is the
enforcer.

Run every ~2 min via plant-photoperiod.timer. Logs only when it corrects the
plug or hits an error, so the journal doubles as a record of how often the plug
was found in the wrong state.
"""
import json
import os
import sys
import urllib.request
from datetime import datetime
from zoneinfo import ZoneInfo

SHELLY_HOST = os.environ.get("SHELLY_HOST", "10.6.17.32")
LOCAL_TZ = ZoneInfo(os.environ.get("LOCAL_TZ", "America/Chicago"))
ON_HOUR = int(os.environ.get("GROW_ON_HOUR", "7"))
OFF_HOUR = int(os.environ.get("GROW_OFF_HOUR", "19"))
TIMEOUT = float(os.environ.get("SHELLY_POLL_TIMEOUT", "4"))

# Dark capture windows (DL-199, vision phase Piece 3). During these short windows
# the grow-light is held OFF so the top-down camera captures the plant free of the
# specular glare the close grow-light throws on the leaves (which corrupts the
# green metric). The windows are the SINGLE source of truth shared with the Pi
# capture orchestrator (Piece 4): centers here, and the orchestrator fires its
# capture at the center once the light is confirmed off. Default = 5 windows
# centered on the even hours 08:00..16:00, each running center-2min .. center+3min
# (i.e. 07:58-08:03, ...), matching the agreed schedule.
DARK_WINDOW_CENTERS  = os.environ.get("DARK_WINDOW_CENTERS", "08:00,10:00,12:00,14:00,16:00")
DARK_WINDOW_LEAD_MIN = int(os.environ.get("DARK_WINDOW_LEAD_MIN", "2"))   # off this many min before center
DARK_WINDOW_TAIL_MIN = int(os.environ.get("DARK_WINDOW_TAIL_MIN", "3"))   # held off this many min after center


def _parse_centers(spec):
    """Parse "HH:MM,HH:MM,..." into a sorted list of minutes-of-day. Bad entries
    are skipped with a warning rather than crashing the enforcer."""
    out = []
    for part in spec.split(","):
        part = part.strip()
        if not part:
            continue
        try:
            hh, mm = part.split(":")
            out.append(int(hh) * 60 + int(mm))
        except (ValueError, IndexError):
            log(f"ignoring malformed dark-window center {part!r}")
    return sorted(out)


DARK_CENTERS = None  # lazily parsed on first use (log() needs to be defined first)


def log(msg):
    ts = datetime.now(LOCAL_TZ).strftime("%Y-%m-%d %H:%M:%S %Z")
    print(f"{ts} [photoperiod] {msg}", flush=True)


def rpc(path):
    with urllib.request.urlopen(f"http://{SHELLY_HOST}/rpc/{path}", timeout=TIMEOUT) as r:
        return json.load(r)


def desired_on(hour):
    # Normal window (ON < OFF) or an overnight window that wraps midnight.
    if ON_HOUR < OFF_HOUR:
        return ON_HOUR <= hour < OFF_HOUR
    return hour >= ON_HOUR or hour < OFF_HOUR


def in_dark_window(now=None):
    """True if `now` falls inside a dark capture window (DL-199). Minute-precise:
    a window runs [center - LEAD, center + TAIL] minutes, inclusive. Parses the
    centers lazily on first call (so log() exists for malformed-entry warnings)."""
    global DARK_CENTERS
    if DARK_CENTERS is None:
        DARK_CENTERS = _parse_centers(DARK_WINDOW_CENTERS)
    if now is None:
        now = datetime.now(LOCAL_TZ)
    mod = now.hour * 60 + now.minute
    for c in DARK_CENTERS:
        if c - DARK_WINDOW_LEAD_MIN <= mod <= c + DARK_WINDOW_TAIL_MIN:
            return True
    return False


def grow_light_should_be_on(now):
    """The single decision: ON only when it's the lit part of the day AND we're
    not inside a dark capture window. Keeping desired_on(hour) unchanged means a
    dark window is a narrow, intentional OFF carved out of the lit day, not a
    change to the photoperiod itself."""
    return desired_on(now.hour) and not in_dark_window(now)


def main():
    now = datetime.now(LOCAL_TZ)
    hour = now.hour
    dark = in_dark_window(now)
    want = desired_on(hour) and not dark
    try:
        cur = bool(rpc("Switch.GetStatus?id=0")["output"])
    except Exception as e:
        log(f"shelly unreachable, will retry next tick: {e}")
        return 0
    if cur == want:
        return 0  # already correct -- stay quiet
    try:
        rpc(f"Switch.Set?id=0&on={'true' if want else 'false'}")
    except Exception as e:
        log(f"correction FAILED (wanted {'ON' if want else 'OFF'}): {e}")
        return 1
    if dark and not want:
        # Intentional OFF for a capture window -- logged distinctly from a real
        # correction so the journal doesn't read it as the plug misbehaving.
        log(f"dark capture window at {now:%H:%M}: holding grow-light OFF "
            f"(was {'ON' if cur else 'OFF'}) -> set OFF")
    else:
        log(f"corrected: light was {'ON' if cur else 'OFF'}, photoperiod wants "
            f"{'ON' if want else 'OFF'} at hour {hour} -> set")
    return 0


if __name__ == "__main__":
    sys.exit(main())
