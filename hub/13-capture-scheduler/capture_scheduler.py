#!/usr/bin/env python3
"""Capture orchestrator (DL-200, vision phase Piece 4).

Runs once per minute (systemd timer) and decides whether to trigger a camera
capture, publishing plant/cmd/capture with a context the receiver tags:

  * "dark"  -- at a dark-window center (08:00/10:00/12:00/14:00/16:00), the
              grow-light is held OFF by the enforcer (DL-199) so the frame is
              free of specular glare. These feed the calibrated metric. The
              orchestrator confirms the plug is actually OFF first (read-only
              query -- it never SETS the plug; the enforcer is the sole setter,
              D1); if the light is not off yet it SKIPS the window and logs,
              rather than capturing a lit frame mislabelled "dark".
  * "lit"   -- every 30 min through the lit day (:00 and :30), light on. A
              visual/time-lapse record only; NOT fed to the calibrated metric.

Dark centers are read from the SAME env (DARK_WINDOW_CENTERS) as the enforcer,
so the two services cannot drift. A dark center takes precedence over the :00
lit slot. Outside the lit day the orchestrator does nothing (the node's own 2 h
fallback still covers liveness).

This service only PUBLISHES a trigger; the node performs the capture+POST and the
receiver stores/tags it. Keeping the orchestrator a pure trigger (no plug writes,
no image handling) is what keeps the single-authority model intact.
"""
import json
import os
import sys
import urllib.request
from datetime import datetime
from zoneinfo import ZoneInfo

import paho.mqtt.publish as mqtt_publish

LOCAL_TZ = ZoneInfo(os.environ.get("LOCAL_TZ", "America/Chicago"))
ON_HOUR = int(os.environ.get("GROW_ON_HOUR", "7"))
OFF_HOUR = int(os.environ.get("GROW_OFF_HOUR", "19"))

SHELLY_HOST = os.environ.get("SHELLY_HOST", "10.6.17.32")
SHELLY_TIMEOUT = float(os.environ.get("SHELLY_POLL_TIMEOUT", "4"))

CAPTURE_CMD_TOPIC = os.environ.get("CAPTURE_CMD_TOPIC", "plant/cmd/capture")
MQTT_HOST = os.environ.get("MQTT_HOST", "localhost")

# Shared with the enforcer (DL-199): the dark-window centers, single source of truth.
DARK_WINDOW_CENTERS = os.environ.get("DARK_WINDOW_CENTERS", "08:00,10:00,12:00,14:00,16:00")

# Lit time-lapse cadence: the minutes-of-hour at which to fire a "lit" capture.
LIT_MINUTES = (0, 30)


def log(msg):
    stamp = datetime.now(LOCAL_TZ).strftime("%Y-%m-%d %H:%M:%S %Z")
    print(f"{stamp} [capture-scheduler]{msg}", flush=True)


def parse_centers(spec):
    """"HH:MM,..." -> sorted minutes-of-day; malformed entries skipped with a log."""
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


def is_lit_day(hour):
    """True if `hour` is inside the grow-light day window (matches the enforcer)."""
    if ON_HOUR < OFF_HOUR:
        return ON_HOUR <= hour < OFF_HOUR
    return hour >= ON_HOUR or hour < OFF_HOUR


def decide(now, centers):
    """Pure decision: what context to capture at `now`, or None to do nothing.
    A dark center wins over the :00 lit slot. Returns "dark" | "lit" | None."""
    if not is_lit_day(now.hour):
        return None
    mod = now.hour * 60 + now.minute
    if mod in centers:
        return "dark"
    if now.minute in LIT_MINUTES:
        return "lit"
    return None


def light_is_off():
    """Read-only query of the Shelly plug. True if OFF. Raises on transport error
    (caller treats an unknown state as 'not confirmed off')."""
    url = f"http://{SHELLY_HOST}/rpc/Switch.GetStatus?id=0"
    with urllib.request.urlopen(url, timeout=SHELLY_TIMEOUT) as r:
        return not bool(json.load(r)["output"])


def publish(context):
    user = os.environ.get("MQTT_USER")
    password = os.environ.get("MQTT_PASS")
    if not user or not password:
        log("MQTT credentials missing (MQTT_USER/MQTT_PASS); cannot publish")
        return False
    mqtt_publish.single(
        CAPTURE_CMD_TOPIC, context, hostname=MQTT_HOST,
        auth={"username": user, "password": password},
        retain=False,   # a retained capture cmd would replay on every reconnect
    )
    return True


def main():
    now = datetime.now(LOCAL_TZ)
    centers = parse_centers(DARK_WINDOW_CENTERS)
    context = decide(now, centers)
    if context is None:
        return 0

    if context == "dark":
        # Only capture if the enforcer has actually taken the light off. Querying
        # the plug is read-only -- we never set it (the enforcer is sole setter).
        try:
            off = light_is_off()
        except Exception as e:
            log(f"dark window {now:%H:%M}: plug state unknown ({e}); skipping")
            return 0
        if not off:
            log(f"dark window {now:%H:%M}: light still ON (enforcer not caught up); skipping")
            return 0

    if publish(context):
        log(f"triggered capture [{context}] at {now:%H:%M}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
