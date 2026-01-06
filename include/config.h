#pragma once
// =============================================================================
// Boiler Temperature Monitor – Hardware & Timing Configuration
// Target: ESP32-C3 SuperMini
// =============================================================================

#include <cstdint>

namespace config {

// -----------------------------------------------------------------------------
// GPIO Pin Assignments
// -----------------------------------------------------------------------------
// DS18B20 OneWire data line (requires external 4.7kΩ pull-up to 3.3V)
constexpr uint8_t PIN_ONEWIRE = 4;

// SSD1306 OLED I2C bus (external pull-ups recommended; internal pull-ups enabled)
constexpr uint8_t PIN_I2C_SDA = 8;
constexpr uint8_t PIN_I2C_SCL = 9;

// Wake / display button (directly to GND; internal pull-up used)
constexpr uint8_t PIN_BUTTON = 3;

// -----------------------------------------------------------------------------
// Display Settings
// -----------------------------------------------------------------------------
constexpr uint8_t OLED_I2C_ADDR    = 0x3C;        // Typical SSD1306 address
constexpr uint8_t OLED_WIDTH       = 128;
constexpr uint8_t OLED_HEIGHT      = 64;
constexpr uint32_t DISPLAY_ON_MS   = 10000;       // OLED on-time after button press

// -----------------------------------------------------------------------------
// Temperature Thresholds & Adaptive Sampling (°C)
// -----------------------------------------------------------------------------
constexpr float TEMP_LOW_THRESHOLD     = 30.0f;   // Below this = boiler idle
constexpr float TEMP_HIGH_THRESHOLD    = 60.0f;   // Above this = boiler at peak
constexpr float TEMP_HYSTERESIS        = 2.0f;    // Debounce band

// -----------------------------------------------------------------------------
// Sampling Intervals (seconds)
// -----------------------------------------------------------------------------
constexpr uint32_t SAMPLE_INTERVAL_IDLE_S   = 60;   // When temp < TEMP_LOW_THRESHOLD
constexpr uint32_t SAMPLE_INTERVAL_ACTIVE_S = 5;    // When temp >= TEMP_HIGH_THRESHOLD
constexpr uint32_t SAMPLE_INTERVAL_ERROR_S  = 10;   // Retry interval on sensor error

// -----------------------------------------------------------------------------
// Notification Settings
// -----------------------------------------------------------------------------
constexpr float TEMP_ALERT_THRESHOLD   = 80.0f;   // Send alert above this temp
constexpr uint32_t WIFI_TIMEOUT_MS     = 10000;   // Wi-Fi connection timeout
constexpr bool NOTIFY_ON_EACH_READ     = false;   // true = notify every reading
                                                   // false = only on alerts

// -----------------------------------------------------------------------------
// Fault Handling
// -----------------------------------------------------------------------------
constexpr uint8_t SENSOR_ERROR_NOTIFY_AFTER = 3;  // Send alert after N consecutive failures

// -----------------------------------------------------------------------------
// Serial Logging
// -----------------------------------------------------------------------------
constexpr uint32_t SERIAL_BAUD = 115200;

}  // namespace config
