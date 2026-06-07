# 02 — Mosquitto Installation

Installs the Mosquitto MQTT broker on the Raspberry Pi hub. Configuration and broker testing are pending and will be appended to this directory in a subsequent commit.

## Installation procedure

From an SSH session on the Pi:

```text
sudo apt update
sudo apt install -y mosquitto mosquitto-clients
```

`mosquitto` provides the broker daemon. `mosquitto-clients` provides the `mosquitto_pub` and `mosquitto_sub` command-line tools used to verify the broker is working before any ESP32 or Shelly client connects to it.

## Current state

- Both packages installed via apt
- Broker daemon's default service may already be running, but no configuration has been written and no end-to-end test has been performed
- The broker is not yet bound to a known port/interface intentionally — that decision is the next step

## What is not yet done

- `mosquitto.conf` written and verified
- Listener bound to the desired interface (LAN-accessible)
- Authentication strategy chosen (open for LAN-only development, or username/password from the start)
- End-to-end test: `mosquitto_sub` on the Pi, `mosquitto_pub` from the developer machine, message delivery confirmed
- Broker survives reboot (systemd enable/disable confirmed)

These are the next-session tasks. The procedure and configuration file will land in this directory once executed.

## Broker verification — loopback pub/sub test

Verified end-to-end on 2026-05-31 with two SSH sessions to the Pi.

### Procedure

1. Confirm the broker service is running:

       systemctl status mosquitto

   Expected: `Active: active (running)` in the status output.

2. In SSH session 1 (the subscriber), listen on a test topic:

       mosquitto_sub -h localhost -t test/hello -v

   The `-v` flag prints the topic name alongside each message payload. The command hangs and waits for messages.

3. In SSH session 2 (the publisher), send messages:

       mosquitto_pub -h localhost -t test/hello -m "broker says hi"
       mosquitto_pub -h localhost -t test/hello -m "second message"
       mosquitto_pub -h localhost -t test/hello -m "third"

4. Confirm each message appears in session 1 immediately after being sent.

### Result

All three test messages were routed by the broker from publisher to subscriber with no observable delay. The Mosquitto broker is verified operational on the Pi.

Evidence: `docs/images/mosquitto-loopback-verification.png`

### What this verifies

- The Mosquitto systemd service is active and accepting connections on `localhost`.
- The broker correctly routes messages between independent clients via the publish/subscribe pattern.
- The default Mosquitto configuration (no custom `mosquitto.conf` written yet) is sufficient for loopback testing.

### What this does not yet verify

- **LAN accessibility.** The broker is currently only verified on `localhost`. Other devices on the same network — the ESP32 nodes and the Shelly smart plug — have not yet been tested as MQTT clients. That requires either binding the broker explicitly to the LAN interface (or `0.0.0.0`) and confirming the listener configuration is correct, which is the next step.
- **Authentication.** The broker is currently open — anyone on the LAN who can reach the Pi on port 1883 can publish and subscribe. For LAN-only development this is acceptable; if the broker ever becomes reachable from outside the LAN (Cloudflare Tunnel, Tailscale, port forward), authentication must be enabled first.
- **Persistence across reboots.** Whether `mosquitto.service` is enabled to start automatically on boot has not been verified. Reboot the Pi and confirm the broker comes back up before declaring this step fully complete.