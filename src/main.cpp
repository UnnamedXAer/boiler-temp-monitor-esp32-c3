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

// Adaptive sampling policy (reset each boot from deep sleep)
static sampling::SamplingPolicy samplingPolicy;

// -----------------------------------------------------------------------------
// RTC memory – persists across deep sleep (not power cycle)
// -----------------------------------------------------------------------------
RTC_DATA_ATTR static uint8_t sensorErrorCount = 0;
RTC_DATA_ATTR static bool sensorErrorNotified = false;

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------
static float readTemperature();
static void handleButtonWake();
static void sendNotificationIfNeeded(float tempC);

// -----------------------------------------------------------------------------
// Setup – runs on every wake from deep sleep
// -----------------------------------------------------------------------------
void setup() {
    Serial.begin(config::SERIAL_BAUD);
    delay(500);  // Allow USB-CDC to enumerate

    Serial.println("=== Boiler Temperature Monitor ===");
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
    if (tempC == DEVICE_DISCONNECTED_C) {
        sensorErrorCount++;
        Serial.printf("Sensor error count: %u\n", sensorErrorCount);
    } else {
        // Reset error tracking on successful read
        sensorErrorCount = 0;
        sensorErrorNotified = false;
    }

    // If woken by button, show display
    if (power::wokeFromButton()) {
        handleButtonWake();
    }

    // Send notification if needed (alert threshold or NOTIFY_ON_EACH_READ)
    sendNotificationIfNeeded(tempC);

    // Determine next sampling interval
    uint32_t intervalS = config::SAMPLE_INTERVAL_IDLE_S;

    if (tempC != DEVICE_DISCONNECTED_C) {
        intervalS = samplingPolicy.update(tempC);

        const char* modeStr = (samplingPolicy.currentMode() == sampling::SamplingPolicy::Mode::Idle)
                              ? "IDLE" : "ACTIVE";

        Serial.printf("Temp: %.2f C  [%s]  sleep for %u s\n", tempC, modeStr, intervalS);
    } else {
        Serial.printf("ERROR: Sensor disconnected – retrying in %u s\n", config::SAMPLE_INTERVAL_ERROR_S);
        intervalS = config::SAMPLE_INTERVAL_ERROR_S;
    }

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
static void handleButtonWake() {
    Serial.println("Button pressed – showing temperature on OLED");

    // Read temperature for display
    const float tempC = readTemperature();

    // Initialize and show display
    if (display::init()) {
        display::on();
        display::showTemperature(tempC);

        // Keep display on for configured duration
        delay(config::DISPLAY_ON_MS);

        // Turn off display before sleeping
        display::off();
        Serial.println("Display off");
    } else {
        Serial.println("Display init failed – skipping");
    }
}

// -----------------------------------------------------------------------------
// Send notification via ntfy.sh if conditions are met
// -----------------------------------------------------------------------------
static void sendNotificationIfNeeded(float tempC) {
    const bool isHighTempAlert = (tempC != DEVICE_DISCONNECTED_C && tempC >= config::TEMP_ALERT_THRESHOLD);

    // Sensor error: only notify after N consecutive failures, and only once
    const bool isSensorError = (tempC == DEVICE_DISCONNECTED_C);
    const bool shouldNotifySensorError = isSensorError 
                                       && (sensorErrorCount >= config::SENSOR_ERROR_NOTIFY_AFTER)
                                       && !sensorErrorNotified;

    // Determine if we need to send anything
    const bool shouldNotify = config::NOTIFY_ON_EACH_READ || isHighTempAlert || shouldNotifySensorError;

    if (!shouldNotify) {
        return;
    }

    // Connect and send
    if (connectivity::connectWiFi(config::WIFI_TIMEOUT_MS)) {
        if (shouldNotifySensorError) {
            connectivity::sendTemperatureNotification(DEVICE_DISCONNECTED_C, false);
            sensorErrorNotified = true;  // Don't spam on every wake
        } else {
            connectivity::sendTemperatureNotification(tempC, isHighTempAlert);
        }
        connectivity::disconnectWiFi();
    }
}