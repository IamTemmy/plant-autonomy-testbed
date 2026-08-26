#include "net_mqtt.h"

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>

#include "config.h"
#include "secrets.h"
#include "net_wifi.h"

static WiFiClient   wifi_client;
static PubSubClient mqtt(wifi_client);

static unsigned long mqtt_next_attempt_ms = 0;

// Retained Last-Will: the broker publishes this on plant/status/camera if the
// node drops uncleanly, so the dashboard sees the camera offline (mirrors WROVER).
static const char MQTT_WILL_PAYLOAD[] = "{\"online\":false}";

// Latched capture request (set in the MQTT callback, consumed by the main loop).
// volatile: written from the PubSubClient callback context.
static volatile bool capture_pending = false;
static char          capture_context[CAPTURE_CONTEXT_MAX] = {0};

// Inbound handler for plant/cmd/capture. The payload is the capture context
// ("dark"|"lit"); an empty payload is treated as an untyped trigger. We only
// latch here -- the blocking capture+POST happens in the main loop.
static void mqtt_on_message(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, MQTT_TOPIC_CMD_CAPTURE) != 0) {
        return;
    }
    // Copy the context, bounded, NUL-terminated.
    unsigned int n = length;
    if (n >= CAPTURE_CONTEXT_MAX) {
        n = CAPTURE_CONTEXT_MAX - 1;
    }
    memcpy(capture_context, payload, n);
    capture_context[n] = '\0';
    capture_pending = true;
    Serial.printf("MQTT: capture requested [%s]\n",
                  capture_context[0] ? capture_context : "-");
}

bool mqtt_connected() {
    return mqtt.connected();
}

void mqtt_begin() {
    mqtt.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    mqtt.setCallback(mqtt_on_message);
    // No connect here; WiFi may not be up yet. mqtt_tick() handles it.
}

void mqtt_publish_status() {
    if (!mqtt.connected()) {
        return;
    }
    char payload[96];
    snprintf(payload, sizeof(payload),
             "{\"online\":true,\"uptime_s\":%lu,\"rssi\":%d}",
             millis() / 1000UL, WiFi.RSSI());
    mqtt.publish(MQTT_TOPIC_STATUS, payload, true);   // retained
}

bool mqtt_take_capture_request(char* out, size_t out_len) {
    if (!capture_pending) {
        return false;
    }
    if (out != nullptr && out_len > 0) {
        strncpy(out, capture_context, out_len - 1);
        out[out_len - 1] = '\0';
    }
    capture_pending = false;
    return true;
}

void mqtt_tick() {
    if (!wifi_connected()) {
        return;
    }

    if (mqtt.connected()) {
        mqtt.loop();  // service keepalive + inbound; must be called regularly.
        return;
    }

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
        mqtt_publish_status();                    // announce online (retained)
        mqtt.subscribe(MQTT_TOPIC_CMD_CAPTURE);   // inbound capture trigger
        Serial.println("MQTT: subscribed to cmd/capture");
    } else {
        Serial.print("failed, rc=");
        Serial.println(mqtt.state());
        // Non-fatal; retried on the next cadence.
    }
}
