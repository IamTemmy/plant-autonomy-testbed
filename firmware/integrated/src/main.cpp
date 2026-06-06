// Plant Autonomy Testbed — Phase 2 integrated firmware
// Entry point: scheduler + heartbeat, WiFi, and MQTT telemetry.
// All periodic tasks follow the same non-blocking millis() pattern
// established by the heartbeat. See DL-040 Principles 1 & 5.

#include <Arduino.h>

#include "config.h"
#include "net_wifi.h"
#include "net_mqtt.h"

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
    Serial.println("Build: heartbeat + WiFi + MQTT (per DL-040)");
    Serial.println("=================================================");
    Serial.println();

    wifi_begin();  // bounded blocking connect at boot, then non-fatal
    mqtt_begin();  // configure broker; connects once WiFi is up (in loop)
}

void loop() {
    const unsigned long now_ms = millis();

    wifi_tick();  // non-blocking: services WiFi reconnects on its cadence
    mqtt_tick();  // non-blocking: connects/reconnects + pumps the client

    // Heartbeat task: prints a tick line every HEARTBEAT_INTERVAL_MS ms and
    // publishes a retained presence message. The scheduling pattern below is
    // the template for all later periodic tasks.
    if (now_ms >= heartbeat_next_ms) {
        heartbeat_count++;
        Serial.print("heartbeat #");
        Serial.print(heartbeat_count);
        Serial.print(" @ ");
        Serial.print(now_ms);
        Serial.println(" ms");

        mqtt_publish_status(heartbeat_count);  // no-op if not connected

        heartbeat_next_ms = now_ms + HEARTBEAT_INTERVAL_MS;
    }

    // Loop returns immediately. No delay(). Future periodic tasks
    // (sensor reads, MQTT publish, etc.) will be added as additional
    // `if (now_ms >= xyz_next_ms)` blocks following the same pattern.
}
