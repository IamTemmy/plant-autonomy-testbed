// Plant Autonomy Testbed — Phase 2 integrated firmware
// Heartbeat commit: non-blocking periodic task using millis() scheduling.
// Establishes the scheduling pattern that all subsequent periodic tasks
// (sensor reads, MQTT publish, LED blink, FSM tick) will follow.
// See DL-040 Principle 1.

#include <Arduino.h>

// Scheduling intervals (milliseconds). All periodic-task timing lives here
// until a dedicated config.h is introduced.
static constexpr unsigned long HEARTBEAT_INTERVAL_MS = 5000;

// Per-task "next due" timestamps. One per scheduled task.
// Initialized to 0 so each task runs once shortly after boot.
static unsigned long heartbeat_next_ms = 0;

// Counter for the heartbeat ticks — useful for confirming the timing
// is consistent across boots and for spotting missed ticks.
static unsigned long heartbeat_count = 0;

void setup() {
    Serial.begin(115200);
    delay(500);  // Boot-time only; principle 1 forbids delay() in the main loop.

    Serial.println();
    Serial.println("=================================================");
    Serial.println("Plant Autonomy Testbed - Phase 2 firmware");
    Serial.println("Build: heartbeat + scheduling pattern (per DL-040)");
    Serial.println("=================================================");
    Serial.println();
}

void loop() {
    const unsigned long now_ms = millis();

    // Heartbeat task: prints a tick line every HEARTBEAT_INTERVAL_MS ms.
    // The scheduling pattern below is the template for all later
    // periodic tasks: compare now to next-due, run if due, update next-due.
    if (now_ms >= heartbeat_next_ms) {
        heartbeat_count++;
        Serial.print("heartbeat #");
        Serial.print(heartbeat_count);
        Serial.print(" @ ");
        Serial.print(now_ms);
        Serial.println(" ms");

        heartbeat_next_ms = now_ms + HEARTBEAT_INTERVAL_MS;
    }

    // Loop returns immediately. No delay(). Future periodic tasks
    // (sensor reads, MQTT publish, etc.) will be added as additional
    // `if (now_ms >= xyz_next_ms)` blocks following the same pattern.
}