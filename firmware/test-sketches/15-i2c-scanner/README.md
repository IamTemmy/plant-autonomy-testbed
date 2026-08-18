# 15 — I2C bus scanner (diagnostic)

A minimal, side-effect-free scanner for the shared I2C bus (SDA=21, SCL=22). It
probes every address and reports which devices ACK, with an explicit PASS/FAIL
for the BME280 (0x76/0x77) and BH1750 (0x23).

## Why

The hub showed lux + temp/humidity/pressure going stale at `2026-08-14 21:17:44Z`
— the moment the board was reflashed from the integrated firmware to the
bottom-water calibration **harness**, which does not read I2C at all. This sketch
tells apart the two possibilities without flashing the integrated build (and its
old watering logic):

- **Sensors ACK** → electrically alive; the stale telemetry was just the harness
  not reading them. No hardware fault.
- **Sensors do NOT ACK** → real hardware fault; do the isolation test (remove one
  device, re-scan, see if the other returns).

## Run

```bash
cd firmware/test-sketches/15-i2c-scanner
pio run -t upload && pio device monitor
```

Reads the serial output once per second. No pump, WiFi, MQTT, or watering — it
only touches the I2C bus.

## Interpreting

- `SUMMARY: BME280 PRESENT | BH1750 PRESENT` → both alive; reflash the reading
  firmware and telemetry returns.
- One or both `MISSING` → isolate: power down, remove the missing device, re-scan;
  if the other now appears, the removed one was dragging the bus.
- `NONE` → suspect the shared SDA/SCL/3V3/GND lines first.
