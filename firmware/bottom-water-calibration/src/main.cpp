// Plant Autonomy Testbed — Bottom-Watering Control Loop (Phase 5 prototype)
// ---------------------------------------------------------------------------
// STANDALONE firmware, separate from the integrated build. It runs the
// AUTONOMOUS bottom-watering loop specified in DL-104..106: trigger at a low
// moisture %, dose a measured volume, wait a long settle + plateau, evaluate the
// trend, and decide (done / supplement / stall) — using the soil probe both as
// the target gauge and as an indirect tray-level sensor (a dose that does not
// raise moisture means the tray is still holding water; do not add more).
//
// Full soil telemetry stays on the NORMAL MQTT topics, so the Pi dashboard and
// SQLite log every reading and every state transition. This prototype's loop is
// what graduates into the integrated firmware once validated.
//
// WHY separate: the integrated watering algorithm (5 mL pulse + 10 s settle +
// 8-pulse watchdog, DL-049/053) is a top-water design that faults at ~40 mL on a
// slow bottom fill. Different regime, its own boundaries.
//
// *** SAFETY — RUN ONLY WHILE SUPERVISED (first cycles) ***
// This firmware drives the pump autonomously. Every dose is bounded by a per-dose
// cap and a session cap; leak / abort / reservoir guards cut or block the pump on
// every tick. The absorption gate (supplement only after a confirmed >=7% rise)
// is the overflow protection; the session cap is the runaway backstop.
//
// Manual overrides for testing: DOSE button / MQTT "start" force a session now;
// ABORT cuts the pump; ACK clears a latched stop and force-advances a wait.

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <math.h>

#include "secrets.h"  // WIFI_SSID/PASSWORD, MQTT_USER/PASSWORD (copy from integrated)

// ======================= Reused hardware configuration =====================
static constexpr uint8_t SOIL_PIN       = 34;
static constexpr uint8_t LEAK_PIN       = 39;
static constexpr uint8_t FLOAT_PIN      = 27;  // INPUT_PULLUP, CLOSED = empty
static constexpr uint8_t PUMP_GATE_PIN  = 25;
static constexpr uint8_t LED_GREEN      = 18;
static constexpr uint8_t LED_YELLOW     = 19;
static constexpr uint8_t LED_RED        = 23;
static constexpr uint8_t BTN_DOSE       = 26;  // force-start a session (test)
static constexpr uint8_t BTN_ACK        = 33;  // clear latched stop / force-advance a wait
static constexpr uint8_t BTN_ABORT      = 32;  // immediate pump-off + latch

// Soil calibration (DL-106). Capacitive: LOWER raw = WETTER.
static constexpr uint8_t  SOIL_SAMPLES       = 16;
static constexpr uint16_t SOIL_RAW_VALID_MIN = 800;
static constexpr uint16_t SOIL_RAW_VALID_MAX = 3200;
static constexpr uint16_t SOIL_RAW_DRY       = 2585;  // 0% anchor: drought floor
static constexpr uint16_t SOIL_RAW_WET       = 2250;  // 100% anchor: healthy-wet w/ headroom

// Leak (DL-026). Conductive pads: HIGHER raw = WETTER.
static constexpr uint8_t  LEAK_SAMPLES     = 16;
static constexpr uint16_t LEAK_THRESHOLD   = 200;
static constexpr uint32_t LEAK_DEBOUNCE_MS = 3000;

static constexpr bool  FLOAT_EMPTY_WHEN_CLOSED = true;   // DL-043
static constexpr float PUMP_ML_PER_SEC         = 1.0f;   // DL-048

// Network / MQTT (matches config.h so telemetry lands on the dashboard)
static constexpr char     MQTT_BROKER_HOST[] = "10.6.19.139";
static constexpr uint16_t MQTT_BROKER_PORT   = 1883;
static constexpr char     MQTT_CLIENT_ID[]   = "wrover";
static constexpr char     T_STATUS[] = "plant/status/wrover";
static constexpr char     T_SOIL[]   = "plant/sensors/soil";
static constexpr char     T_FLOAT[]  = "plant/sensors/float";
static constexpr char     T_LEAK[]   = "plant/sensors/leak";
static constexpr char     T_STATE[]  = "plant/state/wrover";
static constexpr char     T_CMD[]    = "plant/cmd/dose";  // inbound: "start" | "abort"

