#include "float_switch.h"

#include <Arduino.h>

#include "config.h"

void float_switch_begin() {
    pinMode(FLOAT_PIN, INPUT_PULLUP);
}

FloatReading float_switch_read() {
    const bool closed = (digitalRead(FLOAT_PIN) == LOW);  // CLOSED pulls to GND
    const bool empty  = FLOAT_EMPTY_WHEN_CLOSED ? closed : !closed;
    return FloatReading{empty, true};
}
