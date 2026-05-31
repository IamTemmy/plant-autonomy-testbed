# 06 — SSD1306 OLED Display Bench Test

Phase 1 component validation for the 0.96-inch, 128×64 SSD1306 OLED display. Confirms the display is functional, the I²C bus tolerates a third device alongside BME280 and BH1750, and provides a visual preview of the Phase 2 status dashboard.

## Wiring

Shares the I²C bus with BME280 and BH1750.

| OLED | ESP32 |
|---|---|
| VCC | 3.3V (extension board) |
| GND | GND rail |
| SCL | GPIO22 (shared) |
| SDA | GPIO21 (shared) |

## Run

`pio run -t upload`, then `pio device monitor`.

## What success looks like

- Serial banner prints, followed by "SSD1306 detected at I2C address 0x3C"
- Splash screen for 2 seconds (yellow header band, blue body)
- Mock dashboard with title in the yellow zone, mock data in the blue zone
- The uptime counter at the bottom increments once per second — proves the screen is alive, not frozen on a static image
- No I²C errors; BME280 and BH1750 would still be readable on the same bus if a multi-sensor sketch ran

## What failure looks like

- "SSD1306 not found" — wiring fault, or the module's address is 0x3D instead of 0x3C (some clones use the alternate address)
- Display shows garbage or random pixels — possible I²C noise; check shared bus connections
- Display flickers — usually a power issue; try a slightly thicker 3.3V wire or different ground
- Yellow/blue split misalignment — fixed hardware; nothing we can do in software, just confirm your specific module's split height
