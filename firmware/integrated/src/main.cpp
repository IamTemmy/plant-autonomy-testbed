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
#include "soil.h"
#include "float_switch.h"
#include "leak.h"
#include "fsm.h"
#include "pump.h"
#include "oled.h"

// Per-task "next due" timestamps. One per scheduled task.
static unsigned long heartbeat_next_ms      = 0;
static unsigned long oled_next_ms           = 0;
static unsigned long sensor_read_next_ms    = 0;
static unsigned long sensor_publish_next_ms = 0;

static unsigned long heartbeat_count = 0;

// Most recent reading, cached at the read cadence and published at the
// (slower) telemetry cadence. valid=false until the first good read.
static Bme280Reading last_air{NAN, NAN, NAN, false};
static Bh1750Reading last_light{NAN, false};
static SoilReading   last_soil{0, NAN, false};
static FloatReading  last_float{false, false};
static LeakReading   last_leak{0, false, false};

void setup() {
    Serial.begin(115200);
    delay(500);  // Boot-time only; principle 1 forbids delay() in the main loop.

    Serial.println();
    Serial.println("=================================================");
    Serial.println("Plant Autonomy Testbed - Phase 2 firmware");
    Serial.println("Build: WiFi + MQTT + sensors BME280/BH1750/soil/float/leak (DL-040)");
    Serial.println("=================================================");
    Serial.println();

    wifi_begin();     // bounded blocking connect at boot, then non-fatal
    mqtt_begin();     // configure broker; connects once WiFi is up (in loop)
    bme280_begin();   // I2C bus + air sensor init
    bh1750_begin();   // light sensor on the same bus
    soil_begin();     // capacitive soil moisture (analog)
    float_switch_begin();  // reservoir float (digital)
    leak_begin();          // leak detection (analog)
    fsm_begin();           // state machine: LEDs, buttons, stubbed pump, buzzer
    oled_begin();          // status display (shared I2C bus; non-fatal if absent)
}

void loop() {
    const unsigned long now_ms = millis();

    wifi_tick();  // non-blocking: services WiFi reconnects on its cadence
    mqtt_tick();  // non-blocking: connects/reconnects + pumps the client

    // State machine runs every loop for responsive buttons/LEDs, using the
    // latest cached sensor readings (updated by the sensor-read task below).
    fsm_tick(last_soil, last_float, last_leak);

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

        last_soil = soil_read();
        if (last_soil.valid) {
            Serial.print("Soil: raw ");
            Serial.print(last_soil.raw);
            Serial.print(", ");
            Serial.print(last_soil.moisture_pct, 1); Serial.println(" %");
        } else {
            Serial.println("Soil: invalid reading (probe disconnected?)");
        }

        last_float = float_switch_read();
        Serial.print("Float: reservoir ");
        Serial.println(last_float.reservoir_empty ? "EMPTY" : "ok");

        last_leak = leak_read();
        Serial.print("Leak: raw ");
        Serial.print(last_leak.raw);
        Serial.println(last_leak.detected ? "  LEAK DETECTED" : "  (dry)");

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
        if (last_soil.valid) {
            mqtt_publish_soil(last_soil.raw, last_soil.moisture_pct);
        }
        if (last_float.valid) {
            mqtt_publish_float(last_float.reservoir_empty);
        }
        if (last_leak.valid) {
            mqtt_publish_leak(last_leak.raw, last_leak.detected);
        }
        sensor_publish_next_ms = now_ms + MQTT_PUBLISH_INTERVAL_MS;
    }

    // OLED status display task: shows current state + readings.
    if (now_ms >= oled_next_ms) {
        oled_render(fsm_state_name(), last_air, last_soil, last_float, last_leak,
                    pump_is_on(), fsm_daily_pump_ms(),
                    (unsigned long)(MAX_DAILY_PUMP_ML / PUMP_ML_PER_SEC * 1000.0f));
        oled_next_ms = now_ms + OLED_REFRESH_MS;
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
