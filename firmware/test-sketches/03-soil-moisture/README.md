# 03 — Soil Moisture Sensor Bench Test

Phase 1 component validation and calibration for the capacitive soil moisture sensor. Records reference readings in three conditions to establish the operational range used by future firmware thresholds.

## Wiring

| Sensor | ESP32 |
|---|---|
| VCC | 3.3V (extension board) |
| GND | GND rail (common with everything else) |
| AOUT | GPIO34 (ADC1, input-only) |

GPIO34 is chosen because it sits on ADC1 (does not conflict with WiFi) and is input-only — appropriate for an analog sensor.

## Run

`pio run -t upload`, then `pio device monitor`. Readings stream at 1 Hz as `raw_avg, percent_of_full_scale`.

## Calibration procedure

Record stable averaged readings in three conditions:

1. **Dry air** — sensor probe held in air, away from anything wet. Baseline for "no medium at all."
2. **Dry soil** — probe inserted to working depth in dry potting mix. The "needs water urgently" reference.
3. **Wet soil** — same soil after thorough watering and ~10 minutes of distribution time. The "freshly watered" reference.

Lower raw values indicate wetter conditions; higher values indicate drier. The midpoint between (2) and (3) is a reasonable starting threshold for the firmware's watering decision in Phase 2.

## What this test does not verify

Long-term sensor drift, behavior under temperature changes, or behavior with the actual plant root system in place. These are addressed in Phase 2 with real-pot observation.
