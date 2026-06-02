# 03 — Mosquitto Broker LAN Configuration

Configures the Mosquitto MQTT broker to accept authenticated connections from any device on the LAN, not just from the Pi itself. This is the step that makes the broker usable by the Shelly smart plug and the ESP32 nodes.

## What this directory contains

- `plant-autonomy.conf` — the broker config file, copied verbatim from what's deployed at `/etc/mosquitto/conf.d/plant-autonomy.conf` on the Pi. Safe to commit (no secrets).

## What this directory deliberately does NOT contain

- The password file (`/etc/mosquitto/passwd`) — even though the credentials in it are hashed, hashes can be attacked offline. The password file lives only on the Pi and is generated locally per the procedure below.
- The MQTT username and password used by ESP32 firmware and the Shelly — those go in gitignored secrets files in firmware, never in committed source.

## Procedure

On the Pi via SSH, in this order. Substitute your chosen password where indicated.

### 1. Create the password file

```text
sudo mosquitto_passwd -c /etc/mosquitto/passwd basilpi
```

Enter and confirm your chosen password. The `-c` flag creates the file from scratch. To later **rotate** the password for the same user, run the same command **without `-c`** — using `-c` would wipe and recreate the file, dropping all entries.

### 2. Set file ownership and permissions

```text
sudo chown root:mosquitto /etc/mosquitto/passwd
sudo chmod 640 /etc/mosquitto/passwd
```

This satisfies both requirements: `mosquitto_passwd` wants the file owned by `root` (recent versions warn otherwise), and the broker daemon (which runs as the `mosquitto` user) needs read access via the group.

Verify with:

```text
ls -l /etc/mosquitto/passwd
```

Expected: `-rw-r----- 1 root mosquitto ...`

### 3. Install the broker configuration

Copy `plant-autonomy.conf` from this directory to the Pi at `/etc/mosquitto/conf.d/plant-autonomy.conf`. The file in `conf.d/` is automatically loaded by Mosquitto alongside the default config.

### 4. Restart the broker

```text
sudo systemctl restart mosquitto
systemctl status mosquitto
```

Expected: `Active: active (running)` in green. If it shows `failed`, run `sudo journalctl -eu mosquitto.service -n 20 --no-pager` and address the error before proceeding.

### 5. Confirm boot persistence

```text
sudo systemctl is-enabled mosquitto
```

Should print `enabled`. If `disabled`, run `sudo systemctl enable mosquitto`.

## Verification — three tests in increasing scope

### Test 1: anonymous rejected

On the Pi:

```text
mosquitto_sub -h localhost -t test/hello -v
```

Expected: immediate failure with `Connection error: Connection Refused: not authorised.` This confirms authentication is enforced.

### Test 2: authenticated loopback works

On the Pi, in two SSH sessions. Session 1:

```text
mosquitto_sub -h localhost -t test/hello -u basilpi -P 'YOUR-PASSWORD' -v
```

Session 2:

```text
mosquitto_pub -h localhost -t test/hello -m "auth from Pi" -u basilpi -P 'YOUR-PASSWORD'
```

Expected: the message appears in session 1 immediately. Ctrl+C to stop.

### Test 3: LAN access from an external client

From the developer machine (Mac in this build), with `mosquitto-clients` installed (`brew install mosquitto`):

```text
mosquitto_sub -h 10.6.19.139 -t test/hello -u basilpi -P 'YOUR-PASSWORD' -v
```

On the Pi:

```text
mosquitto_pub -h 10.6.19.139 -t test/hello -m "Mac sees Pi broker over LAN" -u basilpi -P 'YOUR-PASSWORD'
```

Expected: the message appears on the Mac. This is the test that proves the architecture — the broker is reachable, authenticated, and operational from any device on the LAN, which is what the Shelly and ESP32 nodes will need.

## Troubleshooting

A few errors encountered and resolved during initial bring-up; recording so they don't have to be re-diagnosed:

- **`Duplicate persistence_location value in configuration.`** Re-declaring `persistence_location` in this config conflicts with the default `mosquitto.conf`. Solution: only override settings that actually differ from the defaults. Persistence stays default.
- **`mosquitto.service: Main process exited, code=exited, status=13`** with no obvious file in the log. Status 13 is `EACCES` (permission denied). Almost always means the broker can't read the password file. Verify ownership (`root:mosquitto`) and permissions (`640`).
- **`Warning: File /etc/mosquitto/passwd owner is not root.`** From `mosquitto_passwd`. Tool wants `root` ownership; broker needs read access. Resolution: `root:mosquitto` ownership + `640` permissions satisfies both.

## Not yet verified

- **Reboot persistence.** The broker is `enabled` in systemd, but a power-cycle test to confirm it actually comes back without intervention has not been performed. Scheduled as a small verification before the Shelly integration.
- **External client at scale.** A single Mac client has connected; the broker has not yet been tested with multiple simultaneous clients (Shelly + ESP32 + dashboard). Confidence is high based on Mosquitto's general behavior, but not formally validated.
- **TLS or any encryption.** The current setup uses plaintext MQTT on port 1883. For LAN-only operation on a trusted network this is acceptable; before any external exposure (Cloudflare Tunnel, Tailscale), TLS must be added.
