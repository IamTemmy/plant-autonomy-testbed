/*
 * Plant Autonomy Testbed — Phase 1 component test
 * Module: ESP32-CAM (OV2640 camera sensor)
 *
 * This is a bare-bones camera initialization and capture test.
 * No WiFi, no streaming — just confirms:
 *   1. The board enumerates correctly with the FTDI programmer
 *   2. The OV2640 camera initializes without errors
 *   3. Frame capture works and returns sensible image data
 *   4. PSRAM is detected and usable for the framebuffer
 *
 * Once this passes, we add WiFi connectivity in a follow-up sketch.
 *
 * Wiring (FTDI -> ESP32-CAM):
 *   FTDI VCC (5V) -> ESP32-CAM 5V
 *   FTDI GND      -> ESP32-CAM GND
 *   FTDI TX       -> ESP32-CAM U0R (RX)
 *   FTDI RX       -> ESP32-CAM U0T (TX)
 *   Extra jumper: ESP32-CAM IO0 -> GND (during flashing only; remove after)
 *
 * After flashing: unplug FTDI, remove the IO0-to-GND jumper, plug FTDI
 * back in to view the serial output.
 */

#include "esp_camera.h"
#include <Arduino.h>

// Pin map for the standard AI-Thinker ESP32-CAM board (the most common one).
// These come from the Espressif example's camera_pins.h — copied inline so
// this sketch has no external pin-config dependency.
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

static uint32_t frame_count = 0;
static uint32_t start_ms = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println(F("Plant Autonomy Testbed — ESP32-CAM bench test"));
  Serial.println(F("============================================="));

  Serial.print(F("PSRAM detected: "));
  Serial.println(psramFound() ? F("yes") : F("no"));
  if (psramFound()) {
    Serial.print(F("PSRAM size: "));
    Serial.print(ESP.getPsramSize() / 1024);
    Serial.println(F(" KB"));
  }

  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_VGA;      // 640x480, conservative starting size
  config.jpeg_quality = 12;                  // 0-63, lower = higher quality
  config.fb_count     = 1;                   // 1 framebuffer for the basic test
  config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;

  Serial.println(F("Initializing camera..."));
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.print(F("ERROR: camera init failed: 0x"));
    Serial.println(err, HEX);
    Serial.println(F("Common causes:"));
    Serial.println(F("  - Ribbon cable not fully seated"));
    Serial.println(F("  - Insufficient supply voltage (need 5V on VCC, not 3.3V)"));
    Serial.println(F("  - PSRAM not enabled in platformio.ini"));
    Serial.println(F("Halting."));
    while (true) { delay(1000); }
  }
  Serial.println(F("Camera initialized successfully."));

  start_ms = millis();
}

void loop() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println(F("ERROR: frame grab returned NULL"));
    delay(1000);
    return;
  }

  frame_count++;
  uint32_t elapsed = (millis() - start_ms) / 1000;

  Serial.print(F("Frame "));
  Serial.print(frame_count);
  Serial.print(F("  size="));
  Serial.print(fb->len);
  Serial.print(F(" bytes  "));
  Serial.print(fb->width);
  Serial.print(F("x"));
  Serial.print(fb->height);
  Serial.print(F("  elapsed="));
  Serial.print(elapsed);
  Serial.println(F("s"));

  esp_camera_fb_return(fb);

  delay(2000);  // 1 frame every 2 seconds is plenty for a validation test
}
