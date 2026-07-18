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
static constexpr uint8_t LED_YELLOW = 19;  // 270 ohm series (DL-044)
static constexpr uint8_t LED_RED    = 23;  // 270 ohm series (DL-044)
static constexpr uint8_t BTN_STOP   = 32;  // INPUT_PULLUP
static constexpr uint8_t BTN_ACK    = 33;  // INPUT_PULLUP
static constexpr uint8_t BTN_MANUAL = 26;  // INPUT_PULLUP
static constexpr uint8_t BUZZER     = 4;   // active buzzer via NPN driver

// ---- I2C device addresses -------------------------------------------------
static constexpr uint8_t BME280_ADDR     = 0x76;  // primary
static constexpr uint8_t BME280_ADDR_ALT = 0x77;  // some BME280 modules ship here
static constexpr uint8_t BH1750_ADDR     = 0x23;
static constexpr uint8_t OLED_ADDR       = 0x3C;

// ---- Timing / scheduling intervals (ms) -----------------------------------
// All periodic-task timing lives here. Scheduling pattern: see main.cpp loop().
static constexpr uint32_t HEARTBEAT_INTERVAL_MS = 5000;
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS    = 10000;  // boot connect wait
static constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 5000;   // loop reconnect cadence
static constexpr uint32_t MQTT_RECONNECT_INTERVAL_MS = 5000;   // loop reconnect cadence
static constexpr uint32_t SENSOR_READ_INTERVAL_MS    = 2000;   // sensor sampling cadence
static constexpr uint32_t MQTT_PUBLISH_INTERVAL_MS   = 30000;  // telemetry publish cadence
// State machine (DL-046)
static constexpr uint32_t LED_BLINK_MS        = 400;    // status LED blink period
static constexpr uint32_t BUTTON_DEBOUNCE_MS  = 50;     // pushbutton debounce
static constexpr uint32_t BTN_LONGPRESS_MS    = 2000;   // hold MANUAL this long to toggle maintenance mode (DL-089)
static constexpr uint32_t LEAK_DEBOUNCE_MS    = 3000;   // leak must persist this long to latch
static constexpr uint32_t WATER_PULSE_MS      = 5000;   // pump-on duration per watering pulse (~5 mL at 1.0 mL/s, DL-049)
static constexpr uint32_t WATER_SETTLE_MS     = 10000;  // wait for water to wick to the off-center probe before re-reading (DL-049)
static constexpr uint8_t  WATER_WATCHDOG_PULSES = 8;    // consecutive no-progress pulses -> watering fault (DL-053)
static constexpr uint16_t WATER_RESPONSE_MARGIN = 30;   // raw drop that counts as the soil responding (DL-053)
static constexpr uint32_t DAILY_WINDOW_MS     = 86400000UL;  // rolling 24h fallback window, used until NTP time is available (DL-058)
static constexpr float    PUMP_ML_PER_SEC     = 1.0f;        // measured pump flow rate (DL-048)
static constexpr float    MAX_DAILY_PUMP_ML   = 200.0f;      // daily water cap in mL (DL-048/058); raise to 250-300 if soil dries too fast
// Time sync for the calendar-midnight daily reset (DL-058). POSIX TZ string carries DST rules.
static constexpr const char* NTP_SERVER_1 = "pool.ntp.org";
static constexpr const char* NTP_SERVER_2 = "time.nist.gov";
static constexpr const char* LOCAL_TZ     = "CST6CDT,M3.2.0,M11.1.0";  // America/Chicago
static constexpr uint32_t OLED_REFRESH_MS     = 500;    // status display refresh cadence
static constexpr uint32_t BUZZER_ON_MS        = 200;    // alarm beep on duration
static constexpr uint32_t BUZZER_OFF_MS       = 600;    // alarm beep gap

// ---- ADC ------------------------------------------------------------------
static constexpr uint16_t ADC_MAX = 4095;  // 12-bit

