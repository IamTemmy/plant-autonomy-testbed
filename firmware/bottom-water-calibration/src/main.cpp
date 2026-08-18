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
#include <Preferences.h>   // NVS: persist the watering transaction across reboots (DL-127 / audit P0-1)
#include <math.h>

// P1-9: build identity. platformio.ini injects the git SHA; fall back if built without it.
#ifndef FW_GIT_SHA
#define FW_GIT_SHA "unknown"
#endif

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
static constexpr uint16_t SOIL_RAW_WET       = 1700;  // 100% anchor: true saturation (DL-121; was 2250, clipped the wet end)

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
static constexpr char     T_MAINT[]  = "plant/cmd/maintenance";  // inbound: "on" | "off" (DL-128)

// ======================= Control-loop parameters (DL-107 spec; DL-124 pass-2) =============
// Pass-2 (DL-124): single metered dose by volume, replacing iterate-and-re-dose.
// SESSION_CAP == DOSE1 makes a supplement structurally impossible (budget hits 0 after
// the first dose), so the loop dispenses exactly one dose then settles/stops -- the
// DL-117 redesign, achieved with the proven harness code rather than an FSM rewrite.
static constexpr float    TRIGGER_PCT      = 30.0f;   // water at/below this (kept safely above the ~6% wilt zone, DL-124)
static constexpr float    TARGET_PCT       = 70.0f;   // healthy-moist, not saturated (DL-117/120 over-dose lesson)
static constexpr int      DOSE1_ML         = 100;     // DL-125 study dose (2nd increment after the DL-124 150 mL); tray-safe fill
static constexpr int      SUPPLEMENT_ML    = 0;       // no supplements (single metered dose)
static constexpr int      MAX_DOSE_ML      = 100;     // per-dose hard cap
static constexpr int      SESSION_CAP_ML   = 100;     // == DOSE1 -> single dose, no supplement possible
static constexpr float    ABSORB_RISE_PCT  = 7.0f;    // rise that counts as "absorbed"
static constexpr uint32_t SETTLE_MIN_MS    = 3UL * 60UL * 60UL * 1000UL;   // 3 h
static constexpr uint32_t PLATEAU_WINDOW_MS= 30UL * 60UL * 1000UL;          // 30 min
static constexpr float    PLATEAU_SLOPE_PCT= 1.0f;    // <= this over a window = plateaued
static constexpr uint32_t GRACE_MS         = 90UL * 60UL * 1000UL;          // 1.5 h (one-time)

// Cadence + smoothing
static constexpr uint32_t SENSOR_READ_MS       = 2000;
// Independent hard pump-on ceiling (audit P0-2). A physical backstop enforced at the
// very top of loop() BEFORE any network work: the pump can never run longer than this,
// regardless of FSM state, a stuck dose-cutoff, or a blocked MQTT reconnect. Set safely
// above the largest legitimate dose (150 mL ~= 150 s at the pump rate) so it never trips
// in normal operation but bounds any runaway.
static constexpr uint32_t MAX_PUMP_ON_MS       = 165000;   // 165 s
static constexpr uint32_t PUBLISH_MS           = 5000;
static constexpr uint32_t HEARTBEAT_MS         = 10000;
static constexpr uint32_t WIFI_BOOT_TIMEOUT_MS = 10000;
static constexpr uint32_t MQTT_RETRY_MS        = 5000;
static constexpr uint32_t BTN_DEBOUNCE_MS      = 50;
static constexpr float    MOIST_EMA_ALPHA      = 0.1f;  // decision variable smoothing
// Soil freshness (audit P0-3). If no VALID soil reading arrives within this window, the
// probe is treated as faulted: auto-watering is blocked and (if dosing) the pump is cut.
// Generous vs the 2 s read cadence so a couple of missed reads don't trip it.
static constexpr uint32_t SOIL_STALE_MS        = 30000;  // 30 s

