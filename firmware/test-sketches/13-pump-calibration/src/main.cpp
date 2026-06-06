/*
 * Plant Autonomy Testbed - Phase 1 component test: Pump flow calibration
 *
 * Wiring (matches integrated config.h):
 *   Pump MOSFET gate: GPIO25 (HIGH = pump on), 1N4007 flyback across pump.
 *   PRIME button:     GPIO32, INPUT_PULLUP (hold to jog the pump)
 *   RUN button:       GPIO26, INPUT_PULLUP (one timed dispense)
 *   ABORT button:     GPIO33, INPUT_PULLUP (stop a timed run early)
 *
 * Procedure (115200 baud):
 *   1. Inlet in water, outlet into the measuring cup.
 *   2. Hold PRIME until water flows steadily; release (discard primed water).
 *   3. Empty the cup. Press RUN: pump runs RUN_MS, then stops.
 *   4. Read the mL collected. mL/s = mL / (RUN_MS / 1000).
 *   5. Repeat 3x and average for a stable figure.
 */

#include <Arduino.h>

static const uint8_t  PUMP      = 25;
static const uint8_t  BTN_PRIME = 32;
static const uint8_t  BTN_RUN   = 26;
static const uint8_t  BTN_ABORT = 33;

static const unsigned long RUN_MS = 90000;  // 2 min per press

static bool          timed_running = false;
static unsigned long run_start = 0;
static unsigned long run_end   = 0;

static int last_run   = HIGH;
static int last_abort = HIGH;

static bool edge_low(uint8_t pin, int& last) {
    int now = digitalRead(pin);
    bool e = (now == LOW && last == HIGH);
    last = now;
    return e;
}

void setup() {
    pinMode(PUMP, OUTPUT);
    digitalWrite(PUMP, LOW);
    pinMode(BTN_PRIME, INPUT_PULLUP);
    pinMode(BTN_RUN, INPUT_PULLUP);
    pinMode(BTN_ABORT, INPUT_PULLUP);

    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println("Pump calibration");
    Serial.print("Hold PRIME (GPIO32) to jog. Press RUN (GPIO26) for ");
    Serial.print(RUN_MS / 1000);
    Serial.println("s dispense. ABORT (GPIO33) stops early.");
}

void loop() {
    const unsigned long now = millis();
    const bool run_edge   = edge_low(BTN_RUN, last_run);
    const bool abort_edge = edge_low(BTN_ABORT, last_abort);

    if (!timed_running) {
        // PRIME: pump on while held.
        const bool prime = (digitalRead(BTN_PRIME) == LOW);
        digitalWrite(PUMP, prime ? HIGH : LOW);

        if (run_edge) {
            timed_running = true;
            run_start = now;
            run_end = now + RUN_MS;
            digitalWrite(PUMP, HIGH);
            Serial.print("RUN start - dispensing ");
            Serial.print(RUN_MS / 1000);
            Serial.println("s...");
        }
    } else {
        if (abort_edge) {
            digitalWrite(PUMP, LOW);
            timed_running = false;
            Serial.print("ABORTED after ");
            Serial.print(now - run_start);
            Serial.println(" ms");
        } else if (now >= run_end) {
            digitalWrite(PUMP, LOW);
            timed_running = false;
            Serial.print("RUN done: ");
            Serial.print(RUN_MS);
            Serial.println(" ms. Read the cup; mL/s = mL / (RUN_MS/1000).");
        }
    }

    delay(10);
}
