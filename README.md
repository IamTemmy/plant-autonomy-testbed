# Plant Autonomy Testbed

> A self-contained device that autonomously keeps a basil plant healthy — built as a demonstration of transferable autonomous-systems engineering.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-ESP32--WROVER-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Status](https://img.shields.io/badge/Status-Operational%20%C2%B7%20autonomous%20watering%20live-brightgreen.svg)](#project-status)

## Overview

The Plant Autonomy Testbed senses soil moisture, air conditions, and light, then waters a basil plant with a precisely metered dose **only when the soil actually needs it** — and verifies that each watering had an effect. It runs the grow light on a daily photoperiod, reports its status to a live remote dashboard, and degrades safely when a sensor, actuator, or the network fails, so the plant can be left unattended.

It originated from an engineering interview question — *"how would you design a system to autonomously care for a plant, including for when you travel?"* — and has been built into a complete, working answer, with every engineering decision recorded in a [decision log](docs/decision-log.md).

## Project status

The system is **operational and watering autonomously**. Development has run in phases:

| Phase | Scope | Status |
|---|---|---|
| **Phase 1** | Each hardware component validated in isolation (bench sketches) | ✅ Complete |
| **Phase 2** | Integrated ESP32-WROVER firmware — sensing, the watering state machine, dosing, fault detection | ✅ Complete |
| **Phase 3** | Raspberry Pi telemetry hub — MQTT broker, database, live dashboard | ✅ Complete |
| **Next** | Camera vision node, adaptive lighting, push alerting, security hardening | 🔜 Roadmap |

## Engineering framing

The plant is the demonstration vehicle. The point is the engineering pattern.

The system is built on the **sense → plan → act** autonomy loop common to robotics, aerospace, industrial automation, and self-driving vehicles. The principles it exercises are general:

- **Closed-loop verification** — every action is checked against its expected effect. If the pump runs but the soil never gets wetter, the system concludes something is wrong (empty reservoir, failed pump, dead float), stops, and raises a fault rather than pumping into the void.
- **Graceful degradation** — sensor or actuator failure does not produce unsafe behavior. Invalid sensor data halts watering; loss of WiFi/MQTT is non-fatal and the controller keeps running locally.
- **Defined fallback states** — leaks, emergency stop, and ineffective watering each latch into a known safe state (pump off) and wait for an explicit acknowledgement.
- **Explicit, inspectable behavior** — a state machine governs watering, and a 57-entry decision log makes every design choice auditable after the fact.

## System architecture

Four functional layers:

| Layer | Components |
|---|---|
| **Perception** | Capacitive soil-moisture sensor · BME280 (air temp/humidity/pressure) · BH1750 (light) · float switch (reservoir level) · conductive leak sensor |
| **Planning** | ESP32-WROVER firmware — a `millis()`-based, non-blocking state machine with threshold logic, closed-loop watering verification, fault detection, and safety-first evaluation |
| **Action** | 12 V peristaltic pump via a logic-level MOSFET · grow light on a Shelly Plug US Gen4 (device-side schedule over local RPC/MQTT) · OLED, traffic-light status LEDs, and buzzer for local feedback |
| **Telemetry** | WiFi → MQTT → Raspberry Pi 4 hub running Mosquitto, SQLite, and a Streamlit dashboard; remote access over Tailscale |

## What it does now

- **Autonomous watering** — waters when soil crosses the dry threshold, in calibrated pulse/settle doses, stopping when re-wetted; a daily volume cap bounds total water.
- **Closed-loop watering watchdog** — detects watering that isn't moving soil moisture and latches a `watering_fault` instead of running dry (catches an empty reservoir, dead pump, clog, or disconnected float).
- **Safety states** — leak detection, emergency stop, reservoir-empty, and daily-limit handling, surfaced on status LEDs, an OLED, and a buzzer.
- **Scheduled grow light** — a fixed daily photoperiod (07:00–19:00) run on the smart plug itself, so it holds even if the hub or network is down.
- **Live remote dashboard** — current state, sensor readings, a soil-moisture trend with watering episodes overlaid, and power telemetry.
- **Device-presence awareness** — if the controller drops offline, the dashboard detects it (via MQTT Last-Will), greys stale readings, and flags exactly when contact was lost.

## Dashboard

A read-only Streamlit dashboard on the Pi (LAN- and Tailscale-accessible) shows live state, sensor cards, and history. The soil-moisture trend overlays watering episodes, making the sense → act → verify loop visible at a glance.

![Dashboard — desktop](docs/images/dashboard-desktop-1.png)
![Dashboard — soil & watering history](docs/images/dashboard-desktop-2.png)

## Hardware

A single 12 V supply feeds the actuators directly and an LM2596 buck converter for the logic rail; the ESP32 only ever sources gate signals, never actuator current.

| Subsystem | Part | Notes |
|---|---|---|
| Main controller | Freenove ESP32-WROVER (with extension board) | dual-core + PSRAM; extension board for power & breakout |
| Telemetry hub | Raspberry Pi 4 | Mosquitto broker, SQLite, Streamlit dashboard |
| Soil moisture | Capacitive soil-moisture sensor | non-corroding; 4-point calibrated (DL-020, DL-042) |
| Air | BME280 | temperature, humidity, pressure (I²C) |
| Light | BH1750 | calibrated lux (I²C) |
| Reservoir level | Float switch | mounted through the lid on a carbon-fiber rod (DL-050) |
| Leak detection | Conductive leak sensor | tray-level, debounced (DL-026) |
| Pump | 12 V peristaltic pump | ~1.0 mL/s, calibrated (DL-048) |
| Pump driver | IRLB8721 logic-level N-channel MOSFET | + 1N4007 flyback diode (DL-009, DL-018) |
| Logic power | LM2596 buck converter | 12 V → 5 V ahead of the extension board (DL-016, DL-017) |
| Grow-light control | Shelly Plug US Gen4 | local RPC/MQTT, device-side scheduler (DL-054) |
| Local feedback | SSD1306 OLED (128×64), 3× traffic-light status LEDs, active buzzer | (DL-024, DL-044, DL-047) |
| Operator input | 3× push buttons — STOP / MANUAL / ACK | (DL-025) |

## Repository layout

```
firmware/
  test-sketches/   Phase 1 — one standalone PlatformIO sketch per component (01–13), each with a README
  integrated/      Phase 2 — the integrated WROVER firmware (state machine, sensors, MQTT)
hub/
  01-pi-setup … 08-grow-light   Phase 3 — Pi hub: broker, listener, dashboard, services, grow-light scheduler
docs/
  decision-log.md  the authoritative engineering record (57 entries)
  explainers/      narrative walkthroughs
  images/          validation and build evidence
```

## Firmware

Built with the **Arduino framework on ESP32, managed by PlatformIO** for reproducible builds. The integrated firmware is non-blocking throughout (no `delay()` in the main loop); sensors return validity-flagged structs, and lighting runs as logic parallel to the watering state machine. From a project directory, `pio run -t upload` compiles and flashes, then `pio device monitor` shows serial output.

## Decision log

Every meaningful choice — part substitutions, pin assignments, calibration, the state-machine design, fault-handling philosophy — is recorded in [`docs/decision-log.md`](docs/decision-log.md), with rationale and alternatives considered. It is the best place to understand *why* the system is built the way it is.

## Roadmap

- **Vision node** — a Seeed XIAO ESP32-S3 Sense (replacing the original ESP32-CAM, DL-034) for time-lapse capture and basic color/health analysis on the Pi.
- **Adaptive lighting** — supplement the photoperiod from the BH1750 against a daily-light-integral target, rather than a fixed schedule.
- **Push alerting** — notify on faults and device-offline events.
- **Security hardening** — dashboard authentication and MQTT over TLS.

## License

MIT — see [LICENSE](LICENSE).

## Author

**Temiloluwa Adesola** — [@IamTemmy](https://github.com/IamTemmy)
