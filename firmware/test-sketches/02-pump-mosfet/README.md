# 02 — Pump + MOSFET Bench Test

Phase 1 component validation: verify the IRLB8721 N-channel MOSFET can be switched by an ESP32 GPIO to drive a 12V peristaltic pump on and off, with a flyback diode catching the inductive turn-off spike.

## Wiring

| Connection | To |
|---|---|
| IRLB8721 Gate, via 220Ω series | ESP32 GPIO25 |
| IRLB8721 Gate, via 10kΩ pull-down | GND |
| IRLB8721 Drain | Pump negative |
| IRLB8721 Source | GND (common with 12V supply and ESP32) |
| Pump positive | +12V rail |
| 1N4007 diode across pump | Banded (cathode) to +12V; unbanded (anode) to drain |

## Run

ESP32 powered by USB during this test; pump powered by the 12V adapter on the +12V rail. From this directory: `pio run -t upload` then `pio device monitor`.

## What success looks like

- Serial banner prints at startup
- Repeating cycle: `[OFF]` → 3s silence → `[ON]` → 2s pump running → repeat
- Pump starts and stops cleanly at each transition — no continued dribble after `[OFF]` (proves the flyback diode is doing its job)
- ESP32 does not reset, MOSFET runs cool to warm (not hot), no smoke
- If the pump moves water in the wrong direction, swap its leads — direction is set by polarity

## What failure looks like

- Pump runs continuously regardless of state → MOSFET wired wrong, or shorted
- Pump never runs during `[ON]` → check gate voltage with multimeter; expect ~3.3V at gate relative to source during `[ON]`
- ESP32 resets when pump turns on → likely a missing common ground, or supply margin issue
- MOSFET heats rapidly → over-current condition (pump stalled, or wiring fault)
