# 04 — BH1750 Light Sensor Bench Test

Phase 1 component validation for the BH1750 ambient light sensor. Returns calibrated lux values directly — no calibration step required.

## Wiring

Shares the I²C bus with the BME280.

| BH1750 | ESP32 |
|---|---|
| VCC | 3.3V (extension board) |
| GND | GND rail (common) |
| SCL | GPIO22 (shared I²C bus) |
| SDA | GPIO21 (shared I²C bus) |
| ADDR | Not connected (default address 0x23) |

## Run

`pio run -t upload`, then `pio device monitor`.

## What success looks like

- Startup line: "BH1750 detected at default address 0x23."
- Continuous lux readings at 1 Hz with a coarse classification (dark / dim / room / bright / very bright)
- Values respond to changes: covering the sensor with a hand drops the reading toward 0; pointing it at a window or strong light pushes it well above 1000 lux
- No `[WARN: implausible reading]` suffix

## Plausibility checks against real-world expectations

| Condition | Expected lux |
|---|---|
| Hand covering sensor | < 1 |
| Dim room, evening | 10–100 |
| Normal room lighting | 100–500 |
| Bright daylight indoors (near sunny window) | 1,000–10,000 |
| Direct sunlight | 30,000–100,000 |

## What failure looks like

- "BH1750 not found" — usually a wiring fault, or the SDA/SCL pair got swapped
- BH1750 found but BME280 stops responding — shared-bus problem (check both sensors' grounds and the pull-up resistors implicitly provided by their breakout boards)
- Readings frozen at one value regardless of light — possible I²C bus stall; power-cycle
