#include "fsm.h"

#include <Arduino.h>
#include <Preferences.h>

#include "config.h"
#include "pump.h"
#include "net_mqtt.h"
#include "buzzer.h"
#include "water_logic.h"

// ============================================================================
// P1-8 (DL-147): the hardened bottom-watering FSM, ported from the calibration
// harness into the integrated firmware's module contract. Replaces the old
// top-water pulse machine (ST_WATERING/MANUAL/DAILY_LIMIT/WATERING_FAULT +
// run_pulse/enter_watering/watchdog) with the settled DL-142 plateau-gated
// volume-dosing loop plus the audit P0 fail-safes:
//   P0-1 reboot-safe watering transaction (NVS) + ST_RECOVERY_HOLD
//   P0-2 independent hard pump-on ceiling (MAX_PUMP_ON_MS) at tick top
//   P0-3 soil freshness (last_valid_soil_ms / ST_SENSOR_FAULT)
//   P0-5 leak-disconnect fail-safe (consumes LeakReading.disconnected)
// Maintenance is boot-default-ON (DL-128): a flash/reboot never auto-waters.
//
// Architecture note: unlike the monolithic harness (which read sensors in
// loop()), here main.cpp reads the sensors and passes them into fsm_tick();
// this module receives SoilReading/FloatReading/LeakReading and owns only the
// decision state, the pump, the NVS transaction, LEDs, buttons, and the state
// publish. Pump primitives live in pump.cpp; state is published via net_mqtt.
// ============================================================================

// ---- state ----------------------------------------------------------------
enum State {
    ST_MONITOR,          // idle: wait for trigger
    ST_DOSING,           // pump running for the current dose
    ST_SETTLE,           // pump off; min settle then wait for plateau
    ST_GRACE,            // pump off; one-time recovery wait after a stall
    ST_RESERVOIR_EMPTY,  // float empty; blocks triggering (auto-clears on refill)
    ST_STOPPED,          // capped / failed / abort / reservoir; latched until ACK
    ST_LEAK_FAULT,       // leak or leak-sensor-disconnect latched; clears on ACK once dry+connected
    ST_MAINTENANCE,      // intentional watering pause (kept for compatibility; maintenance flag is authoritative)
    ST_RECOVERY_HOLD,    // booted into an interrupted session (P0-1): pump off, wait for ACK
    ST_SENSOR_FAULT,     // soil data stale/invalid (P0-3): pump off, wait for ACK once fresh
};
static State state = ST_MONITOR;
static State prev_state = ST_MONITOR;

static const char* state_name(State s) {
    switch (s) {
        case ST_MONITOR:         return "monitor";
        case ST_DOSING:          return "dosing";
        case ST_SETTLE:          return "settle";
        case ST_GRACE:           return "grace";
        case ST_RESERVOIR_EMPTY: return "reservoir_empty";
        case ST_STOPPED:         return "stopped";
        case ST_LEAK_FAULT:      return "leak_fault";
        case ST_MAINTENANCE:     return "maintenance";
        case ST_RECOVERY_HOLD:   return "recovery_hold";
        case ST_SENSOR_FAULT:    return "sensor_fault";
    }
    return "unknown";
}

// ---- session / dose bookkeeping -------------------------------------------
static float         session_ml       = 0.0f;
static int           dose_count       = 0;
static float         session_baseline = NAN;   // % at trigger (reference)
static float         dose_before      = NAN;   // % just before the current dose
static bool          grace_used       = false; // one grace per session
static float         stall_reading    = NAN;   // % when a stall tripped
static const char*   last_reason      = "";    // surfaced in serial/state

static unsigned long dose_start_ms    = 0;
static unsigned long dose_target_ms   = 0;
static int           dose_ml_target   = 0;

// settle / plateau tracking
static unsigned long settle_start_ms  = 0;
static bool          plateau_armed    = false;
static float         plateau_ref_pct  = NAN;
static unsigned long plateau_ref_ms   = 0;

// grace tracking
static unsigned long grace_start_ms   = 0;

// decision variable (smoothed moisture) + freshness (P0-3)
static float         moist_ema        = NAN;
static unsigned long last_valid_soil_ms = 0;

// P0-2: when the pump physically turned on, tracked here since integrated's pump
// module doesn't expose it. Set on every pump-on; read by the hard-ceiling backstop.
static unsigned long pump_on_since_ms = 0;
static void pump_on_tracked()  { if (!pump_is_on()) { pump_on(); pump_on_since_ms = millis(); } }

// ---- maintenance (DL-128): boot-default ON so a flash never auto-waters ----
static bool          maintenance      = true;

