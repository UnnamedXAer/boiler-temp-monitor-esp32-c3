## Intro:

I bought ESP32 C3 SuperMini and now I would like to create a project using PlatformIO (with "arduino" framework) to monitor temperature on my boiler using `DS18B20` sensor.

Let's create a detailed requirements for this project base on high-lever overview.

This project is suppose to:

- measure temperature of my boiler by the DS18B20 sensor.
- those reading should be available to the household potentially outside of home network.
- the boiler is no working all the time, for the most part of the day it does not work so there are no real changes in the temperature, but when it is on, the temperature can change therefore to save power the reading should be less frequent when the boiler is off (the temperature is holding relatively low <30C and still for long time) but increase in frequency when the temperature raises, as the temperature raises.
- potentially down the road we would like to have a notification system if the temperature reaches a threshold.
- the board is intended to be powered by a small battery, so it should be power efficient, probably ho to sleep if the interval between readings are long enough.
- there will be attached SSD1306 0.96 inch I2C OLED plus a button to control it.
- this display must be only on when a user presses button
- when the button is pressed, the board should wake up (if needed), the temperature should be read and displayed on the screen for 10 seconds.

## Summary Project Requirements / Goals

- Functional: Measure boiler temperature with DS18B20; validate sensor disconnect handling; expose latest reading locally and remotely.
- Hardware wiring: DS18B20 on chosen GPIO with 4.7kΩ pull-up; SSD1306 0.96" I2C OLED on SDA/SCL; single button GPIO with pull-up/down defined; battery supply constraints documented.
- Sampling policy: Low-frequency reads while temp < 30°C and stable; adaptive faster reads as temperature rises; debounce transitions (hysteresis/time windows) to avoid thrashing.
- Power management: Use light sleep/deep sleep between reads; wake sources include timer and button; minimize peripherals on (disable Wi-Fi/Bluetooth when idle if not needed); budget current for OLED on-time.
- Display behavior: OLED normally off; on button press, wake (if sleeping), take immediate reading, display for 10 seconds, then turn off; optional short grace period to coalesce rapid button presses.
- Connectivity: Choose and implement transport (Wi-Fi STA with optional captive portal config); define remote access pattern (API endpoint/MQTT) for off-LAN visibility; handle reconnection/backoff; optional TLS support noted.
- Telemetry format: Define payload schema (temp °C, timestamp, battery level if available, status flags); sampling cadence aligned with adaptive policy; retain last-good reading locally.
- Notifications (future): Threshold definitions (high temp, sensor fault); notification channel placeholder (MQTT topic/webhook); rate limiting.
- Diagnostics: Serial logging gated by build flag; basic self-test on boot (sensor presence, I2C scan optional); error codes/messages for OLED.
- Configuration: Centralize constants (GPIOs, temp thresholds, sampling intervals, display duration); support runtime overrides via simple config file/NVS or serial commands.
- Security: Store Wi-Fi credentials securely (NVS); avoid hardcoded secrets in firmware builds.
- Testing: Bench test with stable temp and rising temp scenarios to verify adaptive sampling; button/display wake workflow; sleep current measurement with display off; upload/boot with USB-CDC verified.
