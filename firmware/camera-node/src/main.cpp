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
#include "net_mqtt.h"
#include "camera.h"
#include "poster.h"

static unsigned long next_fallback_ms = 0;

// Capture one frame and POST it with the given trigger context
// ("dark"|"lit"|"fallback"). Shared by the MQTT trigger and the fallback timer.
static void do_capture(const char* context) {
    if (!wifi_connected()) {
        Serial.println("skip capture: WiFi down");
        return;
    }
    camera_fb_t* fb = camera_capture();
    if (!fb) {
        Serial.println("capture failed (NULL framebuffer)");
        return;
    }
    Serial.printf("captured %ux%u  %u bytes [%s] -> POST\n",
                  fb->width, fb->height, (unsigned)fb->len, context);
    poster_post_jpeg(fb->buf, fb->len, context);
    esp_camera_fb_return(fb);
}

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
    mqtt_begin();   // DL-196: inbound capture trigger; connects once WiFi is up
}

void loop() {
    wifi_tick();
    mqtt_tick();

    // Primary path (DL-196): capture when the Pi orchestrator triggers it. The
    // MQTT callback latches the request + context; we perform the blocking
    // capture+POST here in the loop, never inside the callback.
    char ctx[CAPTURE_CONTEXT_MAX];
    if (mqtt_take_capture_request(ctx, sizeof(ctx))) {
        do_capture(ctx[0] ? ctx : "triggered");
        next_fallback_ms = millis() + CAPTURE_INTERVAL_MS;  // defer the fallback
    }

    // Fallback path: if the orchestrator goes silent, still capture occasionally
    // so the node isn't dark. Tagged "fallback" so the receiver keeps these out
    // of the calibrated metric (they may be lit or dark, uncontrolled).
    const unsigned long now = millis();
    if (now >= next_fallback_ms) {
        next_fallback_ms = now + CAPTURE_INTERVAL_MS;
        do_capture("fallback");
    }

    delay(20);
}
