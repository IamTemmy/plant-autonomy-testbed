#pragma once
// Capacitive soil moisture sensor (analog, GPIO34 / ADC1).
// Per DL-040 Principle 3, read() returns a struct with a validity flag.
// Calibrated in DL-020 (three conditions: air / dry soil / wet soil).
//
// Polarity: capacitive sensor reads a LOWER raw ADC count when WETTER.
// moisture_pct maps the dry-soil anchor (0%) to the wet-soil anchor (100%),
// clamped to [0,100]; in-air readings clamp to 0% (probe not in soil).
// See DL-042 for the percentage mapping and the wet-anchor caveat.

#include <stdint.h>

struct SoilReading {
    uint16_t raw;           // averaged ADC count (lower = wetter)
    float    moisture_pct;  // 0% at dry anchor, 100% at wet anchor, clamped
    bool     valid;         // false on out-of-range (likely disconnected probe)
};

// Configure the ADC pin. Call once from setup().
void soil_begin();

// Take one averaged reading. Check `.valid` before trusting the fields.
SoilReading soil_read();
