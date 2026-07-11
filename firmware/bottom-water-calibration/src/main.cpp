// Plant Autonomy Testbed — Bottom-Watering Calibration Harness (Phase 5 prototype)
// ---------------------------------------------------------------------------
// A STANDALONE firmware, separate from the integrated build. It drives the pump
// in controlled DOSES (not the integrated firmware's ~5 mL top-water pulses),
// keeps full soil telemetry on the NORMAL MQTT topics so the Pi dashboard and
// SQLite log every reading, and keeps decisions in the operator's hands
// (guarded-manual): the firmware dispenses and watches; you decide supplements.
//
// This is NOT the production watering FSM. It is the isolated prototype whose
// dose / uptake / evaluate logic graduates into the integrated firmware once
// bottom-watering is validated (the work we have been calling "Phase 5").
//
// WHY a new firmware: the integrated watering algorithm (5 mL pulse + 10 s
// settle + 8-pulse no-progress watchdog, DL-049/053) is built for top-watering
// a fast-wicking probe. Bottom/tray watering wicks over MINUTES, so that logic
// faults at ~40 mL before the plant absorbs anything. Different regime, so a
// clean-slate operation mode with its own boundaries.
//
// *** SAFETY NOTE — deliberate departure from the integrated firmware ***
// The integrated firmware never lets an inbound command drive the pump. THIS
// harness intentionally does (topic plant/cmd/dose), because dashboard-triggered
// dosing was requested for supervised calibration. Every dose is hard-bounded by
// a per-dose cap AND a session cap, and the leak / abort / reservoir guards cut
// or block the pump on every loop tick. RUN ONLY WHILE SUPERVISED.
//
// Reused verbatim from firmware/integrated/src/config.h so behavior matches the
// characterized hardware: pins, soil calibration (DL-020), leak/float polarity,
// pump flow rate (DL-048), broker, topics, payload shapes.

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "secrets.h"  // WIFI_SSID/PASSWORD, MQTT_USER/PASSWORD (copy from integrated)

// ======================= Reused hardware configuration =====================
// Pins (Freenove ESP32-WROVER, Phase 1 bench wiring — identical to config.h)
static constexpr uint8_t SOIL_PIN       = 34;  // capacitive soil moisture (ADC1)
static constexpr uint8_t LEAK_PIN       = 39;  // leak pad (ADC1)
static constexpr uint8_t FLOAT_PIN      = 27;  // reservoir float; INPUT_PULLUP, CLOSED = empty
static constexpr uint8_t PUMP_GATE_PIN  = 25;  // IRLB8721 gate
static constexpr uint8_t LED_GREEN      = 18;
static constexpr uint8_t LED_YELLOW     = 19;
static constexpr uint8_t LED_RED        = 23;
static constexpr uint8_t BTN_DOSE       = 26;  // (BTN_MANUAL) short-press: start a dose
static constexpr uint8_t BTN_ACK        = 33;  // clear a latched fault / advance / reset session
static constexpr uint8_t BTN_ABORT      = 32;  // (BTN_STOP) immediate pump-off + latch

static constexpr uint16_t ADC_MAX = 4095;

// Soil calibration (DL-020). Capacitive: LOWER raw = WETTER.
static constexpr uint8_t  SOIL_SAMPLES       = 16;
static constexpr uint16_t SOIL_RAW_VALID_MIN = 800;
static constexpr uint16_t SOIL_RAW_VALID_MAX = 3200;
static constexpr uint16_t SOIL_RAW_DRY       = 2523;  // 0% anchor (dry potting mix)
static constexpr uint16_t SOIL_RAW_WET       = 1953;  // 100% anchor (~30 min post-water)

// Leak (DL-026). Conductive pads: HIGHER raw = WETTER.
static constexpr uint8_t  LEAK_SAMPLES   = 16;
static constexpr uint16_t LEAK_THRESHOLD = 200;
static constexpr uint32_t LEAK_DEBOUNCE_MS = 3000;   // must persist before latching

// Float polarity (DL-043): CLOSED (pin LOW) == empty.
static constexpr bool FLOAT_EMPTY_WHEN_CLOSED = true;

// Pump flow rate (DL-048).
static constexpr float PUMP_ML_PER_SEC = 1.0f;