// ---- inbound intents (set outside the tick) -------------------------------
static volatile bool maint_req_pending = false;
static volatile bool maint_req_on      = false;
void fsm_request_maintenance(bool on) { maint_req_on = on; maint_req_pending = true; }

// ---- P0-1: reboot-safe watering transaction (NVS) -------------------------
static Preferences   nvs;
static const char*   NVS_NS = "water";
static void nvs_save_txn(bool dosing_now) {
    nvs.begin(NVS_NS, false);
    nvs.putUChar("state",   (uint8_t)state);
    nvs.putFloat("sess_ml", session_ml);
    nvs.putInt("dose_cnt",  dose_count);
    nvs.putBool("dosing",   dosing_now);
    nvs.end();
}

// ---- leak debounce state --------------------------------------------------
static unsigned long leak_since_ms = 0;

// ---- LED blink phase ------------------------------------------------------
static bool          blink_on      = false;
static unsigned long blink_last_ms = 0;

// ---- state publish cadence ------------------------------------------------
static unsigned long state_pub_next_ms = 0;

// ---- buttons (integrated infra; STOP<->abort, ACK, MANUAL<->dose) ----------
struct Button {
    uint8_t       pin;
    int           last_raw;
    int           stable;
    unsigned long change_ms;
    bool          pressed_edge;
    bool          released_edge;
    bool          long_edge;
    unsigned long press_ms;
    bool          long_fired;
};
static Button btn_stop{0, HIGH, HIGH, 0, false, false, false, 0, false};
static Button btn_ack{0, HIGH, HIGH, 0, false, false, false, 0, false};
static Button btn_manual{0, HIGH, HIGH, 0, false, false, false, 0, false};

static void button_init(Button& b, uint8_t pin) {
    b.pin = pin; b.last_raw = HIGH; b.stable = HIGH; b.change_ms = 0;
    b.pressed_edge = false; b.released_edge = false; b.long_edge = false;
    b.press_ms = 0; b.long_fired = false;
    pinMode(pin, INPUT_PULLUP);
}

static void button_update(Button& b, unsigned long now) {
    b.pressed_edge = false; b.released_edge = false; b.long_edge = false;
    const int raw = digitalRead(b.pin);
    if (raw != b.last_raw) { b.last_raw = raw; b.change_ms = now; }
    if (now - b.change_ms >= BUTTON_DEBOUNCE_MS && raw != b.stable) {
        b.stable = raw;
        if (b.stable == LOW) { b.pressed_edge = true; b.press_ms = now; b.long_fired = false; }
        else                 { b.released_edge = true; }
    }
    if (b.stable == LOW && !b.long_fired && now - b.press_ms >= BTN_LONGPRESS_MS) {
        b.long_edge = true; b.long_fired = true;
    }
}

// ---- LEDs -----------------------------------------------------------------
static void drive_leds() {
    // YELLOW = actively dosing; RED = any latched fault/hold; GREEN = healthy idle/waiting.
    const bool dosing = (state == ST_DOSING);
    const bool fault  = (state == ST_LEAK_FAULT || state == ST_STOPPED ||
                         state == ST_RECOVERY_HOLD || state == ST_SENSOR_FAULT);
    const bool green  = (state == ST_MONITOR || state == ST_SETTLE ||
                         state == ST_GRACE || state == ST_MAINTENANCE);
    digitalWrite(LED_YELLOW, dosing ? HIGH : LOW);
    digitalWrite(LED_RED,    fault  ? HIGH : LOW);
    digitalWrite(LED_GREEN,  green  ? HIGH : LOW);
}

// ---- state publish helper -------------------------------------------------
static void publish_state_now() {
    const float m = isnan(moist_ema) ? -1.0f : moist_ema;
    mqtt_publish_state(state_name(state), pump_is_on(), (int)session_ml,
                       dose_count, m, maintenance, last_reason);
}

// ---- dose / session control -----------------------------------------------
static float dose_delivered_ml() {
    unsigned long on_ms = millis() - dose_start_ms;
    if (on_ms > dose_target_ms) on_ms = dose_target_ms;
    return (float)on_ms / 1000.0f * PUMP_ML_PER_SEC;
}

