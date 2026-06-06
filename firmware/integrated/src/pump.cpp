#include "pump.h"

#include <Arduino.h>

#include "config.h"

static bool pump_state = false;

void pump_begin() {
    // STUB: the GPIO is intentionally NOT configured or driven yet. The pump is
    // the only actuator that can physically harm the plant, so it stays stubbed
    // until the watering logic is validated and the pump is calibrated (DL-046).
    // To enable real output later:
    //   pinMode(PUMP_GATE_PIN, OUTPUT); digitalWrite(PUMP_GATE_PIN, LOW);
    pump_state = false;
    Serial.println("[PUMP] stub mode - GPIO not driven");
}

void pump_on() {
    if (!pump_state) {
        pump_state = true;
        Serial.println("[PUMP] ON (stub)");
        // TODO real output: digitalWrite(PUMP_GATE_PIN, HIGH);
    }
}

void pump_off() {
    if (pump_state) {
        pump_state = false;
        Serial.println("[PUMP] OFF (stub)");
        // TODO real output: digitalWrite(PUMP_GATE_PIN, LOW);
    }
}

bool pump_is_on() {
    return pump_state;
}
