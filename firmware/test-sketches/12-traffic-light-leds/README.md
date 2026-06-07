# 12 — Traffic-Light Status LEDs Bench Test

Phase 1 re-validation of the three status LEDs after the DL-044 rewire — green GPIO18, yellow GPIO19, red GPIO23 — arranged as a vertical traffic light (red top, yellow middle, green bottom). Confirms each GPIO drives its intended LED in the correct physical position before the Phase 2 state machine (DL-045/046) relies on them. Validated in DL-045.

## Wiring

| LED | ESP32 pin | Position | Wiring |
|---|---|---|---|
| Red | GPIO23 | top | anode → GPIO; cathode → 270Ω → GND |
| Yellow | GPIO19 | middle | anode → GPIO; cathode → 270Ω → GND |
| Green | GPIO18 | bottom | anode → GPIO; cathode → 270Ω → GND |

Active-HIGH: a GPIO driven HIGH lights its LED. Pin values match the integrated `config.h`.

## Run

`pio run -t upload`, then `pio device monitor`.

## Behavior (looping)

- **Phase A — identify:** each LED lit alone, top-to-bottom (red, then yellow, then green), with a serial label naming which should be on so the physical position can be confirmed.
- **Phase B — all blink:** all three LEDs blink together three times.

## What success looks like

- Each named LED lights *alone* when the serial labels it (no two on at once → no wiring short or swap)
- Each LED is in the correct physical position (red top, yellow middle, green bottom)
- GPIO23 actually drives the red LED (the newly added wire from the rewire)
- All three blink together in Phase B

## What this test does not verify

The state-machine's mapping of FSM states to LED patterns (steady vs. blink, which color for which state) — that belongs to the Phase 2 firmware (DL-046). This sketch confirms only that the three LEDs are wired correctly and individually addressable in the intended traffic-light order.
