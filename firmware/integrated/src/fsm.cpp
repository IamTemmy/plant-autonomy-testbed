#include "fsm.h"

#include <Arduino.h>
#include <time.h>

#include "config.h"
#include "pump.h"
#include "net_mqtt.h"
#include "buzzer.h"

#include <Preferences.h>

enum State {
    ST_MONITORING,
    ST_WATERING,
    ST_MANUAL,
    ST_RESERVOIR_EMPTY,
    ST_DAILY_LIMIT,
    ST_LEAK_FAULT,
    ST_STOPPED,
    ST_WATERING_FAULT,
    ST_MAINTENANCE,
};

static State state = ST_MONITORING;

// Maintenance mode persists across reboots so an intentional watering pause
// (e.g. a dry-down for pest control) survives a power blip (DL-089).
static Preferences prefs;
// Cached maintenance flag, updated ONLY here so the runtime view and the NVS
// value can never drift (the divergence fixed in DL-113).
static bool s_maint = false;
static void maint_persist(bool on) {
    s_maint = on;
    prefs.begin("plant", false);
    prefs.putBool("maint", on);
    prefs.end();
}
static bool maint_load() {
    prefs.begin("plant", true);
    const bool m = prefs.getBool("maint", false);
    prefs.end();
    return m;
}

// Pending remote maintenance request (set by fsm_request_maintenance from the
// MQTT callback, consumed once per fsm_tick). volatile: set outside the tick.
static volatile bool maint_req_pending = false;
static volatile bool maint_req_on = false;
void fsm_request_maintenance(bool on) {
    maint_req_on = on;
    maint_req_pending = true;
}

// Timing / accounting
static unsigned long last_tick_ms          = 0;
static unsigned long daily_window_start_ms = 0;
static unsigned long daily_pump_ms         = 0;   // cumulative pump-on time this window
static bool          time_synced           = false;  // NTP time available yet? (DL-058)
static int           last_reset_yday       = -1;     // local day-of-year of the last daily reset
static unsigned long leak_since_ms         = 0;   // when leak.detected first went true
static unsigned long state_pub_next_ms     = 0;

// LED blink phase
static bool          blink_on      = false;
static unsigned long blink_last_ms = 0;

// Watering pulse machine: alternate pump-on (pulse) and pump-off (settle).
static bool          pulse_phase_on = false;
static unsigned long phase_start_ms = 0;

// Watering-effectiveness watchdog (DL-053): fault if the soil isn't responding.
static uint16_t      watchdog_ref_raw   = 0;
static uint8_t       no_progress_streak = 0;

// Debounced, edge-detected buttons (active LOW with INPUT_PULLUP).
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
static Button btn_stop{0, HIGH, HIGH, 0, false};
static Button btn_ack{0, HIGH, HIGH, 0, false};
static Button btn_manual{0, HIGH, HIGH, 0, false};

static const char* state_name(State s) {
    switch (s) {
        case ST_MONITORING:      return "monitoring";
        case ST_WATERING:        return "watering";
        case ST_MANUAL:          return "manual";
        case ST_RESERVOIR_EMPTY: return "reservoir_empty";
        case ST_DAILY_LIMIT:     return "daily_limit";
        case ST_LEAK_FAULT:      return "leak_fault";
        case ST_STOPPED:         return "stopped";
        case ST_WATERING_FAULT:  return "watering_fault";
        case ST_MAINTENANCE:     return "maintenance";
    }
    return "unknown";
}

static void button_init(Button& b, uint8_t pin) {
    b.pin = pin;
    b.last_raw = HIGH;
    b.stable = HIGH;
    b.change_ms = 0;
    b.pressed_edge = false;
    b.released_edge = false;
    b.long_edge = false;
    b.press_ms = 0;
    b.long_fired = false;
    pinMode(pin, INPUT_PULLUP);
}

static void button_update(Button& b, unsigned long now) {
    b.pressed_edge = false;
    b.released_edge = false;
    b.long_edge = false;
    const int raw = digitalRead(b.pin);
    if (raw != b.last_raw) {
        b.last_raw = raw;
        b.change_ms = now;
    }
    if (now - b.change_ms >= BUTTON_DEBOUNCE_MS && raw != b.stable) {
        b.stable = raw;
        if (b.stable == LOW) {           // press pulls to GND
            b.pressed_edge = true;
            b.press_ms = now;
            b.long_fired = false;
        } else {
            b.released_edge = true;
        }
    }
    // Long-press: held continuously past the threshold, fires once per hold.
    if (b.stable == LOW && !b.long_fired &&
        now - b.press_ms >= BTN_LONGPRESS_MS) {
        b.long_edge = true;
        b.long_fired = true;
    }
}