// ======================= Control-loop parameters (DL-107 spec) =============
static constexpr float    TRIGGER_PCT      = 20.0f;   // start a session at/below this
static constexpr float    TARGET_PCT       = 85.0f;   // stop target
static constexpr int      DOSE1_ML         = 200;     // first dose
static constexpr int      SUPPLEMENT_ML    = 100;     // each top-up after dose 1
static constexpr int      MAX_DOSE_ML      = 200;     // per-dose hard cap
static constexpr int      SESSION_CAP_ML   = 400;     // per-session hard cap (runaway backstop)
static constexpr float    ABSORB_RISE_PCT  = 7.0f;    // rise that counts as "absorbed"
static constexpr uint32_t SETTLE_MIN_MS    = 3UL * 60UL * 60UL * 1000UL;   // 3 h
static constexpr uint32_t PLATEAU_WINDOW_MS= 30UL * 60UL * 1000UL;          // 30 min
static constexpr float    PLATEAU_SLOPE_PCT= 1.0f;    // <= this over a window = plateaued
static constexpr uint32_t GRACE_MS         = 90UL * 60UL * 1000UL;          // 1.5 h (one-time)

// Cadence + smoothing
static constexpr uint32_t SENSOR_READ_MS       = 2000;
static constexpr uint32_t PUBLISH_MS           = 5000;
static constexpr uint32_t HEARTBEAT_MS         = 10000;
static constexpr uint32_t WIFI_BOOT_TIMEOUT_MS = 10000;
static constexpr uint32_t MQTT_RETRY_MS        = 5000;
static constexpr uint32_t BTN_DEBOUNCE_MS      = 50;
static constexpr float    MOIST_EMA_ALPHA      = 0.1f;  // decision variable smoothing

// ============================== State machine ==============================
enum State {
    ST_MONITOR,          // idle: read + publish; wait for trigger
    ST_DOSING,           // pump running for the current dose
    ST_SETTLE,           // pump off; 3 h min then wait for plateau
    ST_GRACE,            // pump off; one-time 1.5 h recovery wait after a stall
    ST_RESERVOIR_EMPTY,  // float empty; blocks triggering (auto-clears on refill)
    ST_STOPPED,          // capped / failed / abort / reservoir; latched until ACK
    ST_LEAK_FAULT        // leak latched; clears on ACK once dry
};
static State state = ST_MONITOR;

static const char* state_name(State s) {
    switch (s) {
        case ST_MONITOR:         return "monitor";
        case ST_DOSING:          return "dosing";
        case ST_SETTLE:          return "settle";
        case ST_GRACE:           return "grace";
        case ST_RESERVOIR_EMPTY: return "reservoir_empty";
        case ST_STOPPED:         return "stopped";
        case ST_LEAK_FAULT:      return "leak_fault";
    }
    return "?";
}

// ---- session / dose bookkeeping ----
static float         session_ml      = 0.0f;
static int           dose_count      = 0;
static float         session_baseline= NAN;   // % at trigger (reference)
static float         dose_before     = NAN;   // % just before the current dose
static bool          grace_used      = false; // one grace per session
static float         stall_reading   = NAN;   // % when a stall tripped
static const char*   last_reason     = "";    // surfaced in state payload + serial

static unsigned long dose_start_ms   = 0;
static unsigned long dose_target_ms  = 0;
static int           dose_ml_target  = 0;

// settle / plateau tracking
static unsigned long settle_start_ms = 0;
static bool          plateau_armed   = false;
static float         plateau_ref_pct = NAN;
static unsigned long plateau_ref_ms  = 0;

// grace tracking
static unsigned long grace_start_ms  = 0;

// decision variable (smoothed moisture)
static float         moist_ema       = NAN;

