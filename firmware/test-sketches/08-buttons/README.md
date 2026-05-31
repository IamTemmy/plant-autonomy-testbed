# 08 — Pushbuttons Bench Test

Phase 1 component validation for the three local pushbuttons. The LEDs validated in `07-feedback-io` are reused here as visual state indicators — this implicitly re-validates the LEDs in their intended role.

## Wiring

| Component | ESP32 pin | Wiring |
|---|---|---|
| Green LED | GPIO18 | anode → GPIO; cathode → 270Ω → GND |
| Red LED | GPIO19 | anode → GPIO; cathode → 270Ω → GND |
| Button A (STOP) | GPIO32 | one leg → GPIO; other → GND. `INPUT_PULLUP`. |
| Button B (ACK) | GPIO33 | one leg → GPIO; other → GND. `INPUT_PULLUP`. |
| Button C (MANUAL) | GPIO26 | one leg → GPIO; other → GND. `INPUT_PULLUP`. |

GPIO35 was originally allocated for Button C but lacks an internal pull-up (input-only without the pull-up/down option). GPIO26 was reclaimed for this purpose; it became available after DL-010 moved grow-light control to the Shelly smart plug.

## Run

`pio run -t upload`, then `pio device monitor`.

## What success looks like

- Boot lands in IDLE: green LED steady, serial logs `[STATE] -> IDLE (boot)`
- Press Button C (MANUAL) → green LED fast-blinks for ~3 seconds, then automatically returns to IDLE
- Press Button A (STOP) from any state → red LED on, green off (FAULT)
- From FAULT, press Button B (ACK) → returns to IDLE
- Serial logs every button press and every state transition
- No spontaneous transitions when buttons are not being pressed (would indicate input noise — the kind of failure mode we avoided by not using GPIO35)

## What this test does not verify

The state machine here is a simplified preview, not the final Phase 2 logic. Phase 2 will introduce additional states (MEASURING, CRITICAL), real sensor inputs, and the buzzer alarm during CRITICAL. This test only validates that the three buttons reliably trigger state transitions and that the LEDs visualize state cleanly.
