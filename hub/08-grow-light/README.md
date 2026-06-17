# hub/08-grow-light — grow-light photoperiod

Two layers control the grow light's 07:00 ON / 19:00 OFF photoperiod:

## 1. Shelly onboard schedule (DL-054) — fast-path fallback
`set-schedule.sh <SHELLY_IP>` writes a built-in scheduler to the Shelly Plug
(07:00 ON, 19:00 OFF) so the light keeps its photoperiod even if the Pi, the
WROVER, or the network is down. Requires the Shelly's timezone to be set so it
fires in local time. This runs device-side and is the fallback.

## 2. Hub enforcement (DL-074) — the reliability layer
`photoperiod.py`, run every 2 minutes by `plant-photoperiod.timer`, asserts the
correct state from the Pi: inside the window -> ensure ON, outside -> ensure OFF
(symmetric). It exists because the Shelly sits on flaky campus WiFi (DL-028) and
reboots frequently (DL-070); a reboot at the wrong moment makes its onboard
scheduler miss the trigger (observed 2026-06-16: the light never came on at
07:00, and the DL-063 alert fired but there was nothing to act on automatically).

The Pi is always-on with an NTP-correct clock and reaches the plug over HTTP RPC
even while its MQTT/WiFi is flapping. Because the controller re-asserts every
~2 min:
- A 07:00 reboot can no longer cost the day — the light is corrected within one
  tick (worst-case ~2 min of darkness, trivial for the plant's DLI).
- Any reboot-induced wrong state self-heals, so the DL-063 "grow light may be
  off" alert becomes a *notice of a self-correcting event* rather than a call to
  press the button.

The controller is **idempotent** (it reads `Switch.GetStatus` and only calls
`Switch.Set` when the plug is actually wrong) and **quiet** (it logs only when it
corrects the plug or hits an error). That makes the journal a running record of
how often the plug was found in the wrong state — useful telemetry while the
plug's reboot problem is monitored:

```bash
journalctl -u plant-photoperiod.service --no-pager | grep -i corrected
```

Env knobs (in `/etc/plant-hub/credentials`): `GROW_ON_HOUR` (7), `GROW_OFF_HOUR`
(19), `LOCAL_TZ` (America/Chicago), `SHELLY_HOST` (10.6.17.32).

### Install
```bash
scp photoperiod.py basilpi@<pi>:/home/basilpi/plant-hub/photoperiod.py
scp plant-photoperiod.service plant-photoperiod.timer basilpi@<pi>:/tmp/
ssh -t basilpi@<pi> "sudo cp /tmp/plant-photoperiod.{service,timer} /etc/systemd/system/ \
  && sudo systemctl daemon-reload \
  && sudo systemctl enable --now plant-photoperiod.timer"
```
