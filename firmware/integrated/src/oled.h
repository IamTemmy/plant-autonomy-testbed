#pragma once
// SSD1306 128x64 OLED (I2C 0x3C, shared bus). Shows current FSM state and the
// key readings as the on-bench status display. Validated in Phase 1 sketch
// 06-oled. Non-fatal: if the display is absent, render is a no-op. See DL-047.

#include "bme280.h"
#include "soil.h"
#include "float_switch.h"
#include "leak.h"

// Probe and initialize the display (after Wire is up). Call once from setup().
void oled_begin();

// Draw the status screen. Call on a throttled cadence from loop().
void oled_render(const char* state, const Bme280Reading& air, const SoilReading& soil,
                 const FloatReading& flt, const LeakReading& leak,
                 bool pump_on, unsigned long daily_pump_ms, unsigned long cap_ms);
