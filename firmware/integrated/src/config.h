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
static constexpr float    PUMP_ML_PER_SEC     = 1.0f;        // measured pump flow rate (DL-048)
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
static constexpr char     MQTT_TOPIC_CMD_DOSE[]      = "plant/cmd/dose";        // inbound: "abort" (DL-169; "start" reserved for later)

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
static constexpr uint16_t SOIL_RAW_WET     = 1700;  // 100%: true saturation (DL-121; soil bottomed ~1620-1700 after real watering, DL-120). Old 2250 clipped the wet end. Dry anchor 2585 is INTERIM -- re-confirm on the next full dry-down plateau.

// Leak sensor (DL-026). Conductive pads: HIGHER raw count = WETTER.
// NOTE: opposite polarity from soil moisture — guard against inverted logic.
static constexpr uint16_t LEAK_RAW_DRY = 0;  // clean dry baseline

// Pump flow rate is PUMP_ML_PER_SEC (above), measured in Phase 2 (DL-048).

// ---- Thresholds: DECIDED boundaries the firmware acts on ------------------
// Seeded from DL-020 / DL-026; tunable in Phase 2.

// Leak: enter CRITICAL at/above this; latched until ACK button. (DL-026)
static constexpr uint16_t LEAK_THRESHOLD = 200;
// P0-5 (DL-140): a 100k pull-up on GPIO39 makes a DISCONNECTED leak sensor float to
// near full-scale instead of reading ~0 ("no leak"). A reading at/above this ceiling is
// treated as a sensor fault (wire cut / unplugged), not a valid value — the comb sensor
// tops out near ~2067 even submerged (DL-026), so a value this high can only be the
// pull-up on an open pin. Bench-validated: dry-connected 0, disconnected 4095.
static constexpr uint16_t LEAK_DISCONNECT_RAW = 3500;

// ---- Bottom-watering loop (DL-142) — the SETTLED strategy the ported FSM uses ----
// Plateau-gated volume dosing on a 60-80% band (DL-203; was 20-40%). (The legacy raw-threshold pulse constants
// — SOIL_THRESHOLD_*, WATER_PULSE_MS, WATER_WATCHDOG_PULSES, WATER_RESPONSE_MARGIN — were
// retired in the P1-8 cleanup, DL-150, along with the old top-water FSM.)
//
// % here are on the DL-121 2585/1700 soil mapping. TARGET_PCT is an EQUILIBRATED OUTCOME of
// metered volume, never a live pump cutoff (the probe is blind to the filling direction,
// DL-125): dose a measured volume, wait the settle, read the plateau, decide.
static constexpr float    TRIGGER_PCT      = 60.0f;   // start a cycle when moisture drifts below this (DL-203: 20->60; a 13-day dry-down showed the plant stressed by ~28% under the old 20% floor and thriving in the 50s-60s)
static constexpr float    TARGET_PCT       = 80.0f;   // stop supplementing once the plateau reaches/exceeds this (DL-203: 40->80; the Aug-20 watering equilibrated ~71% and the plant drew it down healthily over 13 days, so it tolerates and prefers the higher range)
static constexpr uint16_t DOSE1_ML         = 100;     // first dose of a cycle (DL-154: 150->100; the 150mL first cycle equilibrated to ~70% over ~12h, well past the 40% target)
static constexpr uint16_t SUPPLEMENT_ML    = 50;      // each subsequent dose if plateau < target (DL-154: 100->50 for a finer approach to target)
static constexpr uint16_t MAX_DOSE_ML      = 150;     // hard per-dose cap: tray absorbs each dose, never holds standing water
static constexpr uint16_t SESSION_CAP_ML   = 600;     // safety ceiling on total mL across one cycle's doses (SESSION_CAP > DOSE1 re-enables supplements)
static constexpr uint32_t SETTLE_MIN_MS    = 3UL * 60UL * 60UL * 1000UL;  // min wait after a dose before a plateau can gate a supplement (~3h; beats the probe lag)
static constexpr uint32_t PLATEAU_WINDOW_MS= 30UL * 60UL * 1000UL;         // window over which "stopped changing" is judged
static constexpr float    PLATEAU_SLOPE_PCT= 1.0f;    // <= this change over the window = plateaued (probe has caught up)
static constexpr float    ABSORB_RISE_PCT  = 7.0f;    // moisture rise that counts as the dose being "absorbed" (else: stall)
static constexpr uint32_t GRACE_MS         = 90UL * 60UL * 1000UL;         // one-time extra settle if absorption is slow

// ---- Physical pump + sensor safety (P0-2 / P0-3 / DL-127..131) ------------
static constexpr uint32_t MAX_PUMP_ON_MS   = 165000;  // P0-2: independent hard pump-on ceiling (>largest legit dose); checked before any network work
static constexpr uint32_t SOIL_STALE_MS    = 30000;   // P0-3: no valid soil read within this -> stale -> block starts / fault mid-session
static constexpr float    MOIST_EMA_ALPHA  = 0.1f;    // decision-variable smoothing on moisture %
