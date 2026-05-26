# 01 — BME280 Bench Test

Phase 1 component validation: verify the BME280 (air temperature, humidity, atmospheric pressure) is detected on the I2C bus, returns plausible readings, and stays stable over time.

## Wiring

| BME280 pin | ESP32 pin |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO21 |
| SCL | GPIO22 |

## Run

From this directory, run `pio run -t upload` to compile and flash. Then `pio device monitor` to view the serial output at 115200 baud.

## What success looks like

- Startup line: *"BME280 detected at I2C address 0x76"* (or 0x77 — both addresses exist depending on module variant)
- Continuous comma-separated readings, one per ~2 seconds, in the order: temperature (C), humidity (%), pressure (hPa), dew point (C)
- All values inside plausible indoor bounds — no "[WARN: implausible reading]" suffix on any line
- Temperature and humidity respond when you breathe gently on the sensor; both should rise within seconds and return to baseline within a minute

## What failure looks like

- *"ERROR: BME280 not found..."* — wiring fault or you have a BMP280 (no humidity) instead of a BME280
- Frozen readings (no response to breath) — possible I2C bus stall or stuck sensor; power-cycle and retry
- Implausible values — usually a wiring or power issue (loose connections, marginal 3V3)

## Notes

- The sketch auto-detects 0x76 or 0x77 I2C addresses, since both module variants are common.
- Dew point is derived from temperature and humidity using the Magnus formula. It is included as a sanity-check value, not because the project needs dew point directly.