// Network / MQTT (non-secret; matches config.h so telemetry lands on the dashboard)
static constexpr char     MQTT_BROKER_HOST[] = "10.6.19.139";
static constexpr uint16_t MQTT_BROKER_PORT   = 1883;
static constexpr char     MQTT_CLIENT_ID[]   = "wrover";  // same identity: dashboard continuity
static constexpr char     T_STATUS[] = "plant/status/wrover";
static constexpr char     T_SOIL[]   = "plant/sensors/soil";
static constexpr char     T_FLOAT[]  = "plant/sensors/float";
static constexpr char     T_LEAK[]   = "plant/sensors/leak";
static constexpr char     T_STATE[]  = "plant/state/wrover";
static constexpr char     T_CMD_DOSE[] = "plant/cmd/dose";  // inbound: integer mL, or "abort"

// ======================= Calibration boundaries (TUNE HERE) ================
static constexpr int      INITIAL_DOSE_ML = 150;   // first dose of a session
static constexpr int      SUPPLEMENT_ML   = 60;    // each button/MQTT top-up after the first
static constexpr int      MAX_DOSE_ML     = 200;   // hard cap on any single dose (bounds MQTT input)
static constexpr int      MAX_SESSION_ML  = 300;   // hard cap on total dispensed per session
static constexpr uint32_t UPTAKE_WAIT_MS  = 30UL * 60UL * 1000UL;  // 30 min lock between doses

// Cadence: publish faster than the integrated 30 s so the moisture climb is dense
// in the dashboard/DB during a session.
static constexpr uint32_t SENSOR_READ_MS   = 2000;
static constexpr uint32_t PUBLISH_MS       = 5000;
static constexpr uint32_t HEARTBEAT_MS     = 5000;
static constexpr uint32_t WIFI_BOOT_TIMEOUT_MS = 10000;
static constexpr uint32_t MQTT_RETRY_MS    = 5000;
static constexpr uint32_t BTN_DEBOUNCE_MS  = 50;

// ============================== State machine ==============================
enum State {
    ST_MONITOR,          // idle: read + publish; wait for a dose trigger
    ST_DOSING,           // pump running for the current dose's run-time
    ST_UPTAKE,           // pump off; wick + observe; new doses locked out
    ST_RESERVOIR_EMPTY,  // float reads empty; dosing blocked (auto-clears on refill)
    ST_SESSION_CAP,      // session total reached cap; dosing blocked until reset (ACK)
    ST_STOPPED,          // ABORT pressed; latched until ACK
    ST_LEAK_FAULT        // leak latched; clears on ACK once the pad is dry
};

static State state = ST_MONITOR;

static const char* state_name(State s) {
    switch (s) {
        case ST_MONITOR:         return "monitor";
        case ST_DOSING:          return "dosing";
        case ST_UPTAKE:          return "uptake";
        case ST_RESERVOIR_EMPTY: return "reservoir_empty";
        case ST_SESSION_CAP:     return "session_cap";
        case ST_STOPPED:         return "stopped";
        case ST_LEAK_FAULT:      return "leak_fault";
    }
    return "?";
}

// Session / dose bookkeeping
static float         session_ml       = 0.0f;
static int           dose_count       = 0;
static unsigned long dose_start_ms    = 0;
static unsigned long dose_target_ms   = 0;
static int           dose_ml_target   = 0;
static unsigned long uptake_start_ms  = 0;
static float         moisture_before  = NAN;   // captured at dose start
static float         moisture_after   = NAN;   // captured at uptake end

// Inbound-command requests (set in MQTT callback, consumed in loop)
static volatile int  pending_dose_ml  = 0;     // >0 => dose that many mL
static volatile bool pending_abort    = false;

// ============================== I/O primitives =============================
static WiFiClient   wifi_client;
static PubSubClient mqtt(wifi_client);

static bool pump_state = false;
static void pump_on()  { if (!pump_state) { pump_state = true;  digitalWrite(PUMP_GATE_PIN, HIGH); Serial.println("[PUMP] ON"); } }
static void pump_off() { if (pump_state)  { pump_state = false; digitalWrite(PUMP_GATE_PIN, LOW);  Serial.println("[PUMP] OFF"); } }
static bool pump_is_on() { return pump_state; }

struct SoilReading { uint16_t raw; float pct; bool valid; };
static SoilReading soil_read() {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < SOIL_SAMPLES; i++) sum += analogRead(SOIL_PIN);
    uint16_t raw = (uint16_t)(sum / SOIL_SAMPLES);
    SoilReading r{raw, NAN, false};
    r.valid = (raw >= SOIL_RAW_VALID_MIN && raw <= SOIL_RAW_VALID_MAX);
    if (!r.valid) return r;
    float span = (float)(SOIL_RAW_DRY - SOIL_RAW_WET);
    float pct = ((float)SOIL_RAW_DRY - raw) / span * 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    r.pct = pct;
    return r;
}

