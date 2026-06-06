#pragma once
// Pump actuator (GPIO25 via MOSFET). STUBBED for now: pump_on/off log and
// track state but do NOT drive the GPIO, so the watering state machine can be
// validated with zero water risk. Real output is enabled only after the logic
// is validated and the pump is calibrated (mL/s). See DL-046.

void pump_begin();
void pump_on();
void pump_off();
bool pump_is_on();
