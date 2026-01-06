#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "config.h"
#include "sampling.h"
#include "power.h"
#include "display.h"
#include "connectivity.h"

// -----------------------------------------------------------------------------
// Hardware instances
// -----------------------------------------------------------------------------
static OneWire oneWire(config::PIN_ONEWIRE);
static DallasTemperature sensors(&oneWire);

// -----------------------------------------------------------------------------
// RTC memory – persists across deep sleep (not power cycle)
// -----------------------------------------------------------------------------
RTC_DATA_ATTR static uint8_t sensorErrorCount = 0;
RTC_DATA_ATTR static bool sensorErrorNotified = false;
RTC_DATA_ATTR static sampling::Mode samplingMode = sampling::Mode::Idle;
RTC_DATA_ATTR static uint32_t lastAlertTimeS = 0;      // Uptime (seconds) of last high-temp alert
RTC_DATA_ATTR static uint32_t cumulativeUptimeS = 0;   // Accumulated uptime across sleep cycles

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------
static float readTemperature();
static void handleButtonWake(float tempC);
static void sendNotificationIfNeeded(float tempC);

// -----------------------------------------------------------------------------
// Setup – runs on every wake from deep sleep
// -----------------------------------------------------------------------------
void setup() {
    Serial.begin(config::SERIAL_BAUD);
    delay(500);  // Allow USB-CDC to enumerate

    Serial.println("=== Boiler Temperature Monitor ===");
    if constexpr (config::DEBUG_ENABLED) {
        Serial.println("[DEBUG MODE ENABLED]");
    }
    Serial.printf("Wake reason: %s\n", power::wakeupCauseStr());

    // Initialize wake sources for next sleep cycle
    power::initWakeSources();

    // Initialize temperature sensor
    sensors.begin();
    const uint8_t sensorCount = sensors.getDeviceCount();
    Serial.printf("DS18B20 sensors found: %u\n", sensorCount);

    if (sensorCount == 0) {
        Serial.println("WARNING: No DS18B20 detected – check wiring & pull-up resistor");
    }

    // Read temperature
    const float tempC = readTemperature();

    // Track consecutive sensor errors
    if (!config::isValidTemperature(tempC)) {
        sensorErrorCount++;
        Serial.printf("Sensor error count: %u\n", sensorErrorCount);
    } else {
        // Reset error tracking on successful read
        sensorErrorCount = 0;
        sensorErrorNotified = false;
    }

    // If woken by button, show display
    if (power::wokeFromButton()) {
        handleButtonWake(tempC);
    }

    // Send notification if needed (alert threshold or NOTIFY_ON_EACH_READ)
    sendNotificationIfNeeded(tempC);

    // Determine next sampling interval
    uint32_t intervalS;

    if (config::isValidTemperature(tempC)) {
        // Restore mode from RTC memory, update, then save back
        sampling::SamplingPolicy policy(samplingMode);
        intervalS = policy.update(tempC);
        samplingMode = policy.currentMode();  // Persist for next wake

        const char* modeStr = (samplingMode == sampling::Mode::Idle) ? "IDLE" : "ACTIVE";

        Serial.printf("Temp: %.2f C  [%s]  sleep for %u s\n", tempC, modeStr, intervalS);
    } else {
        Serial.printf("ERROR: Sensor disconnected – retrying in %u s\n", config::SAMPLE_INTERVAL_ERROR_S);
        intervalS = config::SAMPLE_INTERVAL_ERROR_S;
    }

    // Track cumulative uptime for alert cooldown (add sleep duration before sleeping)
    cumulativeUptimeS += intervalS;

    // Enter deep sleep until next reading (or button press)
    power::sleepFor(intervalS);
}

// -----------------------------------------------------------------------------
// Loop – not used when deep sleeping; only runs if sleepFor() skips sleep
// -----------------------------------------------------------------------------
void loop() {
    // If sleep was skipped (interval too short), re-enter setup logic
    delay(100);
    ESP.restart();
}

// -----------------------------------------------------------------------------
// Helper: Request and read temperature from first DS18B20
// -----------------------------------------------------------------------------
static float readTemperature() {
    sensors.requestTemperatures();
    return sensors.getTempCByIndex(0);
}

// -----------------------------------------------------------------------------
// Handle button wake: init display, show temperature, wait, then turn off
// -----------------------------------------------------------------------------
static void handleButtonWake(float tempC) {
    Serial.println("Button pressed – showing temperature on OLED");

    // Initialize and show display
    if (display::init()) {
        display::on();
        display::showTemperature(tempC);

        // Keep display on for configured duration (shorter in debug mode)
        const uint32_t displayDuration = config::DEBUG_ENABLED 
                                        ? config::DEBUG_DISPLAY_ON_MS 
                                        : config::DISPLAY_ON_MS;
        delay(displayDuration);

        // Turn off display before sleeping
        display::off();
        display::deinit();  // Release I2C bus
        Serial.println("Display off");
    } else {
        Serial.println("Display init failed – skipping");
    }
}

// -----------------------------------------------------------------------------
// Send notification via ntfy.sh if conditions are met
// -----------------------------------------------------------------------------
static void sendNotificationIfNeeded(float tempC) {
    const bool isHighTempAlert = (config::isValidTemperature(tempC) && tempC >= config::TEMP_ALERT_THRESHOLD);

    // Check cooldown for high-temp alerts (don't spam if boiler stays hot)
    bool alertCooledDown = true;
    if (isHighTempAlert && lastAlertTimeS > 0) {
        const uint32_t elapsed = cumulativeUptimeS - lastAlertTimeS;
        alertCooledDown = (elapsed >= config::ALERT_COOLDOWN_S);
        if (!alertCooledDown) {
            Serial.printf("Alert cooldown: %u s remaining\n", config::ALERT_COOLDOWN_S - elapsed);
        }
    }

    // Sensor error: only notify after N consecutive failures, and only once
    const bool isSensorError = !config::isValidTemperature(tempC);
    const bool shouldNotifySensorError = isSensorError 
                                       && (sensorErrorCount >= config::SENSOR_ERROR_NOTIFY_AFTER)
                                       && !sensorErrorNotified;

    // Determine if we need to send anything
    const bool shouldNotify = config::NOTIFY_ON_EACH_READ 
                            || (isHighTempAlert && alertCooledDown) 
                            || shouldNotifySensorError;

    if (!shouldNotify) {
        return;
    }

    // Connect and send
    if (connectivity::connectWiFi(config::WIFI_TIMEOUT_MS)) {
        if (shouldNotifySensorError) {
            connectivity::sendTemperatureNotification(DEVICE_DISCONNECTED_C, false);
            sensorErrorNotified = true;  // Don't spam on every wake
        } else {
            connectivity::sendTemperatureNotification(tempC, isHighTempAlert && alertCooledDown);
            if (isHighTempAlert && alertCooledDown) {
                lastAlertTimeS = cumulativeUptimeS;  // Record time of this alert
            }
        }
        connectivity::disconnectWiFi();
    }
}