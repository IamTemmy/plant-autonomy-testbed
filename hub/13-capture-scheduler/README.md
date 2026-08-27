# 13 — Capture Orchestrator

Ties the vision-phase capture pipeline together (DL-200). A `oneshot` service run
every minute by a systemd timer, it decides whether to trigger a camera capture
and publishes `plant/cmd/capture` with a context the receiver tags:

- **`dark`** — at a dark-window center (08:00 / 10:00 / 12:00 / 14:00 / 16:00),
  where the grow-light enforcer holds the light OFF (DL-199) so the frame is free
  of the close grow-light's specular glare. These feed the calibrated green metric.
- **`lit`** — every 30 min through the lit day (`:00` and `:30`), light on. A
  visual/time-lapse record only; **not** fed to the calibrated metric.

## Single authority, pure trigger

The orchestrator **never sets the plug** — the photoperiod enforcer is the sole
setter (D1). Before a `dark` capture it *reads* the plug state (a read-only Shelly
RPC) and only triggers if the light is confirmed OFF; if the enforcer hasn't caught
up yet it **skips that window and logs**, rather than capturing a lit frame
mislabelled `dark`. Missing an occasional dark capture is harmless (five windows a
day, and the metric tolerates gaps); a second plug-controller would not be.

The dark-window centers are read from the **same** `DARK_WINDOW_CENTERS` env as the
enforcer, so the two services cannot drift. A dark center takes precedence over the
`:00` lit slot. Outside the lit day the orchestrator does nothing — the node's own
2 h fallback still covers liveness.

## Files

- `capture_scheduler.py` — the orchestrator. `decide(now, centers)` is the pure
  scheduling core (unit-tested in `tests/test_capture_scheduler.py`); the plug
  query and MQTT publish are the I/O around it.
- `plant-capture-scheduler.service` / `.timer` — oneshot + `OnCalendar=*:0/1`.

## Deploy

```
scp capture_scheduler.py basilpi@<pi>:/home/basilpi/plant-hub/
scp plant-capture-scheduler.{service,timer} basilpi@<pi>:/etc/systemd/system/
ssh basilpi@<pi> 'sudo systemctl daemon-reload && sudo systemctl enable --now plant-capture-scheduler.timer'
```

Env (from `/etc/plant-hub/credentials`): `MQTT_USER` / `MQTT_PASS`, `SHELLY_HOST`,
`DARK_WINDOW_CENTERS`, `GROW_ON_HOUR` / `GROW_OFF_HOUR`, `LOCAL_TZ`.
