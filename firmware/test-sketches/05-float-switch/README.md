# 05 — Float Switch Bench Test

Phase 1 component validation for the reservoir float switch. Determines which physical float orientation corresponds to OPEN vs CLOSED, so firmware can be written with correct polarity.

## Wiring

| Float switch | ESP32 |
|---|---|
| Either wire | GPIO27 |
| Other wire | GND |

No polarity. The ESP32's internal pull-up is enabled in firmware (no external resistor needed).

## Run

`pio run -t upload`, then `pio device monitor`. State prints at 5 Hz; transitions are emphasized with a `STATE CHANGE` line.

## Test procedure

1. Hold the float switch with the wires hanging down (float at the bottom). Note whether the monitor shows OPEN or CLOSED.
2. Invert the switch so the float rises to the top. Note the new state.
3. The transition is the calibration data: now you know which orientation means "water present" (float lifted) vs "water absent" (float hanging).
4. Record the mapping in the decision log.

## What success looks like

- Startup banner prints
- A stable initial state (OPEN or CLOSED) on the first reading
- Clear `STATE CHANGE` line when the float is tilted between orientations
- No bouncing/flickering between states when the float is held still

## What failure looks like

- State never changes regardless of float position → wiring fault or a stuck switch
- Rapidly toggles between states when held still → loose connection or extremely bouncy switch (we can add software debouncing if needed, but most float switches are stable)
