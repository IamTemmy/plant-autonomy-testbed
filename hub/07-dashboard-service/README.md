# 07 — Dashboard as a systemd service

Promotes the Phase 3 Streamlit dashboard (`hub/06-dashboard/dashboard.py`) from
manual-launch to a permanent system service that auto-starts at boot and
auto-restarts on crash. Mirrors the listener-service pattern from
[DL-036](../../docs/decision-log.md) but without credentials (the dashboard
only reads SQLite, no MQTT auth needed).

See [DL-037](../../docs/decision-log.md) for the dashboard design.

## Files

| File | Purpose |
|---|---|
| `plant-dashboard.service` | systemd unit file (copy into `/etc/systemd/system/`) |
| `README.md` | This file |

## What this layer does

- Runs `streamlit run dashboard.py` as the `basilpi` user (not root)
- Binds to `0.0.0.0:8501` so the dashboard is LAN-accessible
- Starts automatically on boot, after `plant-listener.service` (soft dependency)
- Restarts automatically on crash (10-second delay)
- Routes Streamlit logs to systemd's journal
- Opts out of Streamlit's anonymous usage telemetry (`--browser.gatherUsageStats false`)

## Differences from the listener service

| Aspect | Listener service (DL-036) | Dashboard service (DL-037 follow-on) |
|---|---|---|
| Needs MQTT credentials | Yes — via EnvironmentFile | No — reads SQLite directly |
| Soft / hard dependencies | `Requires=mosquitto.service` | `After=plant-listener.service` only (soft) |
| Headless flags | N/A | `--server.headless true` (no auto-browser) |

The dashboard is intentionally a *soft* dependent of the listener. If the
listener is offline, the dashboard still loads — it just shows stale data
until the listener comes back. This is the right behavior for an observer.

## Setup procedure on the Pi

### 1. Install the service unit

```text
sudo cp plant-dashboard.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable plant-dashboard.service
sudo systemctl start plant-dashboard.service
```

### 2. Verify

```text
sudo systemctl status plant-dashboard.service
```

Expected: `Active: active (running)`. Press `q` to exit the pager.

Then browse to `http://10.6.19.139:8501` from any LAN device.

For Streamlit's log output:

```text
sudo journalctl -u plant-dashboard -f
```

## Operational reference

| Task | Command |
|---|---|
| Start | `sudo systemctl start plant-dashboard.service` |
| Stop (graceful) | `sudo systemctl stop plant-dashboard.service` |
| Restart | `sudo systemctl restart plant-dashboard.service` |
| Status | `sudo systemctl status plant-dashboard.service` |
| Follow logs | `sudo journalctl -u plant-dashboard -f` |
| Recent logs (50) | `sudo journalctl -u plant-dashboard -n 50 --no-pager` |
| Disable auto-start | `sudo systemctl disable plant-dashboard.service` |
| Re-enable auto-start | `sudo systemctl enable plant-dashboard.service` |

## Verifying auto-restart works

Same procedure as the listener:

```text
sudo systemctl show plant-dashboard.service --property=MainPID
sudo kill -9 <PID>
sleep 15
sudo systemctl show plant-dashboard.service --property=MainPID
```

Different PID after the kill confirms systemd restarted the service.

## Known noise in the logs

Streamlit emits deprecation warnings for `use_container_width=True` in newer
versions, recommending a new `width=...` parameter. These are warnings, not
errors; the dashboard still functions. To be cleaned up in a future polish
session.

## What this layer does not yet handle

- **No remote access.** LAN only. Tailscale comes next.
- **No HTTPS.** Plain HTTP, fine for LAN. Tailscale will handle the encryption
  layer when remote access lands.
- **No password / auth on the dashboard itself.** Acceptable while LAN-only.
