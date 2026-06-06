// Plant Autonomy Testbed — Phase 2 integrated firmware
// Skeleton commit: boot banner only. See DL-040 for architectural principles.

#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    delay(500);  // Boot-time only; principle 1 forbids delay() in the main loop.

    Serial.println();
    Serial.println("=================================================");
    Serial.println("Plant Autonomy Testbed - Phase 2 firmware");
    Serial.println("Build: skeleton (per DL-040)");
    Serial.println("=================================================");
    Serial.println();
}

void loop() {
    // Intentionally empty.
    // Subsequent commits add WiFi, sensors, FSM, telemetry, actuation.
}
