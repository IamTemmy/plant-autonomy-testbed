/*
 * Plant Autonomy Testbed - Phase 1 component test: Traffic-light status LEDs
 *
 * Wiring (active-HIGH, each via 270 ohm to GND):
 *   Red LED:    GPIO23 -> anode  (top)
 *   Yellow LED: GPIO19 -> anode  (middle)
 *   Green LED:  GPIO18 -> anode  (bottom)
 *
 * Behavior (looping, 115200 baud):
 *   Phase A - identify: each LED on alone, top-to-bottom (red, yellow, green),
 *             with a serial label so the physical position can be confirmed.
 *   Phase B - all three blink together three times.
 *
 * Pass criteria: each named LED lights alone in the right position, and all
 * three blink together. Confirms GPIO23 (new) and the red<->yellow swap.
 */

#include <Arduino.h>

static const uint8_t LED_GREEN  = 18;
static const uint8_t LED_YELLOW = 19;
static const uint8_t LED_RED    = 23;

static void allOff() {
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
}

void setup() {
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    allOff();

    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println("Plant Autonomy Testbed - Traffic-light LED test");
    Serial.println("Order: RED (top) -> YELLOW (mid) -> GREEN (bottom)");
}

void loop() {
    // Phase A: identify each LED in physical top-to-bottom order.
    allOff(); digitalWrite(LED_RED, HIGH);
    Serial.println("RED on (expect TOP)");
    delay(800);

    allOff(); digitalWrite(LED_YELLOW, HIGH);
    Serial.println("YELLOW on (expect MIDDLE)");
    delay(800);

    allOff(); digitalWrite(LED_GREEN, HIGH);
    Serial.println("GREEN on (expect BOTTOM)");
    delay(800);

    // Phase B: all blink together three times.
    allOff();
    Serial.println("ALL blink x3");
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_YELLOW, HIGH);
        digitalWrite(LED_RED, HIGH);
        delay(250);
        allOff();
        delay(250);
    }
    delay(600);
}
