// Plant Autonomy Testbed — Phase 2 integrated firmware
// Entry point: scheduler + heartbeat, WiFi, MQTT telemetry, sensor reads.
// All periodic tasks follow the same non-blocking millis() pattern
// established by the heartbeat. See DL-040 Principles 1, 3 & 5.

#include <Arduino.h>

#include "config.h"
#include "net_wifi.h"
#include "net_mqtt.h"
#include "bme280.h"

// Per-task "next due" timestamps. One per scheduled task.
// Initialized to 0 so each task runs once shortly after boot.
static unsigned long heartbeat_next_ms   = 0;
static unsigned long sensor_read_next_ms = 0;

// Counter for the heartbeat ticks — useful for confirming the timing
// is consistent across boots and for spotting missed ticks.
static unsigned long heartbeat_count = 0;

void setup() {
    Serial.begin(115200);
    delay(500);  // Boot-time only; principle 1 forbids delay() in the main loop.

    Serial.println();
    Serial.println("=================================================");
    Serial.println("Plant Autonomy Testbed - Phase 2 firmware");
    Serial.println("Build: heartbeat + WiFi + MQTT + BME280 (per DL-040)");
    Serial.println("=================================================");
    Serial.println();

    wifi_begin();     // bounded blocking connect at boot, then non-fatal
    mqtt_begin();     // configure broker; connects once WiFi is up (in loop)
    bme280_begin();   // I2C bus + air sensor init
}

void loop() {
    const unsigned long now_ms = millis();

    wifi_tick();  // non-blocking: services WiFi reconnects on its cadence
    mqtt_tick();  // non-blocking: connects/reconnects + pumps the client

    // Sensor-read task: sample the air sensor and print. No MQTT publish yet;
    // telemetry envelope + publishing is the next step.
    if (now_ms >= sensor_read_next_ms) {
        const Bme280Reading air = bme280_read();
        if (air.valid) {
            Serial.print("BME280: ");
            Serial.print(air.temperature_c, 1); Serial.print(" C, ");
            Serial.print(air.humidity_pct, 1);  Serial.print(" %RH, ");
            Serial.print(air.pressure_hpa, 1);  Serial.println(" hPa");
        } else {
            Serial.println("BME280: invalid reading (sensor fault or out of bounds)");
        }
        sensor_read_next_ms = now_ms + SENSOR_READ_INTERVAL_MS;
    }

    // Heartbeat task: prints a tick line every HEARTBEAT_INTERVAL_MS ms and
    // publishes a retained presence message.
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

    // Loop returns immediately. No delay().
}
