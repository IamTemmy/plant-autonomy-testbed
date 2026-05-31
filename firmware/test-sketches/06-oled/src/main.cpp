/*
 * Plant Autonomy Testbed — Phase 1 component test
 * Display: SSD1306 128x64 OLED, yellow/blue split (top 16px yellow, bottom 48px blue)
 *
 * Wiring:
 *   OLED VCC -> 3.3V on extension board
 *   OLED GND -> GND rail
 *   OLED SCL -> GPIO22 (shared I2C bus with BME280 and BH1750)
 *   OLED SDA -> GPIO21 (shared I2C bus)
 *
 * Test behavior:
 *   1. Splash screen on boot (2 seconds)
 *   2. Mock dashboard refreshing once per second, with a live uptime counter
 *      so the screen is visibly active (proves we are not looking at a static image)
 *
 * Mock dashboard layout (preview of the eventual Phase 2 UI):
 *   Yellow zone (top 16 px):  project title + phase label
 *   Blue zone (bottom 48 px): sensor data lines + uptime
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static const uint8_t  SCREEN_WIDTH   = 128;
static const uint8_t  SCREEN_HEIGHT  = 64;
static const int8_t   OLED_RESET_PIN = -1;       // no dedicated reset line
static const uint8_t  OLED_ADDRESS   = 0x3C;     // default for most SSD1306 modules

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);

uint32_t start_ms = 0;

void drawDashboard(uint32_t uptime_s) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Yellow zone — title (y = 0..15)
  display.setCursor(0, 0);
  display.print(F("PLANT AUTONOMY"));
  display.setCursor(0, 8);
  display.print(F("Phase 1 bench test"));

  // Blue zone — mock sensor data (y = 16..63)
  display.setCursor(0, 18);
  display.print(F("Temp:   24.3 C"));
  display.setCursor(0, 28);
  display.print(F("Humid:  48 %"));
  display.setCursor(0, 38);
  display.print(F("Light:  120 lx"));
  display.setCursor(0, 48);
  display.print(F("Soil:   45 %"));
  display.setCursor(0, 56);
  display.print(F("Up: "));
  display.print(uptime_s);
  display.print(F("s   Pump: idle"));

  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("Plant Autonomy Testbed — OLED bench test"));
  Serial.println(F("========================================"));

  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("ERROR: SSD1306 not found at I2C address 0x3C."));
    Serial.println(F("Check wiring (SDA->GPIO21, SCL->GPIO22, VCC->3V3, GND->GND)."));
    Serial.println(F("Some modules use 0x3D — verify with an I2C scanner if 0x3C fails."));
    while (true) { delay(1000); }
  }

  Serial.print(F("SSD1306 detected at I2C address 0x"));
  Serial.println(OLED_ADDRESS, HEX);
  Serial.println(F("Displaying splash, then mock dashboard refreshing at 1 Hz."));

  // Splash
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 4);
  display.print(F("PLANT AUTONOMY"));
  display.setCursor(40, 32);
  display.print(F("TESTBED"));
  display.setCursor(28, 48);
  display.print(F("booting..."));
  display.display();
  delay(2000);

  start_ms = millis();
}

void loop() {
  uint32_t uptime_s = (millis() - start_ms) / 1000;
  drawDashboard(uptime_s);
  delay(1000);
}
