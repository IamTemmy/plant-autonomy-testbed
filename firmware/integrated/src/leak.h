#pragma once
// Leak / water-detection sensor (analog, GPIO39 / ADC1).
// Per DL-040 Principle 3, read() returns a struct with a validity flag.
// Characterized in DL-026. Polarity: HIGHER raw count = MORE water on the
// pads (opposite of the soil sensor). `detected` is raw >= LEAK_THRESHOLD.

#include <stdint.h>

struct LeakReading {
    uint16_t raw;       // averaged ADC count (higher = wetter)
    bool     detected;  // raw >= LEAK_THRESHOLD (config.h)
    bool     valid;     // analog read; always valid once begun
};

// Configure the ADC pin. Call once from setup().
void leak_begin();

// Take one averaged reading and apply the leak threshold.
LeakReading leak_read();