// inbound intents
static volatile bool pending_start = false;
static volatile bool pending_abort = false;

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
        if (b.stable == LOW) b.edge = true;
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
    char p[192];
    float m = isnan(moist_ema) ? -1.0f : moist_ema;
    snprintf(p, sizeof(p),
             "{\"state\":\"%s\",\"pump\":%d,\"session_ml\":%d,\"dose_count\":%d,"
             "\"moist_pct\":%.1f,\"reason\":\"%s\"}",
             state_name(state), pump_is_on() ? 1 : 0, (int)session_ml, dose_count,
             m, last_reason);
    mqtt.publish(T_STATE, p, true);   // retained
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
             "{\"online\":true,\"uptime_s\":%lu,\"rssi\":%d,\"heartbeat\":%lu,\"fw\":\"water-loop\"}",
             millis() / 1000UL, WiFi.RSSI(), hb);
    mqtt.publish(T_STATUS, p, true);
}

// Alert = prominent serial line + reason surfaced in the retained state payload
// (dashboard-visible). NOTE: push notification (ntfy) is not wired here; for the
// supervised prototype, serial + dashboard state is the alert surface. A small
// Pi-listener rule on plant/state/wrover reason can forward these to ntfy later.
static void watering_alert(const char* reason) {
    last_reason = reason;
    Serial.printf("[ALERT] %s\n", reason);
    publish_state();
}

// Inbound: plant/cmd/dose = "start" (force a session) | "abort".
static void on_message(char* topic, byte* payload, unsigned int len) {
    if (strcmp(topic, T_CMD) != 0) return;
    char buf[16];
    unsigned int n = len < sizeof(buf) - 1 ? len : sizeof(buf) - 1;
    memcpy(buf, payload, n);
    buf[n] = '\0';
    if      (strcmp(buf, "abort") == 0) { pending_abort = true; Serial.println("MQTT: abort"); }
    else if (strcmp(buf, "start") == 0) { pending_start = true; Serial.println("MQTT: start"); }
    else Serial.println("MQTT: cmd ignored (want 'start' or 'abort')");
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
        mqtt.subscribe(T_CMD);
        publish_state();
        Serial.println("MQTT: subscribed to cmd (start|abort)");
    } else {
        Serial.printf("failed rc=%d\n", mqtt.state());
    }
}

// ============================== Dose / session control =====================
static float dose_delivered_ml() {
    unsigned long on_ms = millis() - dose_start_ms;
    if (on_ms > dose_target_ms) on_ms = dose_target_ms;
    return (float)on_ms / 1000.0f * PUMP_ML_PER_SEC;
}

// Begin one dose (dose 1 or a supplement). Captures the pre-dose baseline for the
// absorption test, then pumps. Bounded by per-dose and session caps.
static void begin_dose(int ml) {
    if (last_reservoir_empty) { last_reason = "reservoir empty"; state = ST_STOPPED; pump_off(); return; }
    int budget = SESSION_CAP_ML - (int)session_ml;
    if (budget <= 0) { last_reason = "capped: target not reached"; state = ST_STOPPED; pump_off(); return; }
    if (ml > MAX_DOSE_ML) ml = MAX_DOSE_ML;
    if (ml > budget)      ml = budget;
    if (ml <= 0) return;

    dose_before    = moist_ema;
    dose_ml_target = ml;
    dose_target_ms = (unsigned long)((float)ml / PUMP_ML_PER_SEC * 1000.0f);
    dose_start_ms  = millis();
    dose_count++;
    pump_on();
    state = ST_DOSING;
    Serial.printf("[DOSE] #%d: %d mL (%lu ms) | before %.1f%% | session -> %d/%d mL\n",
                  dose_count, ml, dose_target_ms, dose_before,
                  (int)session_ml + ml, SESSION_CAP_ML);
    publish_state();
}

static void enter_settle(unsigned long now) {
    settle_start_ms = now;
    plateau_armed   = false;
    plateau_ref_pct = NAN;
    state = ST_SETTLE;
    Serial.printf("[SETTLE] min %lus then wait for plateau\n", SETTLE_MIN_MS / 1000UL);
    publish_state();
}

