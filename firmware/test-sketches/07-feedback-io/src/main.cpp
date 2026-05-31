/*
 * Plant Autonomy Testbed — Phase 1 component test
 * Subsystem: User feedback — Pass 1 (LEDs only)
 *
 * Wiring:
 *   Green LED: GPIO18 -> LED anode; cathode -> 270 ohm -> GND
 *   Red LED:   GPIO19 -> LED anode; cathode -> 270 ohm -> GND
 *
 * State patterns (each held for 3 seconds, then advance):
 *   IDLE       green steady
 *   MEASURING  green slow blink  (500 ms period)
 *   ACTION     green fast blink  (100 ms period)
 *   FAULT      red steady
 *   CRITICAL   red fast blink    (100 ms period)
 *
 * These are the state patterns the Phase 2 firmware will use. The test
 * cycles through all five so we can confirm each is visually distinct.
 */

#include <Arduino.h>

static const uint8_t  LED_GREEN = 18;
static const uint8_t  LED_RED   = 19;
static const uint32_t HOLD_MS   = 3000;

void allOff() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
}

void blinkFor(uint8_t pin, uint16_t halfPeriodMs, uint32_t durationMs) {
  uint32_t start = millis();
  bool on = false;
  uint32_t lastToggle = start;
  while (millis() - start < durationMs) {
    if (millis() - lastToggle >= halfPeriodMs) {
      on = !on;
      digitalWrite(pin, on ? HIGH : LOW);
      lastToggle = millis();
    }
  }
  digitalWrite(pin, LOW);
}

void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  allOff();

  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("Plant Autonomy Testbed - LED state pattern test"));
  Serial.println(F("==============================================="));
  Serial.println(F("Cycling through every state pattern, 3 s each."));
  Serial.println();
}

void loop() {
  Serial.println(F("[IDLE] green steady"));
  digitalWrite(LED_GREEN, HIGH);
  delay(HOLD_MS);
  allOff();

  Serial.println(F("[MEASURING] green slow blink"));
  blinkFor(LED_GREEN, 500, HOLD_MS);

  Serial.println(F("[ACTION] green fast blink"));
  blinkFor(LED_GREEN, 100, HOLD_MS);

  Serial.println(F("[FAULT] red steady"));
  digitalWrite(LED_RED, HIGH);
  delay(HOLD_MS);
  allOff();

  Serial.println(F("[CRITICAL] red fast blink"));
  blinkFor(LED_RED, 100, HOLD_MS);

  Serial.println();
}
