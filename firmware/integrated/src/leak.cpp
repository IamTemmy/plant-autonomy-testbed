#include "leak.h"

#include <Arduino.h>

#include "config.h"

void leak_begin() {
    analogReadResolution(12);                       // 0..4095
    analogSetPinAttenuation(LEAK_PIN, ADC_11db);
}

LeakReading leak_read() {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < LEAK_SAMPLES; i++) {
        sum += analogRead(LEAK_PIN);
    }
    const uint16_t raw = static_cast<uint16_t>(sum / LEAK_SAMPLES);

    LeakReading r;
    r.raw = raw;
    // P0-5 (DL-140): near full-scale = pull-up on an open pin = disconnected (can't be
    // real water; DL-026 caps a submerged reading near ~2067). Below that, >=THRESHOLD = leak.
    r.disconnected = (raw >= LEAK_DISCONNECT_RAW);
    r.detected     = (!r.disconnected && raw >= LEAK_THRESHOLD);
    r.valid        = true;
    return r;
}