static void enter_watering(State s, unsigned long now, uint16_t ref_raw) {
    state = s;
    pump_on();
    pulse_phase_on = true;
    phase_start_ms = now;
    watchdog_ref_raw = ref_raw;
    no_progress_streak = 0;
}

static bool run_pulse(unsigned long now) {
    if (pulse_phase_on) {
        if (now - phase_start_ms >= WATER_PULSE_MS) {
            pump_off();
            pulse_phase_on = false;
            phase_start_ms = now;
        }
    } else {
        if (now - phase_start_ms >= WATER_SETTLE_MS) {
            pump_on();
            pulse_phase_on = true;
            phase_start_ms = now;
            return true;   // a full pulse+settle cycle just completed
        }
    }
    return false;
}

static void drive_leds() {
    bool g = false, y = false, r = false, blink = false;
    switch (state) {
        case ST_MONITORING:               g = true; break;
        case ST_WATERING: case ST_MANUAL: g = true; blink = true; break;
        case ST_RESERVOIR_EMPTY:          y = true; break;
        case ST_DAILY_LIMIT:              y = true; blink = true; break;
        case ST_LEAK_FAULT:               r = true; break;
        case ST_STOPPED:                  r = true; blink = true; break;
        case ST_WATERING_FAULT:           r = true; y = true; break;
        case ST_MAINTENANCE:              g = true; y = true; break;
    }
    if (blink && !blink_on) { g = false; y = false; r = false; }
    digitalWrite(LED_GREEN,  g ? HIGH : LOW);
    digitalWrite(LED_YELLOW, y ? HIGH : LOW);
    digitalWrite(LED_RED,    r ? HIGH : LOW);
}

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

    last_tick_ms = millis();
    daily_window_start_ms = millis();
    s_maint = maint_load();
    if (s_maint) {
        state = ST_MAINTENANCE;
        Serial.println("[FSM] init -> maintenance (restored from NVS)");
    } else {
        state = ST_MONITORING;
        Serial.println("[FSM] init -> monitoring");
    }
}

