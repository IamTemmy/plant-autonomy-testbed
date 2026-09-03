#include "camera.h"

#include <Arduino.h>

// ---- XIAO ESP32-S3 Sense camera pins (CAMERA_MODEL_XIAO_ESP32S3) ----
#define PWDN_GPIO_NUM   -1
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM   10
#define SIOD_GPIO_NUM   40
#define SIOC_GPIO_NUM   39
#define Y9_GPIO_NUM     48
#define Y8_GPIO_NUM     11
#define Y7_GPIO_NUM     12
#define Y6_GPIO_NUM     14
#define Y5_GPIO_NUM     16
#define Y4_GPIO_NUM     18
#define Y3_GPIO_NUM     17
#define Y2_GPIO_NUM     15
#define VSYNC_GPIO_NUM  38
#define HREF_GPIO_NUM   47
#define PCLK_GPIO_NUM   13

bool camera_begin() {
    camera_config_t config = {};
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM; config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM; config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM; config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size   = FRAMESIZE_UXGA;   // 1600x1200 (deployment, DL-080)
    config.jpeg_quality = 10;                // lower number = less compression
    config.fb_count     = psramFound() ? 2 : 1;
    config.grab_mode    = CAMERA_GRAB_LATEST;   // DL-205: return the newest frame, drop stale ones (GRAB_WHEN_EMPTY handed back a buffered pre-light-off frame)
    config.fb_location  = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed: 0x%x\n", err);
        return false;
    }

    for (int i = 0; i < 4; i++) {            // warm up auto-exposure
        camera_fb_t* fb = esp_camera_fb_get();
        if (fb) esp_camera_fb_return(fb);
        delay(100);
    }
    return true;
}

camera_fb_t* camera_capture() {
    // DL-205: return a frame that reflects the scene RIGHT NOW, not a stale one.
    // The driver keeps up to fb_count frames queued; a plain fb_get() could hand
    // back a frame captured before the grow-light was switched off (the "dark"
    // frames were coming back lit). Drain the queued buffers first -- with a short
    // delay between grabs so auto-exposure re-adapts to the darker scene -- then
    // grab the fresh frame we actually return. GRAB_LATEST also biases fb_get()
    // toward the newest frame; the explicit flush makes it deterministic.
    const int flush = psramFound() ? 2 : 1;   // == fb_count
    for (int i = 0; i < flush; i++) {
        camera_fb_t* stale = esp_camera_fb_get();
        if (stale) esp_camera_fb_return(stale);
        delay(120);   // let AE/AGC settle to the new lighting between frames
    }
    return esp_camera_fb_get();
}
