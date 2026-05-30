/*
 * Plant Autonomy Testbed — Phase 1 component test
 * Sensor: Capacitive soil moisture sensor (analog output)
 *
 * Wiring:
 *   Sensor VCC  -> 3.3V on extension board
 *   Sensor GND  -> GND rail (common with everything else)
 *   Sensor AOUT -> ESP32 GPIO34 (ADC1, input-only, WiFi-safe)
 *
 * Output (1 Hz on serial @ 115200):
 *   raw counts (0..4095), percent of full scale, simple 20-sample average
 *
 * Note: lower readings generally mean wetter soil; higher means drier.
 * The exact numbers are sensor- and soil-specific and must be calibrated
 * by recording readings in dry air, dry soil, and freshly-watered soil.
 */

#include <Arduino.h>

static const uint8_t  SOIL_PIN     = 34;
static const uint8_t  AVG_SAMPLES  = 20;
static const uint32_t SAMPLE_MS    = 1000;
static const uint16_t ADC_MAX      = 4095;

uint16_t read_averaged() {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < AVG_SAMPLES; i++) {
    sum += analogRead(SOIL_PIN);
    delay(5);
  }
  return (uint16_t)(sum / AVG_SAMPLES);
}

void setup() {
  analogReadResolution(12);                  // 0..4095
  analogSetPinAttenuation(SOIL_PIN, ADC_11db); // read full 0..3.3V range
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("Plant Autonomy Testbed — Soil moisture bench test"));
  Serial.println(F("=================================================="));
  Serial.print(F("Reading GPIO"));
  Serial.print(SOIL_PIN);
  Serial.print(F(" with "));
  Serial.print(AVG_SAMPLES);
  Serial.println(F("-sample averaging at 1 Hz"));
  Serial.println(F("Columns: raw_avg, percent_of_full_scale"));
  Serial.println();
}

void loop() {
  uint16_t raw = read_averaged();
  float pct = (100.0f * raw) / ADC_MAX;

  Serial.print(raw);
  Serial.print(F(",   "));
  Serial.print(pct, 1);
  Serial.println(F("%"));

  delay(SAMPLE_MS);
}
