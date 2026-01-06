#pragma once
// =============================================================================
// Power Management – Deep Sleep with Timer & GPIO Wake
// Target: ESP32-C3 (Arduino framework)
// =============================================================================

#include <Arduino.h>
#include <esp_sleep.h>

#include "config.h"

namespace power {

/// Minimum sleep duration (seconds) below which we skip deep sleep
/// (the wake-up overhead makes very short sleeps inefficient)
constexpr uint32_t MIN_SLEEP_THRESHOLD_S = 5;

/// Configure GPIO wake source (button) for deep sleep.
/// ESP32-C3 RTC-capable GPIOs are GPIO0-5 only.
/// Call once during setup.
inline void initWakeSources() {
    // Configure button pin as input with pull-up
    pinMode(config::PIN_BUTTON, INPUT_PULLUP);

    // ESP32-C3 deep sleep requires esp_deep_sleep_enable_gpio_wakeup()
    // GPIO3 is RTC-capable (GPIO0-5 are RTC IOs on ESP32-C3)
    const uint64_t buttonMask = 1ULL << config::PIN_BUTTON;
    esp_deep_sleep_enable_gpio_wakeup(buttonMask, ESP_GPIO_WAKEUP_GPIO_LOW);
}

/// Enter deep sleep for the specified duration.
/// @param seconds Duration in seconds; if below threshold, uses light delay instead.
/// @return true if woke from deep sleep, false if skipped (duration too short).
inline bool sleepFor(uint32_t seconds) {
    if (seconds < MIN_SLEEP_THRESHOLD_S) {
        // Too short for deep sleep – just delay
        delay(seconds * 1000);
        return false;
    }

    Serial.printf("Entering deep sleep for %u s...\n", seconds);
    Serial.flush();

    // Enable timer wake
    esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(seconds) * 1000000ULL);

    // Go to deep sleep (GPIO wake was already enabled in initWakeSources)
    esp_deep_sleep_start();

    // Never reached – MCU resets on wake
    return true;
}

/// Check the wake-up cause after reset.
inline esp_sleep_wakeup_cause_t getWakeupCause() {
    return esp_sleep_get_wakeup_cause();
}

/// Return true if the last boot was caused by the button (GPIO wake).
inline bool wokeFromButton() {
    return getWakeupCause() == ESP_SLEEP_WAKEUP_GPIO;
}

/// Return true if the last boot was caused by the timer.
inline bool wokeFromTimer() {
    return getWakeupCause() == ESP_SLEEP_WAKEUP_TIMER;
}

/// Return a human-readable string for the wake-up cause.
inline const char* wakeupCauseStr() {
    switch (getWakeupCause()) {
        case ESP_SLEEP_WAKEUP_TIMER:      return "TIMER";
        case ESP_SLEEP_WAKEUP_GPIO:       return "BUTTON";
        case ESP_SLEEP_WAKEUP_EXT0:       return "EXT0";
        case ESP_SLEEP_WAKEUP_EXT1:       return "EXT1";
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:                          return "POWER_ON";
    }
}

}  // namespace power