struct LeakReading { uint16_t raw; bool detected; };
static LeakReading leak_read() {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < LEAK_SAMPLES; i++) sum += analogRead(LEAK_PIN);
    uint16_t raw = (uint16_t)(sum / LEAK_SAMPLES);
    return LeakReading{raw, raw >= LEAK_THRESHOLD};
}

static bool reservoir_empty_read() {
    bool closed = (digitalRead(FLOAT_PIN) == LOW);
    return FLOAT_EMPTY_WHEN_CLOSED ? closed : !closed;
}

// Debounced, edge-detected active-LOW button.
struct Button { uint8_t pin; int stable; int last; unsigned long change_ms; bool edge; };
static Button b_dose {BTN_DOSE,  HIGH, HIGH, 0, false};
static Button b_ack  {BTN_ACK,   HIGH, HIGH, 0, false};
static Button b_abort{BTN_ABORT, HIGH, HIGH, 0, false};

static void button_update(Button& b, unsigned long now) {
    b.edge = false;
    int raw = digitalRead(b.pin);
    if (raw != b.last) { b.last = raw; b.change_ms = now; }
    if (now - b.change_ms >= BTN_DEBOUNCE_MS && raw != b.stable) {
        b.stable = raw;
        if (b.stable == LOW) b.edge = true;   // press edge
    }
}

// ============================== Cached readings ============================
static SoilReading last_soil{0, NAN, false};
static LeakReading last_leak{0, false};
static bool        last_reservoir_empty = false;
static unsigned long leak_since_ms = 0;

// ============================== MQTT =======================================
static void publish_state() {
    if (!mqtt.connected()) return;
    char p[128];
    snprintf(p, sizeof(p),
             "{\"state\":\"%s\",\"pump\":%d,\"session_ml\":%d,\"dose_count\":%d}",
             state_name(state), pump_is_on() ? 1 : 0, (int)session_ml, dose_count);
    mqtt.publish(T_STATE, p, true);  // retained: dashboard always has current state
}

static void publish_soil(const SoilReading& s) {
    if (!mqtt.connected() || !s.valid) return;
    char p[64];
    snprintf(p, sizeof(p), "{\"soil_raw\":%u,\"moisture_pct\":%.1f}", s.raw, s.pct);
    mqtt.publish(T_SOIL, p);
}
static void publish_leak(const LeakReading& l) {
    if (!mqtt.connected()) return;
    char p[64];
    snprintf(p, sizeof(p), "{\"leak_raw\":%u,\"leak_detected\":%d}", l.raw, l.detected ? 1 : 0);
    mqtt.publish(T_LEAK, p);
}
static void publish_float(bool empty) {
    if (!mqtt.connected()) return;
    char p[40];
    snprintf(p, sizeof(p), "{\"reservoir_empty\":%d}", empty ? 1 : 0);
    mqtt.publish(T_FLOAT, p);
}
static void publish_status(unsigned long hb) {
    if (!mqtt.connected()) return;
    char p[96];
    snprintf(p, sizeof(p),
             "{\"online\":true,\"uptime_s\":%lu,\"rssi\":%d,\"heartbeat\":%lu,\"fw\":\"calib\"}",
             millis() / 1000UL, WiFi.RSSI(), hb);
    mqtt.publish(T_STATUS, p, true);
}

// Inbound: plant/cmd/dose. Payload is an integer mL, or the word "abort".
// Only records intent; the loop applies it under the full guard chain.
static void on_message(char* topic, byte* payload, unsigned int len) {
    if (strcmp(topic, T_CMD_DOSE) != 0) return;
    char buf[16];
    unsigned int n = len < sizeof(buf) - 1 ? len : sizeof(buf) - 1;
    memcpy(buf, payload, n);
    buf[n] = '\0';
    if (strcmp(buf, "abort") == 0) { pending_abort = true; Serial.println("MQTT: dose/abort"); return; }
    int ml = atoi(buf);
    if (ml > 0) { pending_dose_ml = ml; Serial.printf("MQTT: dose request %d mL\n", ml); }
    else        { Serial.println("MQTT: cmd/dose ignored (want integer mL or 'abort')"); }
}

