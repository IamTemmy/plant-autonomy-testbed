/*
 * Plant Autonomy Testbed — Phase 1 component test
 * Sensor: BME280 (air temperature, humidity, atmospheric pressure)
 *
 * Wiring (ESP32 dev board):
 *   BME280 VCC  -> 3V3
 *   BME280 GND  -> GND
 *   BME280 SDA  -> GPIO21
 *   BME280 SCL  -> GPIO22
 *
 * I2C address: 0x76 or 0x77 (this sketch auto-detects).
 * Serial monitor: 115200 baud.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Two possible I2C addresses for the BME280 — module-dependent.
static const uint8_t BME280_ADDR_PRIMARY   = 0x76;
static const uint8_t BME280_ADDR_SECONDARY = 0x77;

// Plausible indoor sanity bounds. Outside these, something is wrong.
static const float TEMP_MIN_C   =   -5.0f;
static const float TEMP_MAX_C   =   60.0f;
static const float HUM_MIN_PCT  =    0.0f;
static const float HUM_MAX_PCT  =  100.0f;
static const float PRES_MIN_HPA =  800.0f;
static const float PRES_MAX_HPA = 1100.0f;

Adafruit_BME280 bme;

// Magnus formula for dew point in Celsius. A derived value from temperature
// and humidity — used here as a sanity-check that the sensor is doing real work.
float dewpoint_c(float t_c, float rh_pct) {
  const float a = 17.62f;
  const float b = 243.12f;
  float gamma = (a * t_c) / (b + t_c) + logf(rh_pct / 100.0f);
  return (b * gamma) / (a - gamma);
}

bool plausible(float val, float lo, float hi) {
  return (val >= lo) && (val <= hi);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("Plant Autonomy Testbed — BME280 bench test"));
  Serial.println(F("=========================================="));

  Wire.begin();  // default SDA=21, SCL=22 on ESP32

  // Try the primary I2C address, fall back to the secondary.
  bool found = bme.begin(BME280_ADDR_PRIMARY, &Wire);
  uint8_t addr = BME280_ADDR_PRIMARY;
  if (!found) {
    found = bme.begin(BME280_ADDR_SECONDARY, &Wire);
    addr = BME280_ADDR_SECONDARY;
  }

  if (!found) {
    Serial.println(F("ERROR: BME280 not found at either 0x76 or 0x77."));
    Serial.println(F("Check wiring: SDA->GPIO21, SCL->GPIO22, VCC->3V3, GND->GND."));
    while (true) { delay(1000); }
  }

  Serial.print(F("BME280 detected at I2C address 0x"));
  Serial.println(addr, HEX);
  Serial.println(F("Reading every 2 seconds. Columns:"));
  Serial.println(F("  temp_C, humidity_%, pressure_hPa, dewpoint_C"));
  Serial.println();
}

void loop() {
  float t  = bme.readTemperature();           // degrees C
  float h  = bme.readHumidity();              // percent
  float p  = bme.readPressure() / 100.0f;     // hPa
  float dp = dewpoint_c(t, h);

  Serial.print(t,  2); Serial.print(F(", "));
  Serial.print(h,  2); Serial.print(F(", "));
  Serial.print(p,  2); Serial.print(F(", "));
  Serial.print(dp, 2);

  if (!plausible(t, TEMP_MIN_C,   TEMP_MAX_C)   ||
      !plausible(h, HUM_MIN_PCT,  HUM_MAX_PCT)  ||
      !plausible(p, PRES_MIN_HPA, PRES_MAX_HPA)) {
    Serial.print(F("   [WARN: implausible reading]"));
  }
  Serial.println();

  delay(2000);
}