static void begin_dose(int ml, bool reservoir_empty) {
    if (reservoir_empty) { last_reason = "reservoir empty"; state = ST_STOPPED; pump_off(); return; }
    ml = water_logic::clamp_dose(ml, (int)session_ml, MAX_DOSE_ML, SESSION_CAP_ML);
    if (ml <= 0) {
        // Budget exhausted (or nothing to give): capped -> stop, matching prior behavior.
        if ((int)session_ml >= SESSION_CAP_ML) {
            last_reason = "capped: target not reached"; state = ST_STOPPED; pump_off();
        }
        return;
    }

    dose_before    = moist_ema;
    dose_ml_target = ml;
    dose_target_ms = (unsigned long)((float)ml / PUMP_ML_PER_SEC * 1000.0f);
    dose_start_ms  = millis();
    dose_count++;
    state = ST_DOSING;
    nvs_save_txn(true);     // P0-1: record "dosing" in NVS BEFORE any water flows
    pump_on_tracked();
    Serial.printf("[DOSE] #%d: %d mL (%lu ms) | before %.1f%% | session -> %d/%d mL\n",
                  dose_count, ml, dose_target_ms, dose_before,
                  (int)session_ml + ml, SESSION_CAP_ML);
    publish_state_now();
}

static void enter_settle(unsigned long now) {
    settle_start_ms = now;
    plateau_armed   = false;
    plateau_ref_pct = NAN;
    state = ST_SETTLE;
    Serial.printf("[SETTLE] min %lus then wait for plateau\n", SETTLE_MIN_MS / 1000UL);
    publish_state_now();
}

static void start_session(const char* how, bool reservoir_empty) {
    session_ml       = 0.0f;
    dose_count       = 0;
    grace_used       = false;
    session_baseline = moist_ema;
    last_reason      = "";
    Serial.printf("[SESSION] start (%s) at %.1f%%\n", how, session_baseline);
    begin_dose(DOSE1_ML, reservoir_empty);   // -> DOSING
}

static void finish_done() {
    pump_off();
    last_reason = "target reached";
    Serial.printf("[SESSION] DONE at %.1f%% | %d mL over %d dose(s)\n",
                  moist_ema, (int)session_ml, dose_count);
    state = ST_MONITOR;
    publish_state_now();
}

static void stop_session(const char* reason) {
    pump_off();
    last_reason = reason;          // F1 (DL-163): must set BEFORE publish, else the
    state = ST_STOPPED;            // stale prior reason ships and the alert never fires
    publish_state_now();
    Serial.printf("[SESSION] STOPPED: %s | %.1f%% | %d mL\n",
                  reason, moist_ema, (int)session_ml);
}

static void evaluate(unsigned long now, bool reservoir_empty) {
    float rise = moist_ema - dose_before;
    Serial.printf("[EVAL] moist %.1f%% | before %.1f%% | rise %+.1f | session %d mL\n",
                  moist_ema, dose_before, rise, (int)session_ml);

    switch (water_logic::evaluate_decision(moist_ema, rise, (int)session_ml,
                                           grace_used, TARGET_PCT, ABSORB_RISE_PCT,
                                           SESSION_CAP_ML)) {
        case water_logic::Decision::DONE:
            finish_done();
            break;
        case water_logic::Decision::SUPPLEMENT:
            begin_dose(SUPPLEMENT_ML, reservoir_empty);
            break;
        case water_logic::Decision::STOP_CAPPED:
            stop_session("capped: target not reached");
            break;
        case water_logic::Decision::GRACE:
            grace_used     = true;
            stall_reading  = moist_ema;
            grace_start_ms = now;
            state = ST_GRACE;
            publish_state_now();
            Serial.printf("[GRACE] one-time %lus recovery wait\n", GRACE_MS / 1000UL);
            break;
        case water_logic::Decision::STOP_FAILED:
            stop_session("failed: not absorbing");
            break;
    }
}

// ---- begin ----------------------------------------------------------------
void fsm_begin() {
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);

    button_init(btn_stop, BTN_STOP);
    button_init(btn_ack, BTN_ACK);
    button_init(btn_manual, BTN_MANUAL);

    pump_begin();
    buzzer_begin();

    // Boot-default maintenance ON (DL-128): a flash/reboot must never auto-water.
    maintenance = true;
    Serial.println("[MAINT] boot default: maintenance ON (auto-watering disabled; arm via MQTT)");

    // P0-1 reboot-safe recovery: if NVS shows a session was interrupted mid-flight,
    // hold pump-off in RECOVERY_HOLD and wait for an operator ACK rather than resuming.
    {
        nvs.begin(NVS_NS, true);
        uint8_t saved  = nvs.getUChar("state", (uint8_t)ST_MONITOR);
        bool    dosing = nvs.getBool("dosing", false);
        float   sess   = nvs.getFloat("sess_ml", 0.0f);
        int     dcount = nvs.getInt("dose_cnt", 0);
        nvs.end();
        bool interrupted = dosing ||
                           saved == ST_DOSING || saved == ST_SETTLE || saved == ST_GRACE;
        if (interrupted) {
            session_ml = sess;
            dose_count = dcount;
            state      = ST_RECOVERY_HOLD;
            pump_off();
            nvs_save_txn(false);
            Serial.printf("[RECOVERY] interrupted session (%d mL, %d dose). Pump OFF, holding for ACK.\n",
                          (int)session_ml, dose_count);
        } else {
            state = ST_MONITOR;
        }
    }
    prev_state = state;
}

