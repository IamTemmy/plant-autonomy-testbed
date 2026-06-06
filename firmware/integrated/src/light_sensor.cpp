#include "light_sensor.h"

#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>

#include "config.h"

static BH1750 light_meter;
static bool   bh_ready = false;

void bh1750_begin() {
    Wire.begin(I2C_SDA, I2C_SCL);  // idempotent; safe if already begun
    bh_ready = light_meter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, BH1750_ADDR);
    if (bh_ready) {
        Serial.println("BH1750: initialized.");
    } else {
        Serial.println("BH1750: NOT found - check wiring/address (0x23).");
    }
}

Bh1750Reading bh1750_read() {
    Bh1750Reading r{NAN, false};

    if (!bh_ready) {
        return r;  // never initialized; reading is invalid
    }

    const float lux = light_meter.readLightLevel();

    // Plausibility bounds from DL-021. readLightLevel() returns a negative
    // value on a read/measurement error, which falls outside [MIN,MAX].
    r.lux   = lux;
    r.valid = (lux >= BH1750_LUX_MIN && lux <= BH1750_LUX_MAX);
    return r;
}
