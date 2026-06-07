#include "fsm.h"

#include <Arduino.h>

#include "config.h"
#include "pump.h"
#include "net_mqtt.h"
#include "buzzer.h"

enum State {
    ST_MONITORING,
    ST_WATERING,
    ST_MANUAL,
    ST_RESERVOIR_EMPTY,
    ST_DAILY_LIMIT,
    ST_LEAK_FAULT,
    ST_STOPPED,
    ST_WATERING_FAULT,
};

static State state = ST_MONITORING;

// Timing / accounting
static unsigned long last_tick_ms          = 0;
static unsigned long daily_window_start_ms = 0;
static unsigned long daily_pump_ms         = 0;   // cumulative pump-on time this window
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
    }
    return "unknown";
}

static void button_init(Button& b, uint8_t pin) {
    b.pin = pin;
    b.last_raw = HIGH;
    b.stable = HIGH;
    b.change_ms = 0;
    b.pressed_edge = false;
    pinMode(pin, INPUT_PULLUP);
}

static void button_update(Button& b, unsigned long now) {
    b.pressed_edge = false;
    const int raw = digitalRead(b.pin);
    if (raw != b.last_raw) {
        b.last_raw = raw;
        b.change_ms = now;
    }
    if (now - b.change_ms >= BUTTON_DEBOUNCE_MS && raw != b.stable) {
        b.stable = raw;
        if (b.stable == LOW) b.pressed_edge = true;  // press pulls to GND
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

    state = ST_MONITORING;
    last_tick_ms = millis();
    daily_window_start_ms = millis();
    Serial.println("[FSM] init -> monitoring");
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

    // Daily window reset + pump-on time accounting
    if (now - daily_window_start_ms >= DAILY_WINDOW_MS) {
        daily_pump_ms = 0;
        daily_window_start_ms = now;
    }
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

    // ---- Safety first: evaluated every tick, overrides all states ----
    if (leak_confirmed) {
        pump_off();
        state = ST_LEAK_FAULT;
    } else if (btn_stop.pressed_edge) {
        pump_off();
        state = ST_STOPPED;
    } else if (state == ST_LEAK_FAULT) {
        // Latched: clears only on ACK, and only once the leak has cleared.
        if (btn_ack.pressed_edge && !leak.detected) state = ST_MONITORING;
    } else if (state == ST_STOPPED) {
        // Latched: clears only on ACK.
        if (btn_ack.pressed_edge) state = ST_MONITORING;
    } else if (state == ST_WATERING_FAULT) {
        // Latched: soil never responded to watering. Clear on ACK once fixed.
        if (btn_ack.pressed_edge) state = ST_MONITORING;
    } else {
        // ---- Not in fault: recoverable gating, then normal operation ----
        if (flt.reservoir_empty) {
            pump_off();
            state = ST_RESERVOIR_EMPTY;
        } else if (daily_pump_ms >= MAX_DAILY_PUMP_MS) {
            pump_off();
            state = ST_DAILY_LIMIT;
        } else {
            if (state == ST_RESERVOIR_EMPTY || state == ST_DAILY_LIMIT) {
                state = ST_MONITORING;  // block cleared
            }
            if (state == ST_MONITORING) {
                if (btn_manual.pressed_edge) {
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
