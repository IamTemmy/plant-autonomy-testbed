# hub/08-grow-light — grow-light photoperiod

Three layers keep the grow light on its 07:00 ON / 19:00 OFF photoperiod:

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

## 3. On-device WiFi self-recovery (DL-085) — last-resort autonomy
Layers 1–2 both assume the Pi can *reach* the plug. But the Shelly has been seen
to drop off WiFi and not auto-reconnect — unreachable for hours while still
powered — and when that happens the Pi enforcer is helpless, because its only
channel to the plug is the network that is down (it just logs `No route to host`
every tick while the light stays stuck). Recovery therefore has to run **on the
device itself**, independent of the network:

- **`wifi-watchdog.js`** runs on the Shelly, checks WiFi every 60 s, and after
  ~5 minutes of sustained disconnection (`FAILS_TO_REBOOT = 5`) reboots the plug
  so it rejoins fresh. It auto-starts on boot (surviving its own reboots) and is
  conservative: the fail counter resets the instant WiFi returns, so brief blips
  never trigger a reboot loop. After a reboot the switch restores its prior state
  (`initial_state = restore_last`) and the layer-2 enforcer re-asserts the
  correct state within ~2 min.
- A blind **daily 02:00 `Shelly.Reboot` schedule** (configured on the device, not
  a repo file) backs this up, clearing any half-open WiFi state the check might
  miss — the light is already off at 02:00, so there is zero plant impact.

Install the watchdog from the Pi over the Shelly's local RPC — which works even
under campus client isolation (DL-028), since the Pi can reach the plug when a
browser cannot. `install_wifi_watchdog.py` is idempotent (re-running stops and
replaces the on-device script in place) and handles two Gen4 quirks: it scrubs
the source to ASCII (`Script.PutCode` rejects non-ASCII) and uploads in
≤512-byte chunks (per-call code-length cap).

```bash
python3 install_wifi_watchdog.py            # uses SHELLY_HOST (default 10.6.17.32)
```

Status: installed and running; the daily 02:00 reboot is confirmed firing
nightly, while the reactive WiFi-loss path awaits a real outage to exercise it.