static void start_session(const char* how) {
    session_ml       = 0.0f;
    dose_count       = 0;
    grace_used       = false;
    session_baseline = moist_ema;
    last_reason      = "";
    Serial.printf("[SESSION] start (%s) at %.1f%%\n", how, session_baseline);
    begin_dose(DOSE1_ML);   // -> DOSING
}

static void finish_done() {
    pump_off();
    last_reason = "target reached";
    Serial.printf("[SESSION] DONE at %.1f%% | %d mL over %d dose(s)\n",
                  moist_ema, (int)session_ml, dose_count);
    state = ST_MONITOR;     // moisture is high now, will not re-trigger
    publish_state();
}

static void stop_session(const char* reason) {
    pump_off();
    state = ST_STOPPED;     // latched until ACK
    watering_alert(reason);
    Serial.printf("[SESSION] STOPPED: %s | %.1f%% | %d mL\n",
                  reason, moist_ema, (int)session_ml);
}

// Decide next action at a plateau (or after a grace recovery). Compares the
// smoothed moisture to the reading before the last dose.
static void evaluate(unsigned long now) {
    float rise = moist_ema - dose_before;
    Serial.printf("[EVAL] moist %.1f%% | before %.1f%% | rise %+.1f | session %d mL\n",
                  moist_ema, dose_before, rise, (int)session_ml);

    if (moist_ema >= TARGET_PCT) {              // reached target
        finish_done();
    } else if (rise >= ABSORB_RISE_PCT) {       // absorbed, still short
        if ((int)session_ml < SESSION_CAP_ML) {
            begin_dose(SUPPLEMENT_ML);          // -> DOSING -> SETTLE
        } else {
            stop_session("capped: target not reached");
        }
    } else {                                    // stall: soil not taking water
        if (!grace_used) {
            grace_used    = true;
            stall_reading = moist_ema;
            grace_start_ms= now;
            state = ST_GRACE;
            watering_alert("stalled: tray may be holding water");
            Serial.printf("[GRACE] one-time %lus recovery wait\n", GRACE_MS / 1000UL);
        } else {
            stop_session("failed: not absorbing");
        }
    }
}