static unsigned long mqtt_next_attempt_ms = 0;
static void mqtt_tick() {
    if (WiFi.status() != WL_CONNECTED) return;
    if (mqtt.connected()) { mqtt.loop(); return; }
    unsigned long now = millis();
    if (now < mqtt_next_attempt_ms) return;
    mqtt_next_attempt_ms = now + MQTT_RETRY_MS;
    Serial.print("MQTT: connecting... ");
    const char* will = "{\"online\":false}";
    if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, T_STATUS, 0, true, will)) {
        Serial.println("connected.");
        publish_status(0);
        mqtt.subscribe(T_CMD_DOSE);
        publish_state();
        Serial.println("MQTT: subscribed to cmd/dose");
    } else {
        Serial.printf("failed rc=%d\n", mqtt.state());
    }
}

// ============================== Dose control ===============================
// Guarded entry point for both the physical button and the MQTT command.
static void start_dose(int ml) {
    if (state != ST_MONITOR) return;                 // only from idle
    if (last_reservoir_empty) { state = ST_RESERVOIR_EMPTY; return; }
    int budget = MAX_SESSION_ML - (int)session_ml;
    if (budget <= 0) { state = ST_SESSION_CAP; return; }
    if (ml > MAX_DOSE_ML) ml = MAX_DOSE_ML;
    if (ml > budget)      ml = budget;               // never exceed the session cap
    if (ml <= 0) return;

    moisture_before = last_soil.valid ? last_soil.pct : NAN;
    dose_ml_target  = ml;
    dose_target_ms  = (unsigned long)((float)ml / PUMP_ML_PER_SEC * 1000.0f);
    dose_start_ms   = millis();
    dose_count++;
    pump_on();
    state = ST_DOSING;
    Serial.printf("[DOSE] #%d start: %d mL (%lu ms), before=%.1f%%, session->%d/%d mL\n",
                  dose_count, ml, dose_target_ms, moisture_before,
                  (int)session_ml + ml, MAX_SESSION_ML);
    publish_state();
}

// Water actually delivered on this dose (mL), based on real pump-on time,
// so a dose cut short by a guard is still accounted against the session cap.
static float dose_delivered_ml() {
    unsigned long on_ms = millis() - dose_start_ms;
    if (on_ms > dose_target_ms) on_ms = dose_target_ms;
    return (float)on_ms / 1000.0f * PUMP_ML_PER_SEC;
}

// ============================== Setup / loop ===============================
static unsigned long sensor_next_ms = 0, publish_next_ms = 0, heartbeat_next_ms = 0;
static unsigned long heartbeat = 0;

void setup() {
    Serial.begin(115200);
    delay(500);  // boot only
    Serial.println("\n=== Bottom-Watering Calibration Harness (Phase 5 prototype) ===");
    Serial.println("Buttons: DOSE(26) start dose | ACK(33) clear/advance/reset | ABORT(32) stop");
    Serial.println("MQTT cmd: plant/cmd/dose  <integer mL> | 'abort'");
    Serial.printf("Caps: dose<=%d mL, session<=%d mL, uptake lock %lus\n",
                  MAX_DOSE_ML, MAX_SESSION_ML, UPTAKE_WAIT_MS / 1000UL);

    pinMode(PUMP_GATE_PIN, OUTPUT); digitalWrite(PUMP_GATE_PIN, LOW);
    pinMode(LED_GREEN, OUTPUT); pinMode(LED_YELLOW, OUTPUT); pinMode(LED_RED, OUTPUT);
    pinMode(BTN_DOSE, INPUT_PULLUP);
    pinMode(BTN_ACK, INPUT_PULLUP);
    pinMode(BTN_ABORT, INPUT_PULLUP);
    analogReadResolution(12);
    analogSetPinAttenuation(SOIL_PIN, ADC_11db);
    analogSetPinAttenuation(LEAK_PIN, ADC_11db);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long t0 = millis();
    Serial.print("WiFi: connecting");
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < WIFI_BOOT_TIMEOUT_MS) {
        delay(250); Serial.print(".");
    }
    Serial.println(WiFi.status() == WL_CONNECTED ? " ok" : " (continuing offline)");

    mqtt.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    mqtt.setCallback(on_message);
}