// ---- tick -----------------------------------------------------------------
void fsm_tick(const SoilReading& soil, const FloatReading& flt, const LeakReading& leak) {
    const unsigned long now = millis();

    // Capture the entry state FIRST — before the P0-2 ceiling below can change it —
    // so dose accounting and the NVS/publish-on-transition checks compare against the
    // true state at tick entry. (Audit #2 / DL-164: the ceiling used to run before
    // prev_state was assigned, so a mid-dose cutoff lost the delivered-water accounting
    // and never persisted/published the STOPPED transition.)
    prev_state = state;

    // P0-2: independent hard pump ceiling, BEFORE anything else. Pure backstop:
    // can only ever turn the pump OFF, whatever the FSM or a stuck path is doing.
    if (pump_is_on() && (uint32_t)(now - pump_on_since_ms) >= MAX_PUMP_ON_MS) {
        pump_off();
        last_reason = "pump max-runtime exceeded";
        state = ST_STOPPED;
        Serial.println("[SAFETY] pump max-runtime exceeded — pump OFF, latched STOPPED");
    }

    button_update(btn_stop, now);
    button_update(btn_ack, now);
    button_update(btn_manual, now);

    // Decision variable + freshness (P0-3). main.cpp reads sensors; we smooth here.
    if (soil.valid) {
        last_valid_soil_ms = now;
        moist_ema = isnan(moist_ema) ? soil.moisture_pct
                                     : (MOIST_EMA_ALPHA * soil.moisture_pct
                                        + (1.0f - MOIST_EMA_ALPHA) * moist_ema);
    }
    const bool soil_stale = water_logic::is_soil_stale(
        !isnan(moist_ema), (unsigned long)(now - last_valid_soil_ms), SOIL_STALE_MS);

    const bool reservoir_empty = flt.reservoir_empty;

    // Leak debounce (P0-5): a disconnected sensor is a fault, folded in with a real leak.
    const bool leak_or_disc = leak.detected || leak.disconnected;
    if (leak_or_disc) { if (leak_since_ms == 0) leak_since_ms = now; }
    else              { leak_since_ms = 0; }
    const bool leak_confirmed = (leak_since_ms != 0 && now - leak_since_ms >= LEAK_DEBOUNCE_MS);

    // Consume inbound maintenance request (MANUAL long-press or MQTT).
    const bool active = (state == ST_DOSING || state == ST_SETTLE || state == ST_GRACE);
    bool maint_toggle_req = false, maint_toggle_on = false;
    if (maint_req_pending) { maint_toggle_req = true; maint_toggle_on = maint_req_on; maint_req_pending = false; }
    if (btn_manual.long_edge) { maint_toggle_req = true; maint_toggle_on = !maintenance; }  // MANUAL long-press toggles
    if (maint_toggle_req) {
        if (maint_toggle_on && !maintenance) {
            maintenance = true;
            if (active) { pump_off(); last_reason = "maintenance"; state = ST_STOPPED; }
            Serial.println("[MAINT] entering maintenance (auto-watering disabled)");
        } else if (!maint_toggle_on && maintenance) {
            maintenance = false;
            Serial.println("[MAINT] armed (auto-watering enabled)");
        }
    }

    // Manual dose intent: MANUAL short-press (release without a long-press) or... (MQTT 'start'
    // is not wired in integrated yet; DL-145 deferred cmd/dose — MANUAL button is the manual path).
    const bool req_start = (btn_manual.released_edge && !btn_manual.long_fired);
    const bool req_abort = (btn_stop.pressed_edge);

    // ---- safety first: overrides everything, every tick ----
    if (leak_confirmed) {
        pump_off();
        last_reason = leak.disconnected ? "leak sensor disconnected" : "leak";
        state = ST_LEAK_FAULT;
    } else if (req_abort) {
        pump_off(); last_reason = "abort"; state = ST_STOPPED;
    } else if (soil_stale && active) {
        pump_off(); last_reason = "soil sensor stale"; state = ST_SENSOR_FAULT;
        Serial.println("[SENSOR] soil stale during session — pump OFF, latched SENSOR_FAULT");
    } else if (state == ST_LEAK_FAULT) {
        if (btn_ack.pressed_edge && !leak.detected && !leak.disconnected) {
            last_reason = ""; state = ST_MONITOR;
        }
    } else if (state == ST_SENSOR_FAULT) {
        pump_off();
        if (btn_ack.pressed_edge && !soil_stale) {
            last_reason = ""; state = ST_MONITOR;
            Serial.println("[SENSOR] ACK — soil fresh again, cleared");
        }
    } else if (state == ST_STOPPED) {
        if (btn_ack.pressed_edge) {
            session_ml = 0; dose_count = 0; grace_used = false;
            last_reason = ""; state = ST_MONITOR; Serial.println("[SESSION] cleared");
        }
    } else if (state == ST_RECOVERY_HOLD) {
        pump_off();
        if (btn_ack.pressed_edge) {
            last_reason = "recovery cleared";
            state = ST_STOPPED;
            Serial.println("[RECOVERY] ACK — cleared to STOPPED (ACK again to re-arm)");
        }
    } else if (reservoir_empty && active) {
        stop_session("reservoir empty");
    } else {
        // ---- normal operation ----
        switch (state) {
            case ST_RESERVOIR_EMPTY:
                if (!reservoir_empty) state = ST_MONITOR;
                break;

            case ST_MONITOR:
                if (reservoir_empty) { state = ST_RESERVOIR_EMPTY; break; }
                if (req_start) {
                    if (soil_stale) Serial.println("[SENSOR] manual dose rejected — soil stale");
                    else start_session("button", reservoir_empty);
                } else if (water_logic::should_trigger(moist_ema, maintenance, soil_stale, TRIGGER_PCT)) {
                    start_session("auto", reservoir_empty);
                }
                break;

            case ST_DOSING:
                if (now - dose_start_ms >= dose_target_ms) {
                    pump_off();
                    enter_settle(now);
                }
                break;

            case ST_SETTLE:
                if (water_logic::target_reached(moist_ema, TARGET_PCT)) {
                    evaluate(now, reservoir_empty);
                } else if (btn_ack.pressed_edge) {
                    evaluate(now, reservoir_empty);
                } else if (now - settle_start_ms >= SETTLE_MIN_MS) {
                    if (!plateau_armed) {
                        plateau_armed   = true;
                        plateau_ref_pct = moist_ema;
                        plateau_ref_ms  = now;
                    } else if (water_logic::is_plateau(
                                   moist_ema, plateau_ref_pct,
                                   (unsigned long)(now - plateau_ref_ms),
                                   PLATEAU_WINDOW_MS, PLATEAU_SLOPE_PCT)) {
                        evaluate(now, reservoir_empty);
                    } else if (now - plateau_ref_ms >= PLATEAU_WINDOW_MS) {
                        // window elapsed but not flat -> re-arm the reference
                        plateau_ref_pct = moist_ema;
                        plateau_ref_ms  = now;
                    }
                }
                break;

            case ST_GRACE:
                if (water_logic::target_reached(moist_ema, TARGET_PCT)) {
                    evaluate(now, reservoir_empty);
                } else if (btn_ack.pressed_edge || now - grace_start_ms >= GRACE_MS) {
                    if (moist_ema - stall_reading >= ABSORB_RISE_PCT) {
                        Serial.println("[GRACE] recovered — resuming evaluate");
                        evaluate(now, reservoir_empty);
                    } else {
                        stop_session("failed: not absorbing");
                    }
                }
                break;

            default: break;
        }
    }

    // Account water delivered whenever we leave DOSING.
    if (prev_state == ST_DOSING && state != ST_DOSING) {
        session_ml += dose_delivered_ml();
    }

    // Blink phase for any blinking LED states (none blink in this port, but keep the tick).
    if (now - blink_last_ms >= LED_BLINK_MS) { blink_on = !blink_on; blink_last_ms = now; }
    drive_leds();
    buzzer_update(state == ST_LEAK_FAULT, now);

    // Persist the transaction + publish on every state change (P0-1).
    if (state != prev_state) {
        nvs_save_txn(state == ST_DOSING);
        Serial.print("[FSM] -> "); Serial.println(state_name(state));
        publish_state_now();
    }

    // Periodic retained state refresh.
    if (now >= state_pub_next_ms) {
        publish_state_now();
        state_pub_next_ms = now + MQTT_PUBLISH_INTERVAL_MS;
    }
}

// ---- accessors ------------------------------------------------------------
const char* fsm_state_name() { return state_name(state); }
int fsm_session_ml()  { return (int)session_ml; }
int fsm_dose_count()  { return dose_count; }
