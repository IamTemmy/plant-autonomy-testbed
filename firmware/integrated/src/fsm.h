#pragma once
// Watering state machine (DL-046; rebuilt for bottom-watering in the P1-8 port, DL-147).
// Consumes the soil, float, and leak readings and decides the system state, driving the
// status LEDs, the pump, and publishing state to plant/state/wrover.
//
// States: monitor, dosing, settle, grace, reservoir_empty, recovery_hold, sensor_fault,
// leak_fault, stopped, maintenance. Safety (leak, disconnect, stop button, the hard
// pump-on ceiling) is evaluated first every tick and overrides all states; faults latch
// until ACK. Reservoir-empty and recovery_hold are recoverable blocks. Maintenance is an
// intentional, NVS-persisted watering pause (boot-default ON, DL-128) toggled by a
// long-press of the MANUAL button (DL-089) or remotely (DL-182). The dose->settle->
// evaluate volume-dosing loop is the DL-142 design; see DL-147 for the port and the
// fail-safes it carries.

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
void fsm_request_start();   // remote start (DL-182): begin a session; respects maintenance (ignored if paused)

// Accessors for the display layer.
const char* fsm_state_name();
int fsm_session_ml();   // mL delivered in the current/last session (for the OLED)
int fsm_dose_count();   // doses in the current/last session
