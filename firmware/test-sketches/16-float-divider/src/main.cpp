/*
 * Plant Autonomy Testbed — diagnostic: supervised float divider reader (P0-5)
 *
 * Prints the raw ADC value on GPIO35 once per second, plus a rough voltage, so the
 * three supervised states of the float divider can be read and confirmed before the
 * firmware three-level logic is written. Expected (R1 = R2 = 10k):
 *
 *   switch CLOSED (reservoir EMPTY)  -> ~0     (~0.0 V)  switch shorts node to GND
 *   switch OPEN   (reservoir WATER)  -> ~2048  (~1.65 V) R1/R2 divider
 *   wire CUT      (float FAULT)      -> ~4095  (~3.3 V)  only R1 pull-up remains
 *
 * Wiring under test:
 *   3.3V --[10k R1]-- GPIO35 --[10k R2]-- GND, with the float switch also across
 *   GPIO35 <-> GND (R2 in parallel with the switch).
 *
 * No pump / WiFi / MQTT / watering. Reads the pin only. Serial @ 115200.
 */
#include <Arduino.h>

static constexpr uint8_t FLOAT_ADC_PIN = 35;   // ADC1, input-only, WiFi-safe (32 is BTN_ABORT)
static constexpr uint8_t SAMPLES        = 16;

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n=== Supervised float divider reader (P0-5 diagnostic) ===");
    Serial.println("GPIO35 (ADC1). Expect: EMPTY ~0 | WATER ~2048 | CUT ~4095");
    analogSetPinAttenuation(FLOAT_ADC_PIN, ADC_11db);   // full 0..3.3V range, matches soil/leak
}

void loop() {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < SAMPLES; i++) sum += analogRead(FLOAT_ADC_PIN);
    uint16_t raw = (uint16_t)(sum / SAMPLES);
    float volts = raw * 3.3f / 4095.0f;
    const char* guess = raw < 700  ? "-> EMPTY (switch closed)"
                      : raw < 3000 ? "-> WATER (switch open, divider)"
                      :              "-> CUT/DISCONNECTED (only pull-up)";
    Serial.printf("GPIO35 raw %4u  (~%.2f V)  %s\n", raw, volts, guess);
    delay(1000);
}
