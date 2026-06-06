#pragma once
// Plant Autonomy Testbed — Phase 2 integrated firmware
// Centralized configuration: pins, I2C addresses, timing, thresholds,
// calibration. See DL-040 Principle 6.
//
// Pin/address names match the Phase 1 validated test-sketches so there is
// no remapping between validation and integration. Calibration figures are
// from the committed decision log (DL-020 soil, DL-026 leak); decision
// boundaries are seeded from those entries and refined empirically in Phase 2.

#include <stdint.h>

// ---- Pin assignments (Freenove ESP32-WROVER, Phase 1 bench wiring) --------

// I2C bus — shared by BME280, BH1750, OLED
static constexpr uint8_t I2C_SDA = 21;
static constexpr uint8_t I2C_SCL = 22;

// Analog sensors (ADC1, input-only, WiFi-safe)
static constexpr uint8_t SOIL_PIN = 34;   // capacitive soil moisture
static constexpr uint8_t LEAK_PIN = 39;   // leak / water-detection pad

// Digital sensor
static constexpr uint8_t FLOAT_PIN = 27;  // reservoir float; INPUT_PULLUP, CLOSED = empty

// Actuator
static constexpr uint8_t PUMP_GATE_PIN = 25;  // IRLB8721 gate; 1N4007 flyback across pump

// User I/O
static constexpr uint8_t LED_GREEN  = 18;  // 270 ohm series
static constexpr uint8_t LED_RED    = 19;  // 270 ohm series
static constexpr uint8_t BTN_STOP   = 32;  // INPUT_PULLUP
static constexpr uint8_t BTN_ACK    = 33;  // INPUT_PULLUP
static constexpr uint8_t BTN_MANUAL = 26;  // INPUT_PULLUP
static constexpr uint8_t BUZZER     = 4;   // active buzzer via NPN driver

// ---- I2C device addresses -------------------------------------------------
static constexpr uint8_t BME280_ADDR = 0x76;  // driver auto-detects 0x76/0x77
static constexpr uint8_t BH1750_ADDR = 0x23;
static constexpr uint8_t OLED_ADDR   = 0x3C;

// ---- Timing / scheduling intervals (ms) -----------------------------------
// All periodic-task timing lives here. Scheduling pattern: see main.cpp loop().
static constexpr uint32_t HEARTBEAT_INTERVAL_MS = 5000;
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS    = 10000;  // boot connect wait
static constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 5000;   // loop reconnect cadence
// Reserved for upcoming tasks (not yet scheduled):
// static constexpr uint32_t SENSOR_READ_INTERVAL_MS  = 2000;
// static constexpr uint32_t MQTT_PUBLISH_INTERVAL_MS = 5000;

// ---- ADC ------------------------------------------------------------------
static constexpr uint16_t ADC_MAX = 4095;  // 12-bit
// ---- Network / MQTT broker (non-secret; credentials live in secrets.h) ----
// WROVER is a plain LAN client on JSU_DEVICE, so it reaches the broker at the
// Pi's LAN IP. The tailnet IP is for remote dashboard access, not the ESP32.
static constexpr char     MQTT_BROKER_HOST[] = "10.6.19.139";
static constexpr uint16_t MQTT_BROKER_PORT   = 1883;

// ---- Calibration: MEASURED references from the decision log ---------------
// These describe what the hardware reads in known conditions. Do not invent.

// Soil moisture (DL-020). Capacitive: LOWER raw count = WETTER soil.
static constexpr uint16_t SOIL_RAW_DRY_AIR = 2854;  // probe in air (reference only)
static constexpr uint16_t SOIL_RAW_DRY     = 2523;  // dry potting mix
static constexpr uint16_t SOIL_RAW_WET     = 1953;  // ~30 min post-watering

// Leak sensor (DL-026). Conductive pads: HIGHER raw count = WETTER.
// NOTE: opposite polarity from soil moisture — guard against inverted logic.
static constexpr uint16_t LEAK_RAW_DRY = 0;  // clean dry baseline

// Pump dose calibration — TO BE MEASURED in Phase 2 with the measuring cup.
// static constexpr float PUMP_ML_PER_SEC = 0.0f;  // FILL AFTER CALIBRATION

// ---- Thresholds: DECIDED boundaries the firmware acts on ------------------
// Seeded from DL-020 / DL-026; tunable in Phase 2.

// Soil: water when raw RISES to/above TRIGGER (drying out); stop when it
// FALLS to/below STOP (re-wetted). Hysteresis prevents toggling. (DL-020)
static constexpr uint16_t SOIL_THRESHOLD_TRIGGER = 2400;  // start watering at/above
static constexpr uint16_t SOIL_THRESHOLD_STOP    = 2200;  // stop watering at/below

// Leak: enter CRITICAL at/above this; latched until ACK button. (DL-026)
static constexpr uint16_t LEAK_THRESHOLD = 200;
