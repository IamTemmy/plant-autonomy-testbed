#include "net_wifi.h"

#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "secrets.h"

static unsigned long wifi_next_check_ms = 0;

bool wifi_connected() {
    return WiFi.status() == WL_CONNECTED;
}

void wifi_begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("WiFi: connecting to ");
    Serial.print(WIFI_SSID);

    const unsigned long start_ms = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - start_ms < WIFI_CONNECT_TIMEOUT_MS) {
        delay(250);   // boot-time only; loop() is never blocked.
        Serial.print(".");
    }
    Serial.println();

    if (wifi_connected()) {
        Serial.print("WiFi: connected, IP ");
        Serial.print(WiFi.localIP());
        Serial.print("  RSSI ");
        Serial.println(WiFi.RSSI());
    } else {
        Serial.println("WiFi: not connected at boot - continuing, will retry.");
    }
}

void wifi_tick() {
    const unsigned long now_ms = millis();
    if (now_ms < wifi_next_check_ms) {
        return;
    }
    wifi_next_check_ms = now_ms + WIFI_RECONNECT_INTERVAL_MS;

    if (wifi_connected()) {
        return;
    }
    Serial.println("WiFi: link down, attempting reconnect...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}