// ============================== State machine ==============================
enum State {
    ST_MONITOR,          // idle: read + publish; wait for trigger
    ST_DOSING,           // pump running for the current dose
    ST_SETTLE,           // pump off; 3 h min then wait for plateau
    ST_GRACE,            // pump off; one-time 1.5 h recovery wait after a stall
    ST_RESERVOIR_EMPTY,  // float empty; blocks triggering (auto-clears on refill)
    ST_STOPPED,          // capped / failed / abort / reservoir; latched until ACK
    ST_LEAK_FAULT,       // leak latched; clears on ACK once dry
    ST_RECOVERY_HOLD,    // booted into an interrupted session (P0-1): pump off, autonomy blocked, wait for ACK
    ST_SENSOR_FAULT      // soil data stale/invalid (P0-3): pump off, autonomy blocked, clears on ACK once fresh
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
        case ST_RECOVERY_HOLD:   return "recovery_hold";
        case ST_SENSOR_FAULT:    return "sensor_fault";
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

// ---- reboot-safe watering transaction (audit P0-1 / DL-127) ----
// The session (state, delivered volume, whether a dose is mid-flight) lives in RAM,
// so a reset during dosing/settle/grace would otherwise return to ST_MONITOR zeroed
// and — with the probe reading low for hours after a bottom-dose (DL-125) — start a
// fresh dose, delivering water with no memory of what was already given. We mirror a
// tiny transaction to NVS on every state transition (not every tick, to spare flash),
// and on boot refuse to resume autonomy if a session was interrupted mid-flight.
// (A persisted rolling daily water allowance is a separate follow-up: it needs a
// wall-clock reset window, deferred to keep this change focused.)
static Preferences   nvs;
static const char*   NVS_NS = "water";

// Persist the current transaction. `dosing_now` = a pump dose is physically in flight.
static void nvs_save_txn(bool dosing_now) {
    nvs.begin(NVS_NS, false);
    nvs.putUChar("state",   (uint8_t)state);
    nvs.putFloat("sess_ml", session_ml);
    nvs.putInt("dose_cnt",  dose_count);
    nvs.putBool("dosing",   dosing_now);
    nvs.end();
}

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
static volatile int  pending_maint = 0;   // 0 = none, +1 = enter maintenance, -1 = arm (DL-128)

// ---- maintenance (safe resting state, DL-128) ----
// When true, the board monitors/publishes normally but NEVER auto-triggers a dose.
// Manual doses (DOSE button / MQTT 'start') are still allowed for controlled tests.
// Defaults ON at every boot (see setup) so a flash/reboot never fires the pump; the
// operator explicitly arms watering via MQTT when ready. Persisted so a reboot mid-armed
// is faithful only after an explicit arm — but boot always re-asserts maintenance first.
static bool maintenance = true;

// ============================== I/O primitives =============================
static WiFiClient   wifi_client;
static PubSubClient mqtt(wifi_client);

static bool pump_state = false;
static unsigned long pump_on_ms = 0;         // when the pump physically turned on (P0-2 hard deadline)
static unsigned long last_valid_soil_ms = 0; // when the last VALID soil reading arrived (P0-3 freshness)
static void pump_on()  { if (!pump_state) { pump_state = true;  pump_on_ms = millis(); digitalWrite(PUMP_GATE_PIN, HIGH); Serial.println("[PUMP] ON"); } }
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
             "\"moist_pct\":%.1f,\"maintenance\":%d,\"reason\":\"%s\"}",
             state_name(state), pump_is_on() ? 1 : 0, (int)session_ml, dose_count,
             m, maintenance ? 1 : 0, last_reason);
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
    char p[128];
    snprintf(p, sizeof(p),
             "{\"online\":true,\"uptime_s\":%lu,\"rssi\":%d,\"heartbeat\":%lu,\"fw\":\"water-loop\",\"build\":\"%s\"}",
             millis() / 1000UL, WiFi.RSSI(), hb, FW_GIT_SHA);
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
    char buf[16];
    unsigned int n = len < sizeof(buf) - 1 ? len : sizeof(buf) - 1;
    memcpy(buf, payload, n);
    buf[n] = '\0';
    if (strcmp(topic, T_MAINT) == 0) {
        if      (strcmp(buf, "on")  == 0) { pending_maint = +1; Serial.println("MQTT: maintenance on"); }
        else if (strcmp(buf, "off") == 0) { pending_maint = -1; Serial.println("MQTT: maintenance off (arm)"); }
        else Serial.println("MQTT: maint cmd ignored (want 'on' or 'off')");
        return;
    }
    if (strcmp(topic, T_CMD) != 0) return;
    if      (strcmp(buf, "abort") == 0) { pending_abort = true; Serial.println("MQTT: abort"); }
    else if (strcmp(buf, "start") == 0) { pending_start = true; Serial.println("MQTT: start"); }
    else Serial.println("MQTT: cmd ignored (want 'start' or 'abort')");
}

