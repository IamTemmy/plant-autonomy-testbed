# 07 — User-Feedback Subsystem Bench Test

Phase 1 component validation for the local user-interaction components: status LEDs, pushbuttons, and an active buzzer. Built across three commits so each component group can be validated in isolation before being combined.

| Pass | Components | Purpose |
|---|---|---|
| 1 | LEDs only | Validate LED hardware and the state-pattern blink scheme |
| 2 | + buttons | Validate buttons and exercise state transitions |
| 3 | + buzzer | Validate audible alarm during CRITICAL state |

## Wiring (all passes)

| Component | ESP32 pin | Wiring |
|---|---|---|
| Green LED | GPIO18 | anode → GPIO; cathode → 270Ω → GND |
| Red LED | GPIO19 | anode → GPIO; cathode → 270Ω → GND |
| Button A (STOP) | GPIO32 | one leg → GPIO; other → GND (INPUT_PULLUP) |
| Button B (ACK) | GPIO33 | one leg → GPIO; other → GND (INPUT_PULLUP) |
| Button C (MANUAL) | GPIO26 | one leg → GPIO; other → GND (INPUT_PULLUP) |
| Active buzzer | GPIO4 → 1kΩ → NPN base | emitter → GND; collector → buzzer (−); buzzer (+) → 5V |

## Pass 1 — LEDs only

`pio run -t upload`, then `pio device monitor`. Cycles through five state patterns, 3 seconds each: IDLE, MEASURING, ACTION, FAULT, CRITICAL. Each pattern is announced over serial as it begins.

### What success looks like (Pass 1)

- Both LEDs visibly light during their respective states
- Green and red are clearly distinguishable
- Slow and fast blink rates are clearly distinguishable from each other and from steady
- Pattern repeats cleanly
