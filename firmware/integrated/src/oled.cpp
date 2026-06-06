#include "oled.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"

static Adafruit_SSD1306 display(128, 64, &Wire, -1);  // -1: no reset pin
static bool ok = false;

void oled_begin() {
    ok = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    if (!ok) {
        Serial.println("[OLED] not found - display disabled (non-fatal)");
        return;
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("Plant Autonomy"));
    display.println(F("starting..."));
    display.display();
    Serial.println("[OLED] ready");
}

void oled_render(const char* state, const Bme280Reading& air, const SoilReading& soil,
                 const FloatReading& flt, const LeakReading& leak,
                 bool pump_on, unsigned long daily_pump_ms, unsigned long cap_ms) {
    if (!ok) {
        return;
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    // Row 0 (y=0): state headline
    display.setCursor(0, 0);
    display.print(state);

    // Row 1 (y=8): pump
    display.setCursor(0, 8);
    display.print(F("Pump: "));
    display.print(pump_on ? F("ON") : F("off"));

    // Row 2 (y=16): soil
    display.setCursor(0, 16);
    display.print(F("Soil: "));
    if (soil.valid) {
        display.print(soil.moisture_pct, 0);
        display.print(F("% ("));
        display.print(soil.raw);
        display.print(F(")"));
    } else {
        display.print(F("--"));
    }

    // Row 3 (y=24): reservoir
    display.setCursor(0, 24);
    display.print(F("Reservoir: "));
    display.print(flt.reservoir_empty ? F("EMPTY") : F("ok"));

    // Row 4 (y=32): leak
    display.setCursor(0, 32);
    display.print(F("Leak: "));
    display.print(leak.detected ? F("WET") : F("dry"));

    // Row 5 (y=40): air
    display.setCursor(0, 40);
    if (air.valid) {
        display.print(F("Air: "));
        display.print(air.temperature_c, 1);
        display.print(F("C "));
        display.print(air.humidity_pct, 0);
        display.print(F("%"));
    } else {
        display.print(F("Air: --"));
    }

    // Row 6 (y=48): daily pump-time budget (seconds)
    display.setCursor(0, 48);
    display.print(F("Daily: "));
    display.print(daily_pump_ms / 1000);
    display.print(F("/"));
    display.print(cap_ms / 1000);
    display.print(F("s"));

    display.display();
}
