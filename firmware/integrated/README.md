# Phase 2 — Integrated WROVER firmware

The plant autonomy testbed's main control firmware. Reads sensors, runs the
state machine, drives actuators, publishes telemetry to the broker, and
operates autonomously for weeks at a time.

See [DL-040](../../docs/decision-log.md) for the architectural principles
guiding this codebase: non-blocking design, module decomposition, sensor
validity flags, FSM control flow, graceful degradation (sensor → halt
watering; WiFi/MQTT loss → non-fatal), centralized configuration, and
gitignored credentials.

## Current build status

**Skeleton.** Boots, prints a banner to serial, idles. No WiFi, no sensors,
no MQTT, no actuation yet. Subsequent commits add functionality in natural
units; this README will update as features land.

## Build, upload, monitor

From the developer Mac with the WROVER connected via USB:

```text
cd firmware/integrated
pio run -t upload
pio device monitor
```

Expected serial output on a successful upload + reset:

```text
=================================================
Plant Autonomy Testbed - Phase 2 firmware
Build: skeleton (per DL-040)
=================================================
```

Press `Ctrl+C` to exit the serial monitor.

## Project layout

```text
firmware/integrated/
├── platformio.ini       PlatformIO build configuration
├── src/
│   └── main.cpp         Main entry point (setup + loop)
└── README.md            This file
```

The layout will grow as modules land. The plan from DL-040:

- `src/main.cpp` — entry point, scheduler, FSM dispatch
- `src/sensors.{h,cpp}` — BME280, BH1750, soil moisture, float switch, leak sensor
- `src/actuators.{h,cpp}` — pump, LEDs, buzzer, OLED
- `src/fsm.{h,cpp}` — state machine
- `src/telemetry.{h,cpp}` — MQTT publish/subscribe
- `src/config.h` — pin assignments, thresholds, calibration constants, timing values
- `src/secrets.h` — WiFi + MQTT credentials (gitignored)
- `src/secrets.h.example` — committed template showing structure

These are aspirational; the exact decomposition emerges as code is written.

## Hardware target

Freenove ESP32 WROVER on the extension board, single 12V power tree with
LM2596 buck down to 5V, with the Phase 1 bench wiring already in place:

| Component | Connection |
|---|---|
| BME280, BH1750, OLED | I²C bus on GPIO21 (SDA) / GPIO22 (SCL) |
| Soil moisture (capacitive) | GPIO34 (ADC1) |
| Float switch | GPIO27 (INPUT_PULLUP, CLOSED = empty) |
| Leak sensor | GPIO39 (ADC1) |
| Pump MOSFET (IRLB8721) | GPIO25 + 1N4007 flyback diode |
| Status LEDs | GPIO18 (green), GPIO19 (red), 270Ω series |
| Buttons | GPIO32 (STOP), GPIO33 (ACK), GPIO26 (MANUAL) |
| Buzzer (active, via NPN driver) | GPIO4 |

Phase 1 validation entries for each component: DL-015 through DL-026.
