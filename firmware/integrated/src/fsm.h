#pragma once
// Watering state machine (DL-046). Consumes the soil, float, and leak readings
// and decides the system state, driving the status LEDs, the (stubbed) pump,
// and publishing state to plant/state/wrover.
//
// States: monitoring, watering, manual, reservoir_empty, daily_limit,
// leak_fault, stopped. Safety (leak, stop button) is evaluated first every
// tick and overrides all states; faults latch until ACK. Reservoir-empty and
// daily-limit are recoverable blocks. See DL-046 for the full design.

#include "soil.h"
#include "float_switch.h"
#include "leak.h"

// Initialize LEDs, buttons, and the pump. Call once from setup().
void fsm_begin();

// Advance the state machine one step. Call every loop() iteration with the
// latest cached sensor readings. Non-blocking.
void fsm_tick(const SoilReading& soil, const FloatReading& flt, const LeakReading& leak);