// ---- Network / MQTT broker (non-secret; credentials live in secrets.h) ----
// WROVER is a plain LAN client on JSU_DEVICE, so it reaches the broker at the
// Pi's LAN IP. The tailnet IP is for remote dashboard access, not the ESP32.
static constexpr char     MQTT_BROKER_HOST[] = "10.6.19.139";
static constexpr uint16_t MQTT_BROKER_PORT   = 1883;
static constexpr char     MQTT_CLIENT_ID[]   = "wrover";
static constexpr char     MQTT_TOPIC_STATUS[]         = "plant/status/wrover";   // retained presence
static constexpr char     MQTT_TOPIC_SENSORS_BME280[] = "plant/sensors/bme280";  // air telemetry
static constexpr char     MQTT_TOPIC_SENSORS_BH1750[] = "plant/sensors/bh1750";  // light telemetry
static constexpr char     MQTT_TOPIC_SENSORS_SOIL[]   = "plant/sensors/soil";    // soil moisture
static constexpr char     MQTT_TOPIC_SENSORS_FLOAT[]  = "plant/sensors/float";   // reservoir level
static constexpr char     MQTT_TOPIC_SENSORS_LEAK[]   = "plant/sensors/leak";    // leak detection
static constexpr char     MQTT_TOPIC_STATE[]          = "plant/state/wrover";    // FSM state (retained)
static constexpr char     MQTT_TOPIC_CMD_MAINT[]     = "plant/cmd/maintenance"; // inbound: "on"|"off"

// ---- Sensor validity bounds (plausibility guards) -------------------------
// A reading outside these ranges signals a sensor fault, not a real value;
// the sensor's read() sets valid=false. BME280 bounds from DL-015.
static constexpr float BME280_TEMP_MIN_C   =   -5.0f;
static constexpr float BME280_TEMP_MAX_C   =   60.0f;
static constexpr float BME280_HUM_MIN_PCT  =    0.0f;
static constexpr float BME280_HUM_MAX_PCT  =  100.0f;
static constexpr float BME280_PRES_MIN_HPA =  800.0f;
static constexpr float BME280_PRES_MAX_HPA = 1100.0f;
// BH1750 bounds from DL-021.
static constexpr float BH1750_LUX_MIN = 0.0f;
static constexpr float BH1750_LUX_MAX = 100000.0f;
// Soil: samples to average per read, and plausible raw band (DL-020). Outside
// this band the probe is likely disconnected. Air ~2854, dry ~2585, wet ~2250.
static constexpr uint8_t  SOIL_SAMPLES        = 16;
static constexpr uint16_t SOIL_RAW_VALID_MIN  = 800;
static constexpr uint16_t SOIL_RAW_VALID_MAX  = 3200;
static constexpr uint8_t  LEAK_SAMPLES        = 16;
// Float switch polarity: true if a CLOSED switch (pin LOW) means empty.
// Flip this if a physical lift/drop test shows the opposite. (DL-043)
static constexpr bool     FLOAT_EMPTY_WHEN_CLOSED = true;

// ---- Calibration: MEASURED references from the decision log ---------------
// These describe what the hardware reads in known conditions. Do not invent.

// Soil moisture (DL-020, recalibrated DL-106 for bottom watering + sand).
// Capacitive: LOWER raw count = WETTER soil.
static constexpr uint16_t SOIL_RAW_DRY_AIR = 2854;  // probe in air (reference only)
static constexpr uint16_t SOIL_RAW_DRY     = 2585;  // 0%: sustained drought floor (drooping, DL-104)
static constexpr uint16_t SOIL_RAW_WET     = 2250;  // 100%: healthy-wet ceiling w/ headroom (DL-106; wettest observed ~2401)

// Leak sensor (DL-026). Conductive pads: HIGHER raw count = WETTER.
// NOTE: opposite polarity from soil moisture — guard against inverted logic.
static constexpr uint16_t LEAK_RAW_DRY = 0;  // clean dry baseline

// Pump flow rate is PUMP_ML_PER_SEC (above), measured in Phase 2 (DL-048).

// ---- Thresholds: DECIDED boundaries the firmware acts on ------------------
// Seeded from DL-020 / DL-026; tunable in Phase 2.

// Soil: water when raw RISES to/above TRIGGER (drying out); stop when it
// FALLS to/below STOP (re-wetted). Hysteresis prevents toggling. (DL-020)
//
// ⚠ LEGACY / STALE (DL-114): these raw thresholds were tuned for the OLD
// 2523/1953 calibration. Under the current 2585/2250 anchors (DL-106) they now
// map to ~70% (trigger) and >100% (stop, unreachable) — NOT valid for the new
// scale. They belong to the top-water pulse FSM that is being retired for the
// bottom-watering loop. Re-tune or remove BEFORE that FSM is ever re-enabled
// (it is reachable only outside maintenance mode).
static constexpr uint16_t SOIL_THRESHOLD_TRIGGER = 2352;  // legacy: ~30% @ old cal, ~70% @ new (DL-073)
static constexpr uint16_t SOIL_THRESHOLD_STOP    = 2200;  // legacy: ~57% @ old cal, >100% @ new

// Leak: enter CRITICAL at/above this; latched until ACK button. (DL-026)
static constexpr uint16_t LEAK_THRESHOLD = 200;
