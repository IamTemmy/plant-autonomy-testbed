/*
 * Plant Autonomy Testbed — diagnostic sketch: I2C bus scanner (DL-132)
 *
 * Scans the shared I2C bus and prints every address that ACKs, flagging the two
 * environmental sensors specifically. Use this to tell apart two very different
 * situations that look identical from the hub's point of view (stale lux/temp):
 *
 *   - Sensors ACK on the bus  -> they are ELECTRICALLY ALIVE. The stale telemetry
 *     was because the running firmware (the watering harness) does not read I2C.
 *     No hardware fault. Fix = run firmware that reads them.
 *
 *   - Sensors do NOT ACK      -> a genuine hardware fault (shared SDA/SCL/power/
 *     ground, or a dead sensor). Then do the isolation test: remove one device,
 *     re-scan, see if the other returns.
 *
 * Wiring (unchanged from the integrated build):
 *   I2C SDA -> GPIO21    I2C SCL -> GPIO22    both sensors on 3V3 + common GND
 *
 * Expected addresses:
 *   BME280  0x76 (primary) or 0x77 (alt)
 *   BH1750  0x23
 *
 * Output: a full 0x01..0x7E scan once per second, with a PASS/FAIL summary line
 * for the two expected sensors. Serial @ 115200. Touches nothing but the I2C bus.
 */
#include <Arduino.h>
#include <Wire.h>

static constexpr uint8_t I2C_SDA      = 21;
static constexpr uint8_t I2C_SCL      = 22;
static constexpr uint8_t BME280_ADDR  = 0x76;
static constexpr uint8_t BME280_ALT   = 0x77;
static constexpr uint8_t BH1750_ADDR  = 0x23;

static bool acks(uint8_t addr) {
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n=== I2C bus scanner (DL-132 diagnostic) ===");
    Serial.printf("SDA=GPIO%u  SCL=GPIO%u\n", I2C_SDA, I2C_SCL);
    Serial.println("Expecting: BME280 @ 0x76/0x77, BH1750 @ 0x23");
    Wire.begin(I2C_SDA, I2C_SCL);
}

void loop() {
    Serial.println("\n--- scanning 0x01..0x7E ---");
    uint8_t count = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (acks(addr)) {
            Serial.printf("  device @ 0x%02X", addr);
            if (addr == BME280_ADDR || addr == BME280_ALT) Serial.print("  <- BME280");
            else if (addr == BH1750_ADDR)                  Serial.print("  <- BH1750");
            Serial.println();
            count++;
        }
    }
    if (count == 0)
        Serial.println("  NONE — no I2C devices ACK. Check shared SDA/SCL/3V3/GND, then isolate.");

    // Explicit PASS/FAIL for the two sensors we care about.
    bool bme = acks(BME280_ADDR) || acks(BME280_ALT);
    bool bh  = acks(BH1750_ADDR);
    Serial.printf("SUMMARY: BME280 %s | BH1750 %s\n",
                  bme ? "PRESENT" : "MISSING",
                  bh  ? "PRESENT" : "MISSING");
    delay(1000);
}
