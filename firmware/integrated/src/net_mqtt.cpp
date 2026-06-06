#include "net_mqtt.h"

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "config.h"
#include "secrets.h"
#include "net_wifi.h"

static WiFiClient   wifi_client;
static PubSubClient mqtt(wifi_client);

// Next time mqtt_tick() should attempt a reconnect.
static unsigned long mqtt_next_attempt_ms = 0;

// Fixed Last-Will payload: the broker publishes this (retained) on
// plant/status/wrover if the WROVER drops uncleanly, so the dashboard
// reflects the device as offline without us having to send anything.
static const char MQTT_WILL_PAYLOAD[] = "{\"online\":false}";

bool mqtt_connected() {
    return mqtt.connected();
}

void mqtt_begin() {
    mqtt.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    // No connect here; WiFi may not be up yet. mqtt_tick() handles it.
}

void mqtt_publish_status(unsigned long heartbeat) {
    if (!mqtt.connected()) {
        return;
    }
    char payload[96];
    snprintf(payload, sizeof(payload),
             "{\"online\":true,\"uptime_s\":%lu,\"rssi\":%d,\"heartbeat\":%lu}",
             millis() / 1000UL, WiFi.RSSI(), heartbeat);
    // retained = true: late subscribers (and the dashboard) get last-known.
    mqtt.publish(MQTT_TOPIC_STATUS, payload, true);
}

void mqtt_publish_bme280(float temperature_c, float humidity_pct, float pressure_hpa) {
    if (!mqtt.connected()) {
        return;
    }
    char payload[128];
    snprintf(payload, sizeof(payload),
             "{\"temperature_c\":%.2f,\"humidity_pct\":%.2f,\"pressure_hpa\":%.2f}",
             temperature_c, humidity_pct, pressure_hpa);
    mqtt.publish(MQTT_TOPIC_SENSORS_BME280, payload);  // not retained
}

void mqtt_publish_bh1750(float lux) {
    if (!mqtt.connected()) {
        return;
    }
    char payload[64];
    snprintf(payload, sizeof(payload), "{\"lux\":%.1f}", lux);
    mqtt.publish(MQTT_TOPIC_SENSORS_BH1750, payload);  // not retained
}

void mqtt_publish_soil(uint16_t raw, float moisture_pct) {
    if (!mqtt.connected()) {
        return;
    }
    char payload[64];
    snprintf(payload, sizeof(payload),
             "{\"soil_raw\":%u,\"moisture_pct\":%.1f}", raw, moisture_pct);
    mqtt.publish(MQTT_TOPIC_SENSORS_SOIL, payload);  // not retained
}

void mqtt_publish_float(bool reservoir_empty) {
    if (!mqtt.connected()) {
        return;
    }
    char payload[40];
    snprintf(payload, sizeof(payload),
             "{\"reservoir_empty\":%d}", reservoir_empty ? 1 : 0);
    mqtt.publish(MQTT_TOPIC_SENSORS_FLOAT, payload);  // not retained
}

void mqtt_publish_leak(uint16_t raw, bool detected) {
    if (!mqtt.connected()) {
        return;
    }
    char payload[64];
    snprintf(payload, sizeof(payload),
             "{\"leak_raw\":%u,\"leak_detected\":%d}", raw, detected ? 1 : 0);
    mqtt.publish(MQTT_TOPIC_SENSORS_LEAK, payload);  // not retained
}

void mqtt_publish_state(const char* state, bool pump_on, unsigned long daily_pump_ms) {
    if (!mqtt.connected()) {
        return;
    }
    char payload[96];
    snprintf(payload, sizeof(payload),
             "{\"state\":\"%s\",\"pump\":%d,\"daily_pump_ms\":%lu}",
             state, pump_on ? 1 : 0, daily_pump_ms);
    mqtt.publish(MQTT_TOPIC_STATE, payload, true);  // retained
}

void mqtt_tick() {
    // MQTT depends on WiFi; nothing to do until the link is up.
    if (!wifi_connected()) {
        return;
    }

    if (mqtt.connected()) {
        mqtt.loop();  // service keepalive + inbound; must be called regularly.
        return;
    }

    // Disconnected: attempt reconnect on a fixed cadence (non-blocking).
    const unsigned long now_ms = millis();
    if (now_ms < mqtt_next_attempt_ms) {
        return;
    }
    mqtt_next_attempt_ms = now_ms + MQTT_RECONNECT_INTERVAL_MS;

    Serial.print("MQTT: connecting to ");
    Serial.print(MQTT_BROKER_HOST);
    Serial.print(":");
    Serial.print(MQTT_BROKER_PORT);
    Serial.print(" ... ");

    // connect(id, user, pass, willTopic, willQos, willRetain, willMessage)
    const bool ok = mqtt.connect(MQTT_CLIENT_ID,
                                 MQTT_USER, MQTT_PASSWORD,
                                 MQTT_TOPIC_STATUS, 0, true, MQTT_WILL_PAYLOAD);
    if (ok) {
        Serial.println("connected.");
        mqtt_publish_status(0);  // announce online immediately (retained)
    } else {
        Serial.print("failed, rc=");
        Serial.println(mqtt.state());
        // Non-fatal (DL-040 Principle 5). Retried on the next cadence.
    }
}
