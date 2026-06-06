#pragma once
// Active buzzer (GPIO4 via NPN driver). digitalWrite HIGH = sound (no PWM).
// Used as the audible alarm for the critical autonomous fault (leak_fault).
// Validated in Phase 1 sketch 09-buzzer. See DL-047.

// Configure the pin (output, silent). Call once from setup().
void buzzer_begin();

// Drive the alarm. Call every tick: when `alarm` is true the buzzer beeps
// intermittently; when false it is silent. Non-blocking.
void buzzer_update(bool alarm, unsigned long now);
