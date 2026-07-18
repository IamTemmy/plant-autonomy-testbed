#pragma once
// Pump actuator (GPIO25 via MOSFET). LIVE: pump_on/off drive the gate and track
// state. (Originally stubbed for dry FSM validation, DL-046; real GPIO output was
// enabled once the logic was validated and flow characterized at 1.0 mL/s, DL-048.)

void pump_begin();
void pump_on();
void pump_off();
bool pump_is_on();
