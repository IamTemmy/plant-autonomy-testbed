/*
 * Plant Autonomy Testbed — Phase 1 component test
 * Components under test: Three pushbuttons
 * Components reused for state visualization: Two LEDs (validated in 07-feedback-io)
 *
 * Wiring:
 *   Green LED: GPIO18 -> anode; cathode -> 270 ohm -> GND
 *   Red LED:   GPIO19 -> anode; cathode -> 270 ohm -> GND
 *   Button A (STOP):   GPIO32, INPUT_PULLUP, other leg to GND
 *   Button B (ACK):    GPIO33, INPUT_PULLUP, other leg to GND
 *   Button C (MANUAL): GPIO26, INPUT_PULLUP, other leg to GND
 *
 * State machine:
 *   IDLE      green steady
 *   ACTION    green fast blink for 3 seconds, then returns to IDLE
 *   FAULT     red steady
 *
 * Button behavior:
 *   Button A (STOP):   any state -> FAULT
 *   Button B (ACK):    FAULT -> IDLE
 *   Button C (MANUAL): IDLE -> ACTION (manual water trigger simulation)
 *
 * Serial prints every state transition and every button press.
 */

#include <Arduino.h>

static const uint8_t  LED_GREEN  = 18;
static const uint8_t  LED_RED    = 19;
static const uint8_t  BTN_STOP   = 32;
static const uint8_t  BTN_ACK    = 33;
static const uint8_t  BTN_MANUAL = 26;

enum State { IDLE, ACTION, FAULT };
State state = IDLE;
uint32_t stateEnteredMs = 0;
uint32_t lastBlinkMs    = 0;
bool blinkOn = false;

int lastA = HIGH, lastB = HIGH, lastC = HIGH;

void setState(State s, const char* name) {
  state = s;
  stateEnteredMs = millis();
  blinkOn = false;
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  Serial.print(F("[STATE] -> "));
  Serial.println(name);
}

void renderState() {
  uint32_t now = millis();
  switch (state) {
    case IDLE:
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
      break;
    case ACTION:
      if (now - lastBlinkMs >= 100) {
        blinkOn = !blinkOn;
        digitalWrite(LED_GREEN, blinkOn ? HIGH : LOW);
        lastBlinkMs = now;
      }
      digitalWrite(LED_RED, LOW);
      if (now - stateEnteredMs >= 3000) {
        setState(IDLE, "IDLE (action complete)");
      }
      break;
    case FAULT:
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, HIGH);
      break;
  }
}

void handleButton(uint8_t pin, int &last, const char* label, void (*action)()) {
  int now = digitalRead(pin);
  if (now == LOW && last == HIGH) {
    Serial.print(F("[BUTTON] "));
    Serial.println(label);
    action();
  }
  last = now;
}

void onStop()   { setState(FAULT, "FAULT (manual stop)"); }
void onAck()    { if (state == FAULT) setState(IDLE, "IDLE (acknowledged)"); }
void onManual() { if (state == IDLE) setState(ACTION, "ACTION (manual water)"); }

void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BTN_STOP,   INPUT_PULLUP);
  pinMode(BTN_ACK,    INPUT_PULLUP);
  pinMode(BTN_MANUAL, INPUT_PULLUP);

  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("Plant Autonomy Testbed - Pushbuttons test"));
  Serial.println(F("========================================="));
  Serial.println(F("Button A (STOP):   any -> FAULT"));
  Serial.println(F("Button B (ACK):    FAULT -> IDLE"));
  Serial.println(F("Button C (MANUAL): IDLE -> ACTION (3 s)"));
  Serial.println();

  setState(IDLE, "IDLE (boot)");
}

void loop() {
  handleButton(BTN_STOP,   lastA, "STOP",   onStop);
  handleButton(BTN_ACK,    lastB, "ACK",    onAck);
  handleButton(BTN_MANUAL, lastC, "MANUAL", onManual);
  renderState();
  delay(10);
}
