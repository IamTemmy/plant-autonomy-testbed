#pragma once
// Watering state machine (DL-046). Consumes the soil, float, and leak readings
// and decides the system state, driving the status LEDs, the (stubbed) pump,
// and publishing state to plant/state/wrover.
//
// States: monitoring, watering, manual, reservoir_empty, daily_limit,
// leak_fault, stopped, watering_fault, maintenance. Safety (leak, stop button)
// is evaluated first every tick and overrides all states; faults latch until
// ACK. Reservoir-empty and daily-limit are recoverable blocks. Maintenance is
// an intentional, NVS-persisted watering pause toggled by a long-press of the
// MANUAL button (DL-089). See DL-046 for the full design.

#include "soil.h"
#include "float_switch.h"
#include "leak.h"

// Initialize LEDs, buttons, and the pump. Call once from setup().
void fsm_begin();

// Advance the state machine one step. Call every loop() iteration with the
// latest cached sensor readings. Non-blocking.
void fsm_safety_tick();   // audit #1 (DL-174): network-independent hard pump-off; call FIRST in loop()
void fsm_tick(const SoilReading& soil, const FloatReading& flt, const LeakReading& leak);

// Request an intentional maintenance pause on/off from outside the FSM (e.g.
// a remote MQTT command). Idempotent and safe: the request is consumed on the
// next fsm_tick() through the same safety-ordered path as the MANUAL long-press
// (faults still override; entering pauses the pump).
void fsm_request_maintenance(bool on);
void fsm_request_abort();   // remote abort (DL-169): stop the current session; STOP-button equivalent

// Accessors for the display layer.
const char* fsm_state_name();
int fsm_session_ml();   // mL delivered in the current/last session (for the OLED)
int fsm_dose_count();   // doses in the current/last session
