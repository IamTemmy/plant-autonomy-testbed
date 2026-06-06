#include "soil.h"

#include <Arduino.h>

#include "config.h"

void soil_begin() {
    // ADC1 channel. 11 dB attenuation gives the full ~0-3.3V range, matching
    // how this sensor was characterized in DL-020.
    analogReadResolution(12);                       // 0..4095
    analogSetPinAttenuation(SOIL_PIN, ADC_11db);
}

SoilReading soil_read() {
    SoilReading r{0, NAN, false};

    // Average several samples to tame the ADC/capacitive jitter observed in
    // DL-020 (~10-16 counts of wander at a fixed moisture level).
    uint32_t sum = 0;
    for (uint8_t i = 0; i < SOIL_SAMPLES; i++) {
        sum += analogRead(SOIL_PIN);
    }
    const uint16_t raw = static_cast<uint16_t>(sum / SOIL_SAMPLES);
    r.raw = raw;

    // Plausibility: a disconnected/faulted probe pins near a rail. A working
    // probe sits within the characterized band (wet < raw < air, with margin).
    r.valid = (raw >= SOIL_RAW_VALID_MIN && raw <= SOIL_RAW_VALID_MAX);
    if (!r.valid) {
        return r;
    }

    // Map dry anchor -> 0%, wet anchor -> 100% (lower raw = wetter), clamp.
    const float span = static_cast<float>(SOIL_RAW_DRY - SOIL_RAW_WET);
    float pct = (static_cast<float>(SOIL_RAW_DRY) - raw) / span * 100.0f;
    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    r.moisture_pct = pct;

    return r;
}
