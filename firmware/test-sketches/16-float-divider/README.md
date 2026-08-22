# 16 — Supervised Float Divider Reader (diagnostic)

Characterizes a **supervised, three-state** reservoir float sensor on the bench before committing to it in firmware. A plain float switch reads two states — closed (empty) or open (water) — but can't tell "water present" from "the wire fell off," so a disconnected float reads the same as a full reservoir: a silent failure on a safety-relevant sensor. This sketch tests giving the float the same disconnect-detection the leak sensor got under P0-5 (DL-140): a resistor divider that resolves a **third** state, wire-cut/disconnected.

It prints the raw ADC value on GPIO35 once per second with a rough voltage and a guessed state, so the three states can be confirmed against real hardware before any firmware three-level logic is written.

## Wiring under test

A voltage divider with the float switch in parallel with the lower leg:

```
3.3V --[10k R1]-- GPIO35 --[10k R2]-- GND
                    |
              float switch --> GND   (in parallel with R2)
```

| Reservoir state | Switch | Node | Raw ADC | ~Voltage |
|---|---|---|---|---|
| **Empty** | closed | shorted to GND | ~0 | ~0.0 V |
| **Water** | open | R1/R2 divider | ~2048 | ~1.65 V |
| **Fault** (wire cut) | — | only R1 pull-up | ~4095 | ~3.3 V |

GPIO35 is ADC1, input-only, and WiFi-safe (GPIO32 is taken by `BTN_ABORT`).

## Run

`pio run -t upload` then `pio device monitor` (115200). No pump, WiFi, MQTT, or watering — it reads the pin only. Toggle the switch and briefly disconnect the sensor lead to walk through all three states.

## What success looks like

```
=== Supervised float divider reader (P0-5 diagnostic) ===
GPIO35 (ADC1). Expect: EMPTY ~0 | WATER ~2048 | CUT ~4095
GPIO35 raw 2041  (~1.64 V)  -> WATER (switch open, divider)
GPIO35 raw    3  (~0.00 V)  -> EMPTY (switch closed)
GPIO35 raw 4093  (~3.30 V)  -> CUT/DISCONNECTED (only pull-up)
```

Each of the three bands reads cleanly and distinctly.

## Outcome — explored, not adopted

This was a design **exploration**, not the shipped design. The production integrated firmware kept the simpler **two-state digital float** on GPIO27 (`FLOAT_PIN`, `INPUT_PULLUP`, closed = empty) rather than the three-state divider here. The supervised-disconnect idea was retained where it earned its complexity — the **leak** sensor, where a disconnected sensor floating to "leak" or "no-leak" is the more dangerous failure (P0-5 / DL-140, the 100k pull-up on GPIO39). The float stayed digital: a disconnected float is caught in practice by the closed-loop watering watchdog (water not moving → `watering_fault`) rather than needing a third electrical state.

Kept in the repo as bench evidence of the alternative that was measured and set aside.
