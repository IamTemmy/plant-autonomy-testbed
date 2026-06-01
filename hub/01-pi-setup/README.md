# 01 — Raspberry Pi Setup

Bootstraps the Raspberry Pi 4 that will run the project's hub services: Mosquitto MQTT broker, Python data services, SQLite database, and the Streamlit dashboard.

## Hardware

- Raspberry Pi 4 Model B
- SanDisk 64 GB microSD card (older cards rejected: 1 GB too small; 16 GB tight for long-term sensor data growth)
- Pi 4 official power supply
- Network access during setup

## Setup procedure

1. **Flash the OS to the SD card** using [Raspberry Pi Imager](https://www.raspberrypi.com/software/) on the developer machine (macOS).
   - OS: Raspberry Pi OS Lite (64-bit) — headless, no desktop
   - Pre-configure during flash (gear icon in Imager): hostname, username, password, SSH enabled, WiFi SSID + password, locale
2. **First boot.** Insert the flashed card into the Pi, connect power. Allow ~1–2 minutes for first-boot expansion.
3. **Locate the Pi on the network.** From the dev machine:
   - `ping <hostname>.local` should resolve via mDNS
   - If that fails, scan the local network with `arp -a` or use the router's client list
4. **SSH in:** `ssh <user>@<hostname>.local`
5. **Update the system:**
```text
   sudo apt update
   sudo apt upgrade -y
```

## Current state

- Pi flashed with Raspberry Pi OS Lite (64-bit)
- SSH operational from developer machine
- `apt update` and `apt upgrade` completed
- WiFi: working on **home network** (development environment)
- WiFi for the **deployment target (campus)**: pending — initial attempt suggested a client-isolation policy on the campus WiFi (devices on the network cannot reach each other directly, even on the same SSID). This blocks SSH and MQTT entirely. Status to be confirmed with campus IT before further work on deployment networking. Possible mitigations if confirmed: a different network segment, Ethernet, or a personal hotspot for the deployed system.

## What is not yet done

- Static IP assignment (will be required before either ESP32 node can connect to the broker reliably)
- Hostname customization (currently default)
- Any service installation beyond Mosquitto

## Replication notes

If reproducing this build, expect WiFi to be the most likely source of friction — institutional and shared networks frequently have client isolation, captive portals, or MAC-registration requirements that make IoT-style device-to-device communication impossible without IT involvement. Plan to validate the network early; do not assume "if SSH works, MQTT will work."
