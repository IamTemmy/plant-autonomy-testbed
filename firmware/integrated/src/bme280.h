#pragma once
// BME280 air sensor (temperature, humidity, pressure) on the shared I2C bus.
// Per DL-040 Principle 3, read() returns a struct with a validity flag;
// downstream code must check `valid` before using the values. Validated in
// DL-015 (re-validated on the integrated bench in DL-019); plausibility bounds
// (in config.h) reused from that bench test.

struct Bme280Reading {
    float temperature_c;
    float humidity_pct;
    float pressure_hpa;
    bool  valid;
};

// Initialize the I2C bus and the sensor. Call once from setup().
void bme280_begin();

// Take one reading. Check `.valid` before trusting the numeric fields: it is
// false if the sensor failed to init or a value fell outside plausible bounds.
Bme280Reading bme280_read();
