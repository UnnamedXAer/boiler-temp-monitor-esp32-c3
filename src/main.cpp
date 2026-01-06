#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Use GPIO4 for DS18B20 data (adjust if you wired differently)
constexpr uint8_t ONE_WIRE_BUS = 4;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Boiler Temperature Monitor starting...");
    sensors.begin();
}

void loop() {
    sensors.requestTemperatures();
    const float tempC = sensors.getTempCByIndex(0);

    if (tempC != DEVICE_DISCONNECTED_C) {
        Serial.printf("Boiler Temperature: %.2f C\n", tempC);
    } else {
        Serial.println("Sensor not detected");
    }

    delay(2000);
}