// ============================== Setup / loop ===============================
static unsigned long sensor_next_ms = 0, publish_next_ms = 0, heartbeat_next_ms = 0;
static unsigned long heartbeat = 0;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Bottom-Watering Control Loop (Phase 5 prototype) ===");
    Serial.println("Autonomous: triggers at <=20%, target 85%; DOSE btn / MQTT 'start' forces a session.");
    Serial.println("Buttons: DOSE(26) start | ACK(33) clear-stop / advance-wait | ABORT(32) stop");
    Serial.printf("Params: dose1 %d, supp %d, cap %d mL | settle %lus, grace %lus\n",
                  DOSE1_ML, SUPPLEMENT_ML, SESSION_CAP_ML,
                  SETTLE_MIN_MS / 1000UL, GRACE_MS / 1000UL);

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

    // ---- sensor read + EMA (decision variable) ----
    if (now >= sensor_next_ms) {
        last_soil = soil_read();
        last_leak = leak_read();
        last_reservoir_empty = reservoir_empty_read();
        if (last_soil.valid) {
            moist_ema = isnan(moist_ema) ? last_soil.pct
                                         : (MOIST_EMA_ALPHA * last_soil.pct
                                            + (1.0f - MOIST_EMA_ALPHA) * moist_ema);
        }
        sensor_next_ms = now + SENSOR_READ_MS;
    }

    // ---- leak debounce ----
    if (last_leak.detected) { if (leak_since_ms == 0) leak_since_ms = now; }
    else                    { leak_since_ms = 0; }
    const bool leak_confirmed = (leak_since_ms != 0 && now - leak_since_ms >= LEAK_DEBOUNCE_MS);

    const State prev = state;
    const bool req_start = pending_start; pending_start = false;
    const bool req_abort = pending_abort; pending_abort = false;
    const bool active = (state == ST_DOSING || state == ST_SETTLE || state == ST_GRACE);

    // ---- safety first: overrides everything, every tick ----
    if (leak_confirmed) {
        pump_off(); last_reason = "leak"; state = ST_LEAK_FAULT;
    } else if (b_abort.edge || req_abort) {
        pump_off(); last_reason = "abort"; state = ST_STOPPED;
    } else if (state == ST_LEAK_FAULT) {
        if (b_ack.edge && !last_leak.detected) { last_reason = ""; state = ST_MONITOR; }
    } else if (state == ST_STOPPED) {
        if (b_ack.edge) { session_ml = 0; dose_count = 0; grace_used = false;
                          last_reason = ""; state = ST_MONITOR; Serial.println("[SESSION] cleared"); }
    } else if (last_reservoir_empty && active) {
        stop_session("reservoir empty");
    } else {
        // ---- normal operation ----
        switch (state) {
            case ST_RESERVOIR_EMPTY:
                if (!last_reservoir_empty) state = ST_MONITOR;
                break;

            case ST_MONITOR:
                if (last_reservoir_empty) { state = ST_RESERVOIR_EMPTY; break; }
                if (b_dose.edge || req_start) {
                    start_session(req_start ? "mqtt" : "button");
                } else if (!isnan(moist_ema) && moist_ema <= TRIGGER_PCT) {
                    start_session("auto");
                }
                break;

            case ST_DOSING:
                if (now - dose_start_ms >= dose_target_ms) {
                    pump_off();
                    enter_settle(now);
                }
                break;

            case ST_SETTLE:
                if (b_ack.edge) {                       // force-advance (test)
                    evaluate(now);
                } else if (now - settle_start_ms >= SETTLE_MIN_MS) {
                    if (!plateau_armed) {
                        plateau_armed   = true;
                        plateau_ref_pct = moist_ema;
                        plateau_ref_ms  = now;
                    } else if (now - plateau_ref_ms >= PLATEAU_WINDOW_MS) {
                        if (fabsf(moist_ema - plateau_ref_pct) <= PLATEAU_SLOPE_PCT) {
                            evaluate(now);               // plateaued
                        } else {
                            plateau_ref_pct = moist_ema; // still moving; re-arm window
                            plateau_ref_ms  = now;
                        }
                    }
                }
                break;

            case ST_GRACE:
                if (b_ack.edge || now - grace_start_ms >= GRACE_MS) {
                    if (moist_ema - stall_reading >= ABSORB_RISE_PCT) {
                        Serial.println("[GRACE] recovered — resuming evaluate");
                        evaluate(now);
                    } else {
                        stop_session("failed: not absorbing");
                    }
                }
                break;

            default: break;
        }
    }

    // ---- account water delivered whenever we leave DOSING ----
    if (prev == ST_DOSING && state != ST_DOSING) {
        session_ml += dose_delivered_ml();
    }

    // ---- LEDs ----
    digitalWrite(LED_YELLOW, state == ST_DOSING);
    digitalWrite(LED_RED, state == ST_LEAK_FAULT || state == ST_STOPPED);
    digitalWrite(LED_GREEN, state == ST_MONITOR || state == ST_SETTLE || state == ST_GRACE);

    if (state != prev) publish_state();

    // ---- telemetry ----
    if (now >= publish_next_ms) {
        publish_soil(last_soil);
        publish_leak(last_leak);
        publish_float(last_reservoir_empty);
        publish_next_ms = now + PUBLISH_MS;
    }

    // ---- heartbeat / progress ----
    if (now >= heartbeat_next_ms) {
        heartbeat++;
        publish_status(heartbeat);
        if (state == ST_SETTLE) {
            long since = (long)((now - settle_start_ms) / 1000UL);
            Serial.printf("[SETTLE] %lds elapsed | soil %.1f%% (ema %.1f, raw %u)\n",
                          since, last_soil.pct, moist_ema, last_soil.raw);
        } else if (state == ST_GRACE) {
            long left = (long)((GRACE_MS - (now - grace_start_ms)) / 1000UL);
            Serial.printf("[GRACE] %lds left | soil %.1f%% vs stall %.1f%%\n",
                          left, moist_ema, stall_reading);
        }
        heartbeat_next_ms = now + HEARTBEAT_MS;
    }
}
