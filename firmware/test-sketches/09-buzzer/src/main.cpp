/*
 * Plant Autonomy Testbed — Phase 1 component test
 * Component under test: Active buzzer (audible alarm)
 * Components reused: 2 LEDs, 3 buttons (validated in 07-feedback-io and 08-buttons)
 *
 * Wiring:
 *   Green LED: GPIO18 -> anode; cathode -> 270 ohm -> GND
 *   Red LED:   GPIO19 -> anode; cathode -> 270 ohm -> GND
 *   Button A (STOP):   GPIO32, INPUT_PULLUP, other leg to GND
 *   Button B (ACK):    GPIO33, INPUT_PULLUP, other leg to GND
 *   Button C (MANUAL): GPIO26, INPUT_PULLUP, other leg to GND
 *   Buzzer driver:
 *     GPIO4 -> 1k ohm -> NPN base
 *     NPN emitter -> GND
 *     NPN collector -> buzzer (-)
 *     Buzzer (+) -> 5V rail
 *
 * State machine:
 *   IDLE      green steady, buzzer silent
 *   ACTION    green fast blink for 3 seconds -> IDLE, buzzer silent
 *   FAULT     red steady, buzzer silent
 *   CRITICAL  red fast blink, buzzer ON
 *
 * Button behavior:
 *   Button A (STOP):   IDLE/ACTION -> FAULT; FAULT -> CRITICAL (escalates)
 *   Button B (ACK):    FAULT or CRITICAL -> IDLE (silences buzzer)
 *   Button C (MANUAL): IDLE -> ACTION
 *
 * The STOP-from-FAULT escalation simulates "a fault that was not
 * acknowledged in time has become critical and now demands attention."
 * It also gives a single button (STOP) double duty for reaching CRITICAL
 * from any healthy state via two presses.
 */

#include <Arduino.h>

static const uint8_t  LED_GREEN  = 18;
static const uint8_t  LED_RED    = 19;
static const uint8_t  BTN_STOP   = 32;
static const uint8_t  BTN_ACK    = 33;
static const uint8_t  BTN_MANUAL = 26;
static const uint8_t  BUZZER     = 4;

enum State { IDLE, ACTION, FAULT, CRITICAL };
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
  digitalWrite(BUZZER, LOW);
  Serial.print(F("[STATE] -> "));
  Serial.println(name);
}

void renderState() {
  uint32_t now = millis();
  switch (state) {
    case IDLE:
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
      digitalWrite(BUZZER, LOW);
      break;
    case ACTION:
      if (now - lastBlinkMs >= 100) {
        blinkOn = !blinkOn;
        digitalWrite(LED_GREEN, blinkOn ? HIGH : LOW);
        lastBlinkMs = now;
      }
      digitalWrite(LED_RED, LOW);
      digitalWrite(BUZZER, LOW);
      if (now - stateEnteredMs >= 3000) {
        setState(IDLE, "IDLE (action complete)");
      }
      break;
    case FAULT:
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, HIGH);
      digitalWrite(BUZZER, LOW);
      break;
    case CRITICAL:
      digitalWrite(LED_GREEN, LOW);
      if (now - lastBlinkMs >= 100) {
        blinkOn = !blinkOn;
        digitalWrite(LED_RED, blinkOn ? HIGH : LOW);
        lastBlinkMs = now;
      }
      digitalWrite(BUZZER, HIGH);
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

void onStop() {
  if (state == FAULT)             setState(CRITICAL, "CRITICAL (escalated)");
  else if (state != CRITICAL)     setState(FAULT,    "FAULT (manual stop)");
}
void onAck() {
  if (state == FAULT || state == CRITICAL) setState(IDLE, "IDLE (acknowledged)");
}
void onManual() {
  if (state == IDLE) setState(ACTION, "ACTION (manual water)");
}

void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BTN_STOP,   INPUT_PULLUP);
  pinMode(BTN_ACK,    INPUT_PULLUP);
  pinMode(BTN_MANUAL, INPUT_PULLUP);

  digitalWrite(BUZZER, LOW);  // explicit off at boot

  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("Plant Autonomy Testbed - Buzzer + state machine test"));
  Serial.println(F("===================================================="));
  Serial.println(F("Button A (STOP):   IDLE/ACTION -> FAULT;  FAULT -> CRITICAL"));
  Serial.println(F("Button B (ACK):    FAULT or CRITICAL -> IDLE (silences buzzer)"));
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