void fsm_tick(const SoilReading& soil, const FloatReading& flt, const LeakReading& leak) {
    const unsigned long now = millis();

    // LED blink phase
    if (now - blink_last_ms >= LED_BLINK_MS) {
        blink_on = !blink_on;
        blink_last_ms = now;
    }

    // Buttons
    button_update(btn_stop, now);
    button_update(btn_ack, now);
    button_update(btn_manual, now);

    // Daily reset: at local calendar midnight once NTP time is available,
    // otherwise a rolling 24h window from boot as a fallback (DL-058).
    struct tm tinfo;
    if (getLocalTime(&tinfo, 0) && tinfo.tm_year >= 120) {  // year >= 2020 => real time
        if (!time_synced) {
            time_synced = true;
            last_reset_yday = tinfo.tm_yday;   // adopt today; no reset on first sync
            Serial.printf("[FSM] NTP time synced: %04d-%02d-%02d %02d:%02d local\n",
                          tinfo.tm_year + 1900, tinfo.tm_mon + 1, tinfo.tm_mday,
                          tinfo.tm_hour, tinfo.tm_min);
        } else if (tinfo.tm_yday != last_reset_yday) {
            daily_pump_ms = 0;                 // crossed local midnight
            last_reset_yday = tinfo.tm_yday;
            Serial.println("[FSM] daily water budget reset (local midnight)");
        }
    } else if (!time_synced && now - daily_window_start_ms >= DAILY_WINDOW_MS) {
        daily_pump_ms = 0;
        daily_window_start_ms = now;
    }

    // Pump-on time accounting (cumulative for the current day/window)
    if (pump_is_on()) {
        daily_pump_ms += (now - last_tick_ms);
    }
    last_tick_ms = now;

    // Leak debounce: must persist LEAK_DEBOUNCE_MS before it counts as a fault.
    if (leak.detected) {
        if (leak_since_ms == 0) leak_since_ms = now;
    } else {
        leak_since_ms = 0;
    }
    const bool leak_confirmed =
        (leak_since_ms != 0 && now - leak_since_ms >= LEAK_DEBOUNCE_MS);

    const State prev = state;

    // Consume any pending remote maintenance command once per tick so the
    // transition runs through the safety-ordered chain below, exactly like the
    // button. Cleared unconditionally: a request that does not apply is dropped.
    const bool rem_pending = maint_req_pending;
    const bool rem_on = maint_req_on;
    maint_req_pending = false;

    // ---- Safety first: evaluated every tick, overrides all states ----
    if (leak_confirmed) {
        pump_off();
        state = ST_LEAK_FAULT;
    } else if (btn_stop.pressed_edge) {
        pump_off();
        state = ST_STOPPED;
    } else if (state == ST_LEAK_FAULT) {
        // Latched: clears only on ACK, and only once the leak has cleared. If the
        // fault interrupted a maintenance pause, return to it (DL-113).
        if (btn_ack.pressed_edge && !leak.detected)
            state = s_maint ? ST_MAINTENANCE : ST_MONITORING;
    } else if (state == ST_STOPPED) {
        // Latched: clears only on ACK.
        if (btn_ack.pressed_edge) state = s_maint ? ST_MAINTENANCE : ST_MONITORING;
    } else if (state == ST_WATERING_FAULT) {
        // Latched: soil never responded to watering. Clear on ACK once fixed.
        if (btn_ack.pressed_edge) state = s_maint ? ST_MAINTENANCE : ST_MONITORING;
    } else if (state == ST_MAINTENANCE) {
        // Intentional pause (DL-089): watering disabled; a long-press of MANUAL
        // resumes. Not a fault -- safety (leak/stop) is still checked above.
        pump_off();
        if (btn_manual.long_edge || (rem_pending && !rem_on)) {
            state = ST_MONITORING;
            maint_persist(false);
            Serial.println(btn_manual.long_edge
                ? "[FSM] maintenance -> monitoring (resumed)"
                : "[FSM] maintenance -> monitoring (resumed via remote)");
        }
    } else if (btn_manual.long_edge || (rem_pending && rem_on)) {
        // Enter the intentional pause from any normal/recoverable state.
        pump_off();
        state = ST_MAINTENANCE;
        maint_persist(true);
        Serial.println(btn_manual.long_edge
            ? "[FSM] -> maintenance (watering paused)"
            : "[FSM] -> maintenance (watering paused via remote)");
    } else {
        // ---- Not in fault: recoverable gating, then normal operation ----
        if (flt.reservoir_empty) {
            pump_off();
            state = ST_RESERVOIR_EMPTY;
        } else if ((daily_pump_ms / 1000.0f) * PUMP_ML_PER_SEC >= MAX_DAILY_PUMP_ML) {
            pump_off();
            state = ST_DAILY_LIMIT;
        } else {
            if (state == ST_RESERVOIR_EMPTY || state == ST_DAILY_LIMIT) {
                state = ST_MONITORING;  // block cleared
            }
            if (state == ST_MONITORING) {
                if (btn_manual.released_edge && !btn_manual.long_fired) {
                    enter_watering(ST_MANUAL, now, soil.valid ? soil.raw : ADC_MAX);
                } else if (soil.valid && soil.raw >= SOIL_THRESHOLD_TRIGGER) {
                    enter_watering(ST_WATERING, now, soil.raw);
                }
            } else if (state == ST_WATERING || state == ST_MANUAL) {
                if (soil.valid && soil.raw <= SOIL_THRESHOLD_STOP) {
                    pump_off();
                    state = ST_MONITORING;  // re-wetted to target
                } else if (run_pulse(now) && soil.valid) {
                    // One pulse+settle cycle elapsed: did the soil respond?
                    if ((int)soil.raw <= (int)watchdog_ref_raw - WATER_RESPONSE_MARGIN) {
                        watchdog_ref_raw = soil.raw;     // progress; reset the streak
                        no_progress_streak = 0;
                    } else if (++no_progress_streak >= WATER_WATCHDOG_PULSES) {
                        pump_off();
                        state = ST_WATERING_FAULT;       // acting, but soil not responding
                    }
                }
            }
        }
    }

    drive_leds();
    buzzer_update(state == ST_LEAK_FAULT, now);

    // Publish state on change, and refresh periodically (retained).
    const bool changed = (state != prev);
    if (changed) {
        Serial.print("[FSM] ");
        Serial.print(state_name(prev));
        Serial.print(" -> ");
        Serial.println(state_name(state));
    }
    if (changed || now >= state_pub_next_ms) {
        mqtt_publish_state(state_name(state), pump_is_on(), daily_pump_ms);
        state_pub_next_ms = now + MQTT_PUBLISH_INTERVAL_MS;
    }
}

const char* fsm_state_name() {
    return state_name(state);
}

unsigned long fsm_daily_pump_ms() {
    return daily_pump_ms;
}
