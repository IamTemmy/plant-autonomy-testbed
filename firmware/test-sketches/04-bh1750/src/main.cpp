/*
 * Plant Autonomy Testbed — Phase 1 component test
 * Sensor: BH1750 ambient light sensor (lux, calibrated)
 *
 * Wiring:
 *   BH1750 VCC  -> 3.3V on extension board
 *   BH1750 GND  -> GND rail (common)
 *   BH1750 SCL  -> GPIO22 (shared I2C bus with BME280)
 *   BH1750 SDA  -> GPIO21 (shared I2C bus with BME280)
 *   BH1750 ADDR -> not connected (default address 0x23)
 *
 * Output (1 Hz on serial @ 115200):
 *   lux value with a coarse classification (dark / dim / room / bright / very bright)
 *
 * Plausibility bounds: readings outside 0..100000 lux are flagged.
 */

#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

static const float LUX_MIN = 0.0f;
static const float LUX_MAX = 100000.0f;

const char* classify(float lux) {
  if (lux < 1.0f)     return "dark";
  if (lux < 50.0f)    return "dim";
  if (lux < 1000.0f)  return "room";
  if (lux < 10000.0f) return "bright";
  return "very bright";
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("Plant Autonomy Testbed — BH1750 bench test"));
  Serial.println(F("=========================================="));

  Wire.begin();   // default SDA=21, SCL=22 on ESP32

  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("ERROR: BH1750 not found on I2C bus."));
    Serial.println(F("Check wiring (SDA->GPIO21, SCL->GPIO22, VCC->3V3, GND->GND)."));
    Serial.println(F("Confirm the BME280 still responds — a wiring fault on the shared"));
    Serial.println(F("bus would affect both sensors."));
    while (true) { delay(1000); }
  }

  Serial.println(F("BH1750 detected at default address 0x23."));
  Serial.println(F("Reading every 1 second. Columns: lux, classification"));
  Serial.println();
}

void loop() {
  float lux = lightMeter.readLightLevel();

  Serial.print(lux, 2);
  Serial.print(F(" lx   ("));
  Serial.print(classify(lux));
  Serial.print(F(")"));

  if (lux < LUX_MIN || lux > LUX_MAX) {
    Serial.print(F("   [WARN: implausible reading]"));
  }
  Serial.println();

  delay(1000);
}
