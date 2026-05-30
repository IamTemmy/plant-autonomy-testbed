/*
 * Plant Autonomy Testbed — Phase 1 component test
 * Sensor: Float switch (reservoir level)
 *
 * Wiring:
 *   Float switch wire 1 -> ESP32 GPIO27 (with internal pull-up enabled)
 *   Float switch wire 2 -> GND
 *
 * GPIO27 reads:
 *   HIGH (1) when the switch is OPEN
 *   LOW  (0) when the switch is CLOSED
 *
 * Which orientation produces OPEN vs CLOSED depends on the specific switch.
 * That mapping is what this test determines.
 *
 * Output (5 Hz on serial @ 115200):
 *   raw digital read + interpretation (OPEN / CLOSED)
 *   announces a "STATE CHANGE" line on each transition for easy tracking
 */

#include <Arduino.h>

static const uint8_t FLOAT_PIN = 27;

int lastState = -1;  // sentinel so the first reading always prints a transition

void setup() {
  pinMode(FLOAT_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("Plant Autonomy Testbed — Float switch bench test"));
  Serial.println(F("================================================="));
  Serial.print(F("Reading GPIO"));
  Serial.print(FLOAT_PIN);
  Serial.println(F(" with INPUT_PULLUP at 5 Hz."));
  Serial.println(F("Tilt or move the float and watch for STATE CHANGE lines."));
  Serial.println();
}

void loop() {
  int state = digitalRead(FLOAT_PIN);
  const char* label = (state == HIGH) ? "OPEN  " : "CLOSED";

  if (state != lastState) {
    Serial.print(F(">>> STATE CHANGE: now "));
    Serial.println(label);
    lastState = state;
  } else {
    Serial.print(F("state: "));
    Serial.print(state);
    Serial.print(F("  ("));
    Serial.print(label);
    Serial.println(F(")"));
  }

  delay(200);
}
