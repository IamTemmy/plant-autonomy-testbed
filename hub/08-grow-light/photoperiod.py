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


def main():
    hour = datetime.now(LOCAL_TZ).hour
    want = desired_on(hour)
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
    log(f"corrected: light was {'ON' if cur else 'OFF'}, photoperiod wants "
        f"{'ON' if want else 'OFF'} at hour {hour} -> set")
    return 0


if __name__ == "__main__":
    sys.exit(main())
