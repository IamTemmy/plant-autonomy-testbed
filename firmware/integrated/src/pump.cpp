#include "pump.h"

#include <Arduino.h>

#include "config.h"

static bool pump_state = false;

void pump_begin() {
    // Real output: configure the MOSFET gate and ensure the pump starts OFF.
    pinMode(PUMP_GATE_PIN, OUTPUT);
    digitalWrite(PUMP_GATE_PIN, LOW);
    pump_state = false;
    Serial.println("[PUMP] ready (GPIO25 live)");
}

void pump_on() {
    if (!pump_state) {
        pump_state = true;
        digitalWrite(PUMP_GATE_PIN, HIGH);
        Serial.println("[PUMP] ON");
    }
}

void pump_off() {
    if (pump_state) {
        pump_state = false;
        digitalWrite(PUMP_GATE_PIN, LOW);
        Serial.println("[PUMP] OFF");
    }
}

bool pump_is_on() {
    return pump_state;
}
