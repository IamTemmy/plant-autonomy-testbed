# 09 — Active Buzzer Bench Test

Phase 1 component validation for the active buzzer in its intended role: audible alarm during a CRITICAL system state. Reuses the LEDs and buttons validated in `07-feedback-io` and `08-buttons`, implicitly re-validating them in a combined four-state state machine.

## Wiring

| Component | ESP32 pin | Wiring |
|---|---|---|
| Green LED | GPIO18 | anode → GPIO; cathode → 270Ω → GND |
| Red LED | GPIO19 | anode → GPIO; cathode → 270Ω → GND |
| Button A (STOP) | GPIO32 | one leg → GPIO; other → GND. `INPUT_PULLUP`. |
| Button B (ACK) | GPIO33 | one leg → GPIO; other → GND. `INPUT_PULLUP`. |
| Button C (MANUAL) | GPIO26 | one leg → GPIO; other → GND. `INPUT_PULLUP`. |
| Buzzer driver | GPIO4 → 1kΩ → NPN base | NPN emitter → GND; collector → buzzer (−); buzzer (+) → 5V |

## Run

`pio run -t upload`, then `pio device monitor`.

## State machine

| State | LED | Buzzer | Triggered by |
|---|---|---|---|
| IDLE | green steady | silent | boot, or ACK from fault states |
| ACTION | green fast blink | silent | MANUAL from IDLE (auto-returns to IDLE after 3 s) |
| FAULT | red steady | silent | STOP from IDLE or ACTION |
| CRITICAL | red fast blink | **ON** | STOP from FAULT (fault escalation) |

## What success looks like

- Boot lands in IDLE — green steady, buzzer silent (the critical check: buzzer must default off at power-on)
- Pressing STOP once enters FAULT — red steady, buzzer still silent
- Pressing STOP again (from FAULT) escalates to CRITICAL — red fast blink, **buzzer audible**
- Pressing ACK from CRITICAL returns to IDLE — buzzer silences immediately
- Manual route to test buzzer faster: STOP, STOP, listen, ACK

## What this test does not verify

The CRITICAL state's escalation trigger (STOP from FAULT) is a stand-in for what the Phase 2 firmware will use: real fault conditions like a detected leak, an unrecoverable sensor failure, or a watering attempt that didn't move the soil moisture reading. Those triggers belong to the integration phase; here we just confirm the buzzer drives correctly when the state demands it.
