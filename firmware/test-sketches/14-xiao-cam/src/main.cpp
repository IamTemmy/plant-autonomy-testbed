/*
 * 14 — XIAO ESP32-S3 Sense camera bring-up.
 *
 * Proves the vision node enumerates over USB-C and the camera initializes and
 * captures frames. No WiFi, no SD, no HTTP POST: the XIAO equivalent of the
 * ESP32-CAM bench sketch (11-esp32-cam). One USB-C *data* cable does power +
 * flash + serial; the XIAO auto-resets into the bootloader, so no jumpers and
 * no boot-button dance.
 *
 * Validated 2026-06-19: PSRAM 8MB found, camera init OK, sensor PID 0x3660
 * (OV3660), steady ~16 KB SVGA JPEG frames (DL-077). Pin map is the fixed
 * CAMERA_MODEL_XIAO_ESP32S3 layout.
 */
#include <Arduino.h>
#include "esp_camera.h"

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

static int frameN = 0;

void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 3000) { delay(10); }  // bounded wait, never hangs
  delay(500);
  Serial.println();
  Serial.println("XIAO ESP32-S3 Sense — camera bring-up");
  Serial.printf("PSRAM: %s\n", psramFound() ? "found" : "NOT FOUND (check build flags)");

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
  config.frame_size   = FRAMESIZE_SVGA;   // 800x600 for bring-up
  config.jpeg_quality = 12;
  config.fb_count     = psramFound() ? 2 : 1;
  config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location  = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("CAMERA INIT FAILED: 0x%x\n", err);
    Serial.println("Check: camera board seated on the B2B connector, ribbon latched.");
    while (true) { delay(1000); }
  }
  Serial.println("Camera init OK");

  sensor_t *s = esp_camera_sensor_get();
  if (s) Serial.printf("Sensor PID: 0x%x  (OV2640=0x26, OV3660=0x3660, OV5640=0x5640)\n",
                       s->id.PID);

  for (int i = 0; i < 4; i++) {              // warm up auto-exposure
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb) esp_camera_fb_return(fb);
    delay(100);
  }
  Serial.println("Capturing frames...");
}

void loop() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("frame grab returned NULL");
    delay(1000);
    return;
  }
  Serial.printf("Frame %d  %ux%u  %u bytes\n",
                ++frameN, fb->width, fb->height, (unsigned)fb->len);
  esp_camera_fb_return(fb);
  delay(2000);
}
