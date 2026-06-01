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
