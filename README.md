# Plant Autonomy Testbed

> A self-contained device that autonomously keeps a basil plant healthy — built as a demonstration of transferable autonomous-systems engineering.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Status](https://img.shields.io/badge/Status-Phase%201%3A%20Bench%20Prototype-orange.svg)](#current-phase)

## Overview

The Plant Autonomy Testbed senses soil moisture, air conditions, and light; waters the plant with a precisely metered dose only when needed; supplements light when the room is too dark; watches the plant with a camera; and reports its status remotely so the plant can be left unattended for weeks.

It originated from an engineering interview question — *"how would you design a system to autonomously care for a plant, including for when you travel?"* — and is being built into a complete, demonstrable answer.

## Engineering Framing

The plant is the demonstration vehicle. The point is the engineering pattern.

The system is built on the **sense → plan → act** autonomy loop common to robotics, aerospace, industrial automation, medical devices, and self-driving vehicles. The principles it exercises are general:

- **Closed-loop verification** — every action is checked against its expected effect. If the pump runs but the soil never gets wetter, something is wrong, and the system notices.
- **Graceful degradation** — sensor or actuator failure does not produce unsafe behavior. A broken moisture sensor cannot lead to overwatering.
- **Defined fallback state** — when anything goes wrong, the system enters a known safe configuration and alerts the operator, rather than guessing.
- **Explicit, inspectable behavior** — a state machine and a decision log make every choice the system makes auditable after the fact.

These patterns apply across autonomous-systems domains. The plant just makes them testable on a workbench.

## System Architecture

The system is organized into four functional layers:

| Layer | Components |
|---|---|
| **Perception** | Capacitive soil moisture sensor · BME280 (air temperature, humidity, pressure) · BH1750 (light) · float switch (reservoir level) · ESP32-CAM (vision) |
| **Planning** | ESP32 firmware — state machine with threshold logic, closed-loop verification, fault detection, decision logging |
| **Action** | Peristaltic pump driven by a MOSFET · grow light controlled by a Shelly smart plug over MQTT · OLED display, status LEDs, and buzzer for local feedback |
| **Telemetry** | WiFi → MQTT → Raspberry Pi 4 hub running Mosquitto, SQLite, a Streamlit dashboard, and ntfy push alerts |

## Hardware

The system runs on an **ESP32** main controller and an **ESP32-CAM** vision node, with a **Raspberry Pi 4** acting as the telemetry hub. The full bill of materials, including any substitutions and sourcing notes, will be finalized at the end of Phase 1 once all parts have been validated.

## Firmware

Built with the **Arduino framework on ESP32, managed by PlatformIO** for reproducible builds.

Phase 1 firmware lives in `firmware/test-sketches/` — small single-purpose sketches that validate one component each before integration begins in Phase 2. From a sketch's directory, run `pio run -t upload` to compile and flash, then `pio device monitor` to view serial output.

## License

MIT — see [LICENSE](LICENSE).

## Author

**Temiloluwa Adesola** — [@IamTemmy](https://github.com/IamTemmy)
