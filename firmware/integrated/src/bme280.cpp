#include "bme280.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

#include "config.h"

static Adafruit_BME280 bme;
static bool bme_ready = false;

// Print every responding device on the I2C bus. Used as a diagnostic when the
// BME280 is not found, to distinguish "wrong address" from "dead bus/wiring".
static void i2c_scan() {
    uint8_t count = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.print("  I2C device at 0x");
            if (addr < 16) Serial.print("0");
            Serial.println(addr, HEX);
            count++;
        }
    }
    if (count == 0) {
        Serial.println("  no I2C devices found - check SDA/SCL wiring and 3V3 power.");
    }
}

void bme280_begin() {
    Wire.begin(I2C_SDA, I2C_SCL);

    // DL-015/DL-019 validated the sensor; try primary then alternate (auto-detect).
    if (bme.begin(BME280_ADDR)) {
        bme_ready = true;
        Serial.println("BME280: initialized at 0x76.");
    } else if (bme.begin(BME280_ADDR_ALT)) {
        bme_ready = true;
        Serial.println("BME280: initialized at 0x77.");
    } else {
        bme_ready = false;
        Serial.println("BME280: NOT found at 0x76 or 0x77. Scanning I2C bus...");
        i2c_scan();
    }
}

Bme280Reading bme280_read() {
    Bme280Reading r{NAN, NAN, NAN, false};

    if (!bme_ready) {
        return r;  // never initialized; reading is invalid
    }

    const float t = bme.readTemperature();         // degC
    const float h = bme.readHumidity();            // %RH
    const float p = bme.readPressure() / 100.0f;   // Pa -> hPa

    // Plausibility bounds from DL-015. Out-of-range => sensor fault, not data.
    const bool plausible =
        t >= BME280_TEMP_MIN_C   && t <= BME280_TEMP_MAX_C  &&
        h >= BME280_HUM_MIN_PCT  && h <= BME280_HUM_MAX_PCT &&
        p >= BME280_PRES_MIN_HPA && p <= BME280_PRES_MAX_HPA;

    r.temperature_c = t;
    r.humidity_pct  = h;
    r.pressure_hpa  = p;
    r.valid         = plausible;
    return r;
}
