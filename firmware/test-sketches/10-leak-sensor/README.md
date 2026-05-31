# 10 — Leak Sensor Bench Test

Phase 1 component validation and calibration for the DIYables water-detection sensor. Will trigger the CRITICAL fault state in Phase 2 firmware if water is detected outside the reservoir.

## Wiring

| Sensor | ESP32 |
|---|---|
| VCC | 3.3V (extension board) |
| GND | GND rail (common) |
| AOUT | GPIO39 (ADC1, input-only) |

## Run

`pio run -t upload`, then `pio device monitor`.

## Calibration procedure

Record stable averaged readings in three conditions:

1. **Dry pads** — sensor on dry surface, no contact with water. Baseline.
2. **Damp pads** — touch a slightly wet finger to the pads, or a drop of water at the edge. Confirms responsiveness to small water presence.
3. **Wet pads** — pour a small amount of water across the pads. The "definitely leaking" reference.

Higher raw values indicate more conductive water bridging the pads. The leak-trigger threshold for Phase 2 firmware will be set above the dry-pad baseline but below the wet-pad value, with hysteresis to prevent toggling near the threshold.

## What success looks like

- Stable dry-pad reading near the low end of the ADC range
- Visible jump when a damp finger or small water drop bridges the pads
- High reading (large fraction of ADC scale) under sustained wet conditions
- Response within ~1 second of water contact

## What this test does not verify

Long-term sensor stability (the exposed copper pads will corrode over time with sustained wet exposure — this is a known property of this style of sensor and is mitigated in deployment by mounting the sensor *under* the reservoir/pump area rather than continuously submerged). Behavior with non-pure water (potting-mix runoff has dissolved minerals that change conductivity — Phase 2 may need to re-tune the threshold once observing in the deployed environment).
