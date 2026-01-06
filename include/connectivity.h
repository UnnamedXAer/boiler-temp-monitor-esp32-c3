#pragma once
// =============================================================================
// Connectivity – Wi-Fi + ntfy.sh Push Notifications
// =============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "config.h"
#include "secrets.h"

namespace connectivity {

/// Maximum Wi-Fi connection retries
constexpr uint8_t WIFI_MAX_RETRIES = 3;

/// Connect to Wi-Fi with timeout and retry logic.
/// @param timeoutMs Maximum time per attempt.
/// @param maxRetries Number of connection attempts.
/// @return true if connected, false after all retries exhausted.
inline bool connectWiFi(uint32_t timeoutMs = 10000, uint8_t maxRetries = WIFI_MAX_RETRIES) {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    WiFi.mode(WIFI_STA);

    for (uint8_t attempt = 1; attempt <= maxRetries; ++attempt) {
        Serial.printf("Wi-Fi '%s' attempt %u/%u...\n", secrets::WIFI_SSID, attempt, maxRetries);
        WiFi.begin(secrets::WIFI_SSID, secrets::WIFI_PASSWORD);

        const uint32_t startMs = millis();
        while (WiFi.status() != WL_CONNECTED) {
            if (millis() - startMs >= timeoutMs) {
                Serial.println("  timeout");
                WiFi.disconnect(true);
                break;
            }
            delay(100);
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("Wi-Fi connected! IP: %s\n", WiFi.localIP().toString().c_str());
            return true;
        }

        // Brief pause before retry
        if (attempt < maxRetries) {
            delay(500);
        }
    }

    Serial.println("Wi-Fi connection failed after retries");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return false;
}

/// Disconnect Wi-Fi and turn off radio to save power.
inline void disconnectWiFi() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("Wi-Fi off");
}

/// Maximum HTTP request retries
constexpr uint8_t HTTP_MAX_RETRIES = 2;

/// Send a push notification via ntfy.sh with retry logic.
/// @param message The notification body text.
/// @param title Optional title (shown bold in notification).
/// @param priority Priority: 1=min, 3=default, 5=urgent.
/// @param maxRetries Number of send attempts.
/// @return true if sent successfully (HTTP 200), false otherwise.
inline bool sendNtfyNotification(const char* message, 
                                  const char* title = nullptr,
                                  uint8_t priority = 3,
                                  uint8_t maxRetries = HTTP_MAX_RETRIES) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("ntfy: Wi-Fi not connected");
        return false;
    }

    String url = "https://ntfy.sh/";
    url += secrets::NTFY_TOPIC;

    for (uint8_t attempt = 1; attempt <= maxRetries; ++attempt) {
        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "text/plain");
        http.setTimeout(5000);  // 5 second timeout

        if (title != nullptr) {
            http.addHeader("Title", title);
        }

        // Priority: 1=min, 2=low, 3=default, 4=high, 5=urgent
        if (priority != 3) {
            http.addHeader("Priority", String(priority));
        }

        Serial.printf("ntfy: Sending (attempt %u/%u)...\n", attempt, maxRetries);
        int httpCode = http.POST(message);
        http.end();

        if (httpCode == 200) {
            Serial.println("ntfy: Notification sent");
            return true;
        }

        Serial.printf("ntfy: HTTP %d\n", httpCode);

        // Brief pause before retry
        if (attempt < maxRetries) {
            delay(500);
        }
    }

    Serial.println("ntfy: Failed after retries");
    return false;
}

/// Send a temperature reading notification.
/// @param tempC Current temperature in Celsius.
/// @param isAlert true if this is a threshold alert (higher priority).
inline bool sendTemperatureNotification(float tempC, bool isAlert = false) {
    char message[64];

    if (tempC == DEVICE_DISCONNECTED_C) {
        snprintf(message, sizeof(message), "⚠️ Sensor disconnected!");
        return sendNtfyNotification(message, "Boiler Alert", 5);
    }

    if (isAlert) {
        snprintf(message, sizeof(message), "🔥 High temp: %.1f °C", tempC);
        return sendNtfyNotification(message, "Boiler Alert", 4);
    }

    snprintf(message, sizeof(message), "🌡️ Boiler: %.1f °C", tempC);
    return sendNtfyNotification(message, "Boiler Temp", 3);
}

}  // namespace connectivity
