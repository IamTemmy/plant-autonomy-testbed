// Plant Autonomy Testbed — Phase 2 integrated firmware
// Entry point: scheduler + heartbeat, WiFi, MQTT telemetry, sensor reads.
// Reads run at SENSOR_READ_INTERVAL_MS; telemetry publishes at the slower
// MQTT_PUBLISH_INTERVAL_MS to keep the database lean. See DL-040 P1, 3 & 5.

#include <Arduino.h>

#include "config.h"
#include "net_wifi.h"
#include "net_mqtt.h"
#include "bme280.h"
#include "light_sensor.h"

// Per-task "next due" timestamps. One per scheduled task.
static unsigned long heartbeat_next_ms      = 0;
static unsigned long sensor_read_next_ms    = 0;
static unsigned long sensor_publish_next_ms = 0;

static unsigned long heartbeat_count = 0;

// Most recent reading, cached at the read cadence and published at the
// (slower) telemetry cadence. valid=false until the first good read.
static Bme280Reading last_air{NAN, NAN, NAN, false};
static Bh1750Reading last_light{NAN, false};

void setup() {
    Serial.begin(115200);
    delay(500);  // Boot-time only; principle 1 forbids delay() in the main loop.

    Serial.println();
    Serial.println("=================================================");
    Serial.println("Plant Autonomy Testbed - Phase 2 firmware");
    Serial.println("Build: WiFi + MQTT + BME280 telemetry (per DL-040)");
    Serial.println("=================================================");
    Serial.println();

    wifi_begin();     // bounded blocking connect at boot, then non-fatal
    mqtt_begin();     // configure broker; connects once WiFi is up (in loop)
    bme280_begin();   // I2C bus + air sensor init
    bh1750_begin();   // light sensor on the same bus
}

void loop() {
    const unsigned long now_ms = millis();

    wifi_tick();  // non-blocking: services WiFi reconnects on its cadence
    mqtt_tick();  // non-blocking: connects/reconnects + pumps the client

    // Sensor-read task: sample and cache. Prints for bench visibility.
    if (now_ms >= sensor_read_next_ms) {
        last_air = bme280_read();
        if (last_air.valid) {
            Serial.print("BME280: ");
            Serial.print(last_air.temperature_c, 1); Serial.print(" C, ");
            Serial.print(last_air.humidity_pct, 1);  Serial.print(" %RH, ");
            Serial.print(last_air.pressure_hpa, 1);  Serial.println(" hPa");
        } else {
            Serial.println("BME280: invalid reading (sensor fault or out of bounds)");
        }

        last_light = bh1750_read();
        if (last_light.valid) {
            Serial.print("BH1750: ");
            Serial.print(last_light.lux, 1); Serial.println(" lux");
        } else {
            Serial.println("BH1750: invalid reading (sensor fault or out of bounds)");
        }
        sensor_read_next_ms = now_ms + SENSOR_READ_INTERVAL_MS;
    }

    // Telemetry-publish task: publish the latest valid reading. Withholds
    // invalid readings entirely rather than sending garbage (Principle 3/5).
    if (now_ms >= sensor_publish_next_ms) {
        if (last_air.valid) {
            mqtt_publish_bme280(last_air.temperature_c,
                                last_air.humidity_pct,
                                last_air.pressure_hpa);
        }
        if (last_light.valid) {
            mqtt_publish_bh1750(last_light.lux);
        }
        sensor_publish_next_ms = now_ms + MQTT_PUBLISH_INTERVAL_MS;
    }

    // Heartbeat task: presence + liveness.
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
