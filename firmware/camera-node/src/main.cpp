/*
 * Plant Autonomy Testbed — camera node (XIAO ESP32-S3 Sense).
 *
 * Vision-node firmware v1 (DL-078): on a timer, capture a JPEG and HTTP POST it
 * to the Pi image receiver (hub/09-camera), which stores it and computes an
 * Excess-Green "greenness" value. Image bytes go over HTTP; the MQTT capture
 * event / presence is a later slice. WiFi loss is non-fatal — captures are
 * skipped while down and resume on reconnect.
 *
 * Capture cadence is the deployment value in config.h (hourly); photoperiod
 * gating (capture only during the lit window) is enforced Pi-side (DL-082).
 */
#include <Arduino.h>

#include "config.h"
#include "net_wifi.h"
#include "camera.h"
#include "poster.h"

static unsigned long next_capture_ms = 0;

void setup() {
    Serial.begin(115200);
    unsigned long t0 = millis();
    while (!Serial && millis() - t0 < 3000) { delay(10); }
    delay(300);
    Serial.println();
    Serial.println("Camera node starting");

    if (!camera_begin()) {
        Serial.println("FATAL: camera init failed — check daughter-board seating.");
        while (true) { delay(1000); }
    }
    Serial.println("Camera ready");

    wifi_begin();
}

void loop() {
    wifi_tick();

    const unsigned long now = millis();
    if (now < next_capture_ms) {
        delay(20);
        return;
    }
    next_capture_ms = now + CAPTURE_INTERVAL_MS;

    if (!wifi_connected()) {
        Serial.println("skip capture: WiFi down");
        return;
    }

    camera_fb_t* fb = camera_capture();
    if (!fb) {
        Serial.println("capture failed (NULL framebuffer)");
        return;
    }
    Serial.printf("captured %ux%u  %u bytes -> POST\n",
                  fb->width, fb->height, (unsigned)fb->len);
    poster_post_jpeg(fb->buf, fb->len);
    esp_camera_fb_return(fb);
}
