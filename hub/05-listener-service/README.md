# 05 — Listener as a systemd service

Promotes the Phase 3 listener (`hub/04-listener/listener.py`) from "I have
to run it manually in a terminal" to "it runs as a permanent system service
that survives reboots, crashes, and network interruptions."

See [DL-036](../../docs/decision-log.md) for design rationale and validation.

## Files

| File | Purpose |
|---|---|
| `plant-listener.service` | systemd unit file (copy into `/etc/systemd/system/`) |
| `README.md` | This file |

## What this layer does

- Runs `listener.py` as the `basilpi` user (not root)
- Starts automatically on boot, after `mosquitto.service`
- Restarts automatically on crash (10-second delay)
- Routes listener logs to systemd's journal
- Loads MQTT credentials from `/etc/plant-hub/credentials` (root-only)

## Setup procedure on the Pi

### 1. Create the credentials file

```text
sudo mkdir -p /etc/plant-hub
sudo chmod 700 /etc/plant-hub
sudo nano /etc/plant-hub/credentials
```

Contents (systemd EnvironmentFile syntax — no quotes, no `export`):

```text
MQTT_USER=basilmqtt
MQTT_PASS=<the current MQTT password>
RUN_PHASE=phase3_hub_service
```

Optional — push notifications via [ntfy](https://ntfy.sh) (DL-061). Alerting is
off unless `NTFY_TOPIC` is set; if set, the listener pushes for genuine problems
only (leak, watering fault, reservoir empty, prolonged offline, flapping reboots)
plus recoveries and a daily summary. The topic is a shared secret, so it lives
here and never in the repo:

```text
NTFY_TOPIC=<your-long-unguessable-topic-name>
# Optional overrides (defaults shown):
# NTFY_SERVER=https://ntfy.sh
# ALERT_OFFLINE_GRACE_S=300
# HEARTBEAT_HOUR=9
# LOCAL_TZ=America/Chicago
```

Lock it down:

```text
sudo chmod 600 /etc/plant-hub/credentials
sudo chown root:root /etc/plant-hub/credentials
```

### 2. Install the service unit

Copy `plant-listener.service` from this repo to `/etc/systemd/system/`:

```text
sudo cp plant-listener.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable plant-listener.service
sudo systemctl start plant-listener.service
```

### 3. Verify

```text
sudo systemctl status plant-listener.service
```

Expected: `Active: active (running)`.

For the listener's log output (Python prints, errors, reconnection notices):

```text
sudo journalctl -u plant-listener -f
```

Press Ctrl+C to stop following.

## Operational reference

| Task | Command |
|---|---|
| Start service | `sudo systemctl start plant-listener.service` |
| Stop service (graceful) | `sudo systemctl stop plant-listener.service` |
| Restart service | `sudo systemctl restart plant-listener.service` |
| Check status | `sudo systemctl status plant-listener.service` |
| Follow logs (live) | `sudo journalctl -u plant-listener -f` |
| Recent logs (last 50) | `sudo journalctl -u plant-listener -n 50 --no-pager` |
| Disable auto-start at boot | `sudo systemctl disable plant-listener.service` |
| Re-enable auto-start at boot | `sudo systemctl enable plant-listener.service` |

## Verifying auto-restart works

Note the PID, kill the process, confirm a new PID appears within ~15 seconds:

```text
sudo systemctl show plant-listener.service --property=MainPID
sudo kill -9 <PID from above>
sleep 15
sudo systemctl show plant-listener.service --property=MainPID
```

Different PID = restart works. The `runs` table records both the killed run
(with `ended_ts` NULL — forensic signal that it crashed) and the new run
(with `ended_ts` NULL while running).

## Credential rotation procedure

When the MQTT password needs to be rotated:

1. Set new password on the broker:
```text
   sudo mosquitto_passwd -b /etc/mosquitto/passwd basilmqtt "NEW-PASSWORD"
   sudo systemctl restart mosquitto
```

2. Update the credentials file:
```text
   sudo nano /etc/plant-hub/credentials
```

3. Restart the listener so it picks up the new credentials:
```text
   sudo systemctl restart plant-listener.service
```

4. Update the Shelly via its web UI (Settings → MQTT → new password → Save → Reboot now).

The credentials file is never typed at a shell prompt and never committed to
the repo. This is the security boundary that makes the rotation queue tractable
going forward.

## What this layer does not yet handle

- **No metrics export.** Listener has no Prometheus endpoint; system health
  is observed via `journalctl` and direct SQLite queries.
- **No log rotation.** systemd journal handles its own rotation by default
  (size-capped); not configured for explicit retention policy.
- **No backup of `plant.db`.** SQLite file lives on the Pi's SD card; a card
  failure loses all collected data. Acceptable for a research project;
  revisit if data becomes load-bearing.