static unsigned long mqtt_last_attempt_ms = 0;
static void mqtt_tick() {
    if (WiFi.status() != WL_CONNECTED) return;
    if (mqtt.connected()) { mqtt.loop(); return; }
    unsigned long now = millis();
    if ((uint32_t)(now - mqtt_last_attempt_ms) < MQTT_RETRY_MS) return;   // P0-4: rollover-safe
    mqtt_last_attempt_ms = now;
    Serial.print("MQTT: connecting... ");
    const char* will = "{\"online\":false}";
    if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, T_STATUS, 0, true, will)) {
        Serial.println("connected.");
        publish_status(0);
        mqtt.subscribe(T_CMD);
        mqtt.subscribe(T_MAINT);
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
    state = ST_DOSING;
    nvs_save_txn(true);     // P0-1: record "dosing" in NVS BEFORE any water flows
    pump_on();
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
static unsigned long sensor_last_ms = 0, publish_last_ms = 0, heartbeat_last_ms = 0;
static unsigned long heartbeat = 0;

void setup() {
    Serial.begin(115200);
    delay(500);
    // P1-9: print the ACTUAL compiled constants + a build stamp so the banner can never
    // drift from the code again (the old hardcoded "20%/85%" line caused a false deploy
    // diagnosis). FW_GIT_SHA is injected by platformio.ini.
    Serial.println("\n=== Bottom-Watering Control Loop (Phase 5 prototype) ===");
    Serial.printf("Build: %s\n", FW_GIT_SHA);
    Serial.printf("Autonomous: triggers at <=%.0f%%, target %.0f%%; DOSE btn / MQTT 'start' forces a session.\n",
                  TRIGGER_PCT, TARGET_PCT);
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
    mqtt.setSocketTimeout(2);   // P0-2: bound a blocked reconnect to ~2 s, not the default 15 s

    // ---- always boot into maintenance (DL-128, Q1-a) ----
    // A flash/reboot must never fire the pump. The board comes up monitoring-only;
    // auto-watering is enabled explicitly via `plant/cmd/maintenance off`. This is
    // re-asserted every boot regardless of any prior armed state.
    maintenance = true;
    Serial.println("[MAINT] boot default: maintenance ON (auto-watering disabled; arm via MQTT)");

    // ---- reboot-safe recovery (audit P0-1) ----
    // If NVS shows a session was interrupted mid-flight (a dose was in progress, or we
    // were in dosing/settle/grace), do NOT resume autonomy. Boot pump-off into
    // RECOVERY_HOLD and wait for an operator ACK — because the probe reads low for hours
    // after a bottom-dose (DL-125), a naive return to MONITOR would re-dose blindly.
    {
        nvs.begin(NVS_NS, true);   // read-only
        uint8_t saved   = nvs.getUChar("state", (uint8_t)ST_MONITOR);
        bool    dosing  = nvs.getBool("dosing", false);
        float   sess    = nvs.getFloat("sess_ml", 0.0f);
        int     dcount  = nvs.getInt("dose_cnt", 0);
        nvs.end();
        bool interrupted = dosing ||
                           saved == ST_DOSING || saved == ST_SETTLE || saved == ST_GRACE;
        if (interrupted) {
            // Conservative: assume the in-flight/last dose was fully delivered — never
            // double-dose. Carry the delivered volume forward so it's visible.
            session_ml = sess;
            dose_count = dcount;
            state      = ST_RECOVERY_HOLD;
            pump_off();
            nvs_save_txn(false);   // clear the dosing flag; hold state persists
            Serial.printf("[RECOVERY] interrupted session found (was %s, %d mL, %d dose). "
                          "Pump OFF, holding for ACK.\n",
                          state_name((State)saved), (int)session_ml, dose_count);
        }
    }
}

void loop() {
    const unsigned long now = millis();

    // ---- P0-2: independent physical pump safety, BEFORE any network work ----
    // The pump can never run past MAX_PUMP_ON_MS, whatever the FSM or a blocked MQTT
    // reconnect is doing. This is a pure backstop: it can only ever turn the pump OFF.
    if (pump_is_on() && (uint32_t)(now - pump_on_ms) >= MAX_PUMP_ON_MS) {
        pump_off();
        last_reason = "pump max-runtime exceeded";
        state = ST_STOPPED;
        Serial.println("[SAFETY] pump max-runtime exceeded — pump OFF, latched STOPPED");
    }

    button_update(b_dose, now);
    button_update(b_ack, now);
    button_update(b_abort, now);

    // ---- sensor read + EMA (decision variable) ----
    if ((uint32_t)(now - sensor_last_ms) >= SENSOR_READ_MS) {   // P0-4: rollover-safe
        last_soil = soil_read();
        last_leak = leak_read();
        last_reservoir_empty = reservoir_empty_read();
        if (last_soil.valid) {
            last_valid_soil_ms = now;   // P0-3: freshness timestamp
            moist_ema = isnan(moist_ema) ? last_soil.pct
                                         : (MOIST_EMA_ALPHA * last_soil.pct
                                            + (1.0f - MOIST_EMA_ALPHA) * moist_ema);
        }
        sensor_last_ms = now;
    }

    // P0-3: soil is "stale" if no valid reading has arrived within SOIL_STALE_MS, or we
    // have never had one (EMA still NaN). A stale probe must not drive watering.
    const bool soil_stale = isnan(moist_ema) ||
                            (uint32_t)(now - last_valid_soil_ms) > SOIL_STALE_MS;

    // ---- leak debounce ----
    if (last_leak.detected) { if (leak_since_ms == 0) leak_since_ms = now; }
    else                    { leak_since_ms = 0; }
    const bool leak_confirmed = (leak_since_ms != 0 && now - leak_since_ms >= LEAK_DEBOUNCE_MS);

    const State prev = state;
    const bool req_start = pending_start; pending_start = false;
    const bool req_abort = pending_abort; pending_abort = false;
    const int  req_maint = pending_maint; pending_maint = 0;
    const bool active = (state == ST_DOSING || state == ST_SETTLE || state == ST_GRACE);

    // ---- maintenance toggle (DL-128): entering maintenance is always safe; if a
    // session is active, entering maintenance aborts it to STOPPED first. Arming never
    // itself starts a dose — the auto-trigger fires on a later tick only if warranted.
    if (req_maint == +1 && !maintenance) {
        maintenance = true;
        if (active) { pump_off(); last_reason = "maintenance"; state = ST_STOPPED; }
        Serial.println("[MAINT] entering maintenance (auto-watering disabled)");
    } else if (req_maint == -1 && maintenance) {
        maintenance = false;
        Serial.println("[MAINT] armed (auto-watering enabled)");
    }

    // ---- safety first: overrides everything, every tick ----
    if (leak_confirmed) {
        pump_off(); last_reason = "leak"; state = ST_LEAK_FAULT;
    } else if (b_abort.edge || req_abort) {
        pump_off(); last_reason = "abort"; state = ST_STOPPED;
    } else if (soil_stale && active) {
        // P0-3: soil data went stale/invalid during an active session. Cut the pump and
        // latch a sensor fault — never keep operating the pump on a dead/frozen probe.
        pump_off(); last_reason = "soil sensor stale"; state = ST_SENSOR_FAULT;
        Serial.println("[SENSOR] soil stale during session — pump OFF, latched SENSOR_FAULT");
    } else if (state == ST_LEAK_FAULT) {
        if (b_ack.edge && !last_leak.detected) { last_reason = ""; state = ST_MONITOR; }
    } else if (state == ST_SENSOR_FAULT) {
        // Clears on ACK, but only once fresh valid data has returned (like leak needs dry).
        pump_off();
        if (b_ack.edge && !soil_stale) { last_reason = ""; state = ST_MONITOR;
                                         Serial.println("[SENSOR] ACK — soil fresh again, cleared"); }
    } else if (state == ST_STOPPED) {
        if (b_ack.edge) { session_ml = 0; dose_count = 0; grace_used = false;
                          last_reason = ""; state = ST_MONITOR; Serial.println("[SESSION] cleared"); }
    } else if (state == ST_RECOVERY_HOLD) {
        // P0-1: booted into an interrupted session. Keep the pump off and block all
        // autonomy. An operator ACK clears the hold to the latched STOPPED state (a
        // second ACK then re-arms to MONITOR) — two deliberate actions to resume.
        pump_off();
        if (b_ack.edge) {
            last_reason = "recovery cleared";
            state = ST_STOPPED;
            Serial.println("[RECOVERY] ACK — cleared to STOPPED (ACK again to re-arm)");
        }
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
                    // Manual doses are allowed even in maintenance, but NOT on a stale probe
                    // (P0-3): a dose with no valid soil data can't be verified or guarded.
                    if (soil_stale) Serial.println("[SENSOR] manual dose rejected — soil stale");
                    else start_session(req_start ? "mqtt" : "button");
                } else if (!maintenance && !soil_stale && moist_ema <= TRIGGER_PCT) {
                    start_session("auto");                          // auto: blocked in maintenance (DL-128) or when soil stale (P0-3)
                }
                break;

            case ST_DOSING:
                if (now - dose_start_ms >= dose_target_ms) {
                    pump_off();
                    enter_settle(now);
                }
                break;

            case ST_SETTLE:
                if (moist_ema >= TARGET_PCT) {           // target is a hard stop — do not wait for plateau
                    evaluate(now);
                } else if (b_ack.edge) {                 // force-advance (test)
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
                if (moist_ema >= TARGET_PCT) {           // target reached during grace — done now
                    evaluate(now);
                } else if (b_ack.edge || now - grace_start_ms >= GRACE_MS) {
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
    digitalWrite(LED_RED, state == ST_LEAK_FAULT || state == ST_STOPPED || state == ST_RECOVERY_HOLD || state == ST_SENSOR_FAULT);
    digitalWrite(LED_GREEN, state == ST_MONITOR || state == ST_SETTLE || state == ST_GRACE);

    // ---- persist the watering transaction on every state change (P0-1) ----
    // One central point catches all transitions (dose start/end, settle, grace, stop,
    // done). If power is lost mid-session, boot reads this back and refuses to resume.
    if (state != prev) {
        nvs_save_txn(state == ST_DOSING);
        publish_state();
    }

    // ---- telemetry ----
    if ((uint32_t)(now - publish_last_ms) >= PUBLISH_MS) {   // P0-4: rollover-safe
        publish_soil(last_soil);
        publish_leak(last_leak);
        publish_float(last_reservoir_empty);
        publish_last_ms = now;
    }

    // ---- heartbeat / progress ----
    if ((uint32_t)(now - heartbeat_last_ms) >= HEARTBEAT_MS) {   // P0-4: rollover-safe
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
        heartbeat_last_ms = now;
    }
    // P0-4 note: all periodic timers now use the rollover-safe elapsed idiom
    // `(uint32_t)(now - last) >= interval`, correct across the ~49.7-day millis() wrap.
    // TODO(P2-13): add a host-compilable simulated-rollover test near UINT32_MAX.

    // ---- network LAST (P0-2): all physical safety + FSM + telemetry above run every
    // loop before any (potentially blocking) MQTT reconnect is attempted. Incoming
    // commands (abort/start/maintenance) set flags consumed at the top of the next loop.
    mqtt_tick();
}
