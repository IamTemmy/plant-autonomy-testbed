#pragma once
// BH1750 ambient light sensor (lux) on the shared I2C bus.
// Per DL-040 Principle 3, read() returns a struct with a validity flag;
// downstream code must check `valid` before using the value. Validated in
// DL-021; plausibility bounds (in config.h) reused from that bench test.

struct Bh1750Reading {
    float lux;
    bool  valid;
};

// Initialize the sensor on the shared I2C bus. Call once from setup(), after
// the bus is up (bme280_begin already calls Wire.begin; this is idempotent).
void bh1750_begin();

// Take one reading. Check `.valid` before trusting `lux`: it is false if the
// sensor failed to init or the value fell outside plausible bounds.
Bh1750Reading bh1750_read();
