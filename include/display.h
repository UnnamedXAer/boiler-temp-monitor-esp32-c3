#pragma once
// =============================================================================
// OLED Display Driver – SSD1306 128×64 via I2C
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

/// Initialize I2C and OLED display.
/// @return true on success, false if display not detected
inline bool init() {
    Wire.begin(config::PIN_I2C_SDA, config::PIN_I2C_SCL);

    if (!getDisplay().begin(SSD1306_SWITCHCAPVCC, config::OLED_I2C_ADDR)) {
        Serial.println("ERROR: SSD1306 not found at 0x3C");
        return false;
    }

    getDisplay().clearDisplay();
    getDisplay().display();
    return true;
}

/// Turn off the display (low power).
inline void off() {
    getDisplay().ssd1306_command(SSD1306_DISPLAYOFF);
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
inline void showTemperature(float tempC) {
    auto& oled = getDisplay();
    oled.clearDisplay();

    oled.setTextColor(SSD1306_WHITE);

    // Title
    oled.setTextSize(1);
    oled.setCursor(20, 0);
    oled.print("Boiler Temp");

    // Temperature value (large)
    oled.setTextSize(3);
    oled.setCursor(10, 24);

    if (config::isValidTemperature(tempC)) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f C", tempC);
        oled.print(buf);
    } else {
        oled.print("ERR");
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