void loop() {
    const unsigned long now = millis();
    mqtt_tick();

    button_update(b_dose, now);
    button_update(b_ack, now);
    button_update(b_abort, now);

    // ---- Sensor read cadence (cache) ----
    if (now >= sensor_next_ms) {
        last_soil = soil_read();
        last_leak = leak_read();
        last_reservoir_empty = reservoir_empty_read();
        sensor_next_ms = now + SENSOR_READ_MS;
    }

    // ---- Leak debounce ----
    if (last_leak.detected) { if (leak_since_ms == 0) leak_since_ms = now; }
    else                    { leak_since_ms = 0; }
    const bool leak_confirmed = (leak_since_ms != 0 && now - leak_since_ms >= LEAK_DEBOUNCE_MS);

    const State prev = state;

    // ---- Consume inbound MQTT intents once per tick ----
    const int  req_dose  = pending_dose_ml;  pending_dose_ml = 0;
    const bool req_abort = pending_abort;    pending_abort = false;

    // ---- Safety first: overrides everything, every tick ----
    if (leak_confirmed) {
        pump_off();
        state = ST_LEAK_FAULT;
    } else if (b_abort.edge || req_abort) {
        pump_off();
        state = ST_STOPPED;
    } else if (state == ST_LEAK_FAULT) {
        if (b_ack.edge && !last_leak.detected) state = ST_MONITOR;
    } else if (state == ST_STOPPED) {
        if (b_ack.edge) state = ST_MONITOR;
    } else if (state == ST_SESSION_CAP) {
        // Sticky until the operator resets the session.
        if (b_ack.edge) { session_ml = 0; dose_count = 0; state = ST_MONITOR;
                          Serial.println("[SESSION] reset"); }
    } else if (last_reservoir_empty) {
        pump_off();
        state = ST_RESERVOIR_EMPTY;
    } else {
        if (state == ST_RESERVOIR_EMPTY) state = ST_MONITOR;   // refilled
        // ---- Normal operation ----
        switch (state) {
            case ST_MONITOR:
                if (session_ml >= MAX_SESSION_ML) { state = ST_SESSION_CAP; break; }
                if (b_dose.edge) {
                    start_dose(session_ml <= 0.0f ? INITIAL_DOSE_ML : SUPPLEMENT_ML);
                } else if (req_dose > 0) {
                    start_dose(req_dose);
                }
                break;
            case ST_DOSING:
                if (now - dose_start_ms >= dose_target_ms) {
                    pump_off();
                    moisture_after = NAN;
                    uptake_start_ms = now;
                    state = ST_UPTAKE;
                }
                break;
            case ST_UPTAKE:
                // Doses locked out while wicking. ACK advances early; else the
                // timer releases the lock. ABORT/leak still handled above.
                if (b_ack.edge || now - uptake_start_ms >= UPTAKE_WAIT_MS) {
                    moisture_after = last_soil.valid ? last_soil.pct : NAN;
                    Serial.printf("[DOSE] #%d done: %d mL | before %.1f%% -> after %.1f%% | "
                                  "delta %+.1f | session %d mL\n",
                                  dose_count, dose_ml_target, moisture_before, moisture_after,
                                  moisture_after - moisture_before, (int)session_ml);
                    state = ST_MONITOR;
                }
                break;
            default: break;
        }
    }

    // ---- Account water delivered whenever we leave DOSING (normal or cut) ----
    if (prev == ST_DOSING && state != ST_DOSING) {
        session_ml += dose_delivered_ml();
    }

    // ---- LEDs: green idle, yellow dosing, red fault ----
    digitalWrite(LED_YELLOW, state == ST_DOSING);
    digitalWrite(LED_RED, state == ST_LEAK_FAULT || state == ST_STOPPED);
    digitalWrite(LED_GREEN, state == ST_MONITOR || state == ST_UPTAKE);

    if (state != prev) publish_state();

    // ---- Telemetry publish cadence ----
    if (now >= publish_next_ms) {
        publish_soil(last_soil);
        publish_leak(last_leak);
        publish_float(last_reservoir_empty);
        publish_next_ms = now + PUBLISH_MS;
    }

    // ---- Heartbeat ----
    if (now >= heartbeat_next_ms) {
        heartbeat++;
        publish_status(heartbeat);
        if (state == ST_UPTAKE) {
            unsigned long left = (UPTAKE_WAIT_MS - (now - uptake_start_ms)) / 1000UL;
            Serial.printf("[UPTAKE] %lus left | soil %.1f%% (raw %u)\n",
                          left, last_soil.pct, last_soil.raw);
        }
        heartbeat_next_ms = now + HEARTBEAT_MS;
    }
    // No delay(): loop returns immediately.
}
