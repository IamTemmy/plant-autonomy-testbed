# 13 — Pump Flow Calibration

Drives the real 12V peristaltic pump (GPIO25 via the MOSFET validated in `02-pump-mosfet`) to measure its flow rate in mL/s. The result sets the daily water cap (DL-048) and is the prerequisite for enabling the real pump in the Phase 2 firmware (DL-049).

## Wiring

| Component | ESP32 pin | Notes |
|---|---|---|
| Pump MOSFET gate | GPIO25 | HIGH = pump on; 1N4007 flyback across the pump (per `02-pump-mosfet`) |
| PRIME button | GPIO32 | `INPUT_PULLUP` — hold to jog the pump |
| RUN button | GPIO26 | `INPUT_PULLUP` — one timed dispense (`RUN_MS`, currently 90 s) |
| ABORT button | GPIO33 | `INPUT_PULLUP` — stop a timed run early |

Also: pump inlet in the water source, outlet into a measuring cup, and the 12V rail powered.

**Note on the buttons:** these roles (PRIME / RUN / ABORT) are reassigned *for this calibration sketch only*. The same GPIOs are STOP (32) / MANUAL (26) / ACK (33) in the integrated firmware — they revert to those roles when the integrated firmware is reflashed.

## Run

`pio run -t upload`, then `pio device monitor`.

## Procedure

1. Inlet in water, outlet in the cup. **Hold PRIME** until water flows in a steady, bubble-free stream; release. Empty the primed water from the cup.
2. **Press RUN** — the pump runs for `RUN_MS` and stops on its own (ABORT stops early if needed).
3. Read the volume collected. `mL/s = mL ÷ (RUN_MS / 1000)`.
4. Repeat several times *without emptying the cup*, reading the cumulative volume at eye level (avoid parallax) on a clearly-marked graduation. Larger total volume → smaller read error.

## What success looks like

- Priming produces a steady, bubble-free stream (the peristaltic pump holds prime when off)
- Repeated timed runs deliver consistent volumes
- The computed rate is stable across runs

Measured result: **~1.0 mL/s** (DL-048), from which the daily cap was set to 200 mL.

## What this test does not verify

Long-term flow drift, and how water actually spreads through real potting soil to reach the off-center moisture probe — that delivery behavior (and the pulse/settle tuning that depends on it) is observed and tuned on the real plant (DL-049). Small air bubbles from the inline tube connectors are negligible at calibration volumes but were noted for the deployment build.
