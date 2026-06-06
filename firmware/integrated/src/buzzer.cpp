#include "buzzer.h"

#include <Arduino.h>

#include "config.h"

static bool          sounding   = false;
static unsigned long last_ms    = 0;

void buzzer_begin() {
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, LOW);
    sounding = false;
    last_ms = 0;
}

void buzzer_update(bool alarm, unsigned long now) {
    if (!alarm) {
        if (sounding) {
            digitalWrite(BUZZER, LOW);
            sounding = false;
        }
        last_ms = now;
        return;
    }
    // Alarm active: intermittent beep (BUZZER_ON_MS on, BUZZER_OFF_MS off).
    const unsigned long interval = sounding ? BUZZER_ON_MS : BUZZER_OFF_MS;
    if (now - last_ms >= interval) {
        sounding = !sounding;
        digitalWrite(BUZZER, sounding ? HIGH : LOW);
        last_ms = now;
    }
}
