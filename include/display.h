#pragma once
// =============================================================================
// OLED Display Driver – SSD1306 128×32 (0.91") via I2C
// =============================================================================

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"  // for isValidTemperature()

namespace display {

/// Global display instance (constructed on first use)
inline Adafruit_SSD1306& getDisplay() {
    static Adafruit_SSD1306 oled(config::OLED_WIDTH, config::OLED_HEIGHT, &Wire, -1);
    return oled;
}

/// Scan I2C bus and print found devices (for debugging)
inline void scanI2C() {
    Serial.printf("I2C scan on SDA=%u, SCL=%u:\n", config::PIN_I2C_SDA, config::PIN_I2C_SCL);
    uint8_t found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("  Found device at 0x%02X\n", addr);
            found++;
            break;
        }
    }
    if (found == 0) {
        Serial.println("  No I2C devices found! Check wiring.");
    }
}

/// Initialize I2C and OLED display.
/// @return true on success, false if display not detected
inline bool init() {
    Wire.begin(config::PIN_I2C_SDA, config::PIN_I2C_SCL);
    
    // In debug mode, scan for I2C devices
    #ifdef DEBUG_MODE
    scanI2C();
    #endif

    if (!getDisplay().begin(SSD1306_SWITCHCAPVCC, config::OLED_I2C_ADDR)) {
        Serial.printf("ERROR: SSD1306 not found at 0x%02X\n", config::OLED_I2C_ADDR);
        return false;
    }

    Serial.println("OLED initialized successfully");
    getDisplay().clearDisplay();
    getDisplay().display();
    return true;
}

/// Turn off the display (low power).
inline void off() {
    getDisplay().ssd1306_command(SSD1306_DISPLAYOFF);
}

/// Deinitialize I2C bus to save power before deep sleep.
inline void deinit() {
    Wire.end();
}

/// Turn on the display.
inline void on() {
    getDisplay().ssd1306_command(SSD1306_DISPLAYON);
}

/// Clear screen buffer.
inline void clear() {
    getDisplay().clearDisplay();
}

/// Push buffer to screen.
inline void show() {
    getDisplay().display();
}

/// Show temperature on screen (large, centered).
/// @param tempC Temperature in Celsius (or DEVICE_DISCONNECTED_C on error)
/// Layout optimized for 128x32 display.
inline void showTemperature(float tempC) {
    auto& oled = getDisplay();
    oled.clearDisplay();

    oled.setTextColor(SSD1306_WHITE);

    // For 32px height: use size 2 for temp, small label on top
    // Title (small, top-left)
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.print("Boiler:");

    // Temperature value (size 2 fits in 32px height)
    oled.setTextSize(2);
    oled.setCursor(0, 14);

    if (config::isValidTemperature(tempC)) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f C", tempC);
        oled.print(buf);
    } else {
        oled.print("ERROR");
    }

    oled.display();
}

/// Show a simple message on screen.
inline void showMessage(const char* line1, const char* line2 = nullptr) {
    auto& oled = getDisplay();
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);

    oled.setCursor(0, 10);
    oled.print(line1);

    if (line2) {
        oled.setCursor(0, 30);
        oled.print(line2);
    }

    oled.display();
}

}  // namespace display
