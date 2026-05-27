/*
 * Plant Autonomy Testbed — Phase 1 component test
 * Subsystem: Peristaltic pump driven by IRLB8721 MOSFET
 *
 * Wiring:
 *   IRLB8721 Gate, via 220 ohm series -> ESP32 GPIO25
 *   IRLB8721 Gate, via 10k pull-down  -> GND  (forces OFF at boot)
 *   IRLB8721 Drain                    -> Pump negative
 *   IRLB8721 Source                   -> GND (common with 12V and ESP32)
 *   Pump positive                     -> +12V rail
 *   1N4007 diode across pump:
 *     banded (cathode)                -> +12V rail
 *     unbanded (anode)                -> MOSFET drain
 *
 * Power:
 *   ESP32 powered by USB during this test.
 *   Pump powered by 12V adapter on the +12V rail (common ground with ESP32).
 *
 * Cycle: 3 seconds OFF, 2 seconds ON, repeat. Serial: 115200 baud.
 */

#include <Arduino.h>

static const uint8_t  PUMP_GATE_PIN = 25;
static const uint32_t OFF_MS        = 3000;
static const uint32_t ON_MS         = 2000;

void setup() {
  pinMode(PUMP_GATE_PIN, OUTPUT);
  digitalWrite(PUMP_GATE_PIN, LOW);   // ensure OFF before anything else

  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("Plant Autonomy Testbed — Pump + MOSFET bench test"));
  Serial.println(F("=================================================="));
  Serial.print(F("Gate driven from GPIO"));
  Serial.println(PUMP_GATE_PIN);
  Serial.println(F("Cycle: 3 s OFF, 2 s ON, repeat."));
  Serial.println(F("If the pump runs in the wrong direction, swap its leads."));
  Serial.println(F("Press EN/RESET or unplug to stop."));
  Serial.println();
}

void loop() {
  Serial.println(F("[OFF]"));
  digitalWrite(PUMP_GATE_PIN, LOW);
  delay(OFF_MS);

  Serial.println(F("[ON]"));
  digitalWrite(PUMP_GATE_PIN, HIGH);
  delay(ON_MS);
}
