#pragma once
// =============================================================================
// Secrets – Wi-Fi & Notification Credentials
// Copy this file to secrets.h and fill in your values.
// secrets.h is gitignored – never commit real credentials!
// =============================================================================

namespace secrets {

// Wi-Fi credentials
constexpr const char* WIFI_SSID     = "YOUR_WIFI_SSID";
constexpr const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ntfy.sh topic (acts as simple auth – use a random string)
// Example: "boiler-temp-a7x9k2m"
// Subscribe to this topic in the ntfy app to receive notifications
constexpr const char* NTFY_TOPIC    = "YOUR_NTFY_TOPIC";

// Telegram bot (future – leave empty for now)
// constexpr const char* TELEGRAM_BOT_TOKEN = "";
// constexpr const char* TELEGRAM_CHAT_ID   = "";

}  // namespace secrets
