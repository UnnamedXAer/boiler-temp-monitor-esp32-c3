# Boiler Temperature Monitor – Project Requirements

## Overview

Battery-powered ESP32-C3 SuperMini monitors boiler temperature using a DS18B20
sensor, with adaptive sampling, deep sleep, and an on-demand SSD1306 OLED display.
Remote access is planned via Wi-Fi/MQTT.

---

## Hardware

| Component | Specification | GPIO |
|-----------|---------------|------|
| MCU | ESP32-C3 SuperMini | – |
| Temp sensor | DS18B20 (OneWire) | GPIO4 + 4.7 kΩ pull-up |
| Display | SSD1306 128×64 I2C OLED | SDA=GPIO8, SCL=GPIO9 |
| Button | Momentary (NO) to GND | GPIO3 (internal pull-up) |
| Power | 3.3 V from battery | – |

---

## Functional Requirements

### FR-1: Temperature Measurement

- Read DS18B20 on each wake cycle.
- Handle sensor-disconnected condition gracefully; retry after short delay.

### FR-2: Adaptive Sampling

- When boiler is idle (< 30 °C), sample every 60 s.
- When boiler is active (≥ 60 °C), sample every 5 s.
- Linear interpolation for temperatures between thresholds.
- Hysteresis band (±2 °C) prevents mode thrashing.

### FR-3: Power Management

- Enter deep sleep between readings.
- Wake sources: timer (adaptive interval) and button GPIO.
- Skip deep sleep for intervals < 5 s (overhead inefficient).

### FR-4: Display

- OLED is normally off to save power.
- On button press: wake, read temperature, show on OLED for 10 s, then sleep.

### FR-5: Remote Access (planned)

- Wi-Fi STA connection with credentials stored securely (NVS).
- Publish temperature telemetry via MQTT or HTTP.
- Expose readings outside local network (via broker or reverse proxy).

### FR-6: Notifications (future)

- Configurable high-temperature threshold.
- Alert channel (MQTT topic / webhook) for over-threshold events.

---

## Non-Functional Requirements

| ID | Requirement |
|----|-------------|
| NFR-1 | Deep-sleep current < 50 µA (OLED off, sensor idle). |
| NFR-2 | Wake-to-display latency < 1 s. |
| NFR-3 | No heap allocations in hot paths. |
| NFR-4 | All tuning constants centralized in `config.h`. |

---

## Configuration Constants (`include/config.h`)

| Symbol | Value | Description |
|--------|-------|-------------|
| `PIN_ONEWIRE` | 4 | DS18B20 data GPIO |
| `PIN_I2C_SDA` | 8 | OLED SDA |
| `PIN_I2C_SCL` | 9 | OLED SCL |
| `PIN_BUTTON` | 3 | Wake button GPIO |
| `TEMP_LOW_THRESHOLD` | 30.0 °C | Idle mode ceiling |
| `TEMP_HIGH_THRESHOLD` | 60.0 °C | Active mode floor |
| `SAMPLE_INTERVAL_IDLE_S` | 60 s | Idle sampling period |
| `SAMPLE_INTERVAL_ACTIVE_S` | 5 s | Active sampling period |
| `DISPLAY_ON_MS` | 10000 ms | OLED on-time after button |

---

## Wiring Diagram (text)

```text
                    ESP32-C3 SuperMini
                    ┌────────────────┐
  DS18B20 DATA ─────┤ GPIO4          │
  DS18B20 VCC ──────┤ 3V3            │
  DS18B20 GND ──────┤ GND            │
                    │                │
  OLED SDA ─────────┤ GPIO8          │
  OLED SCL ─────────┤ GPIO9          │
  OLED VCC ─────────┤ 3V3            │
  OLED GND ─────────┤ GND            │
                    │                │
  Button ───────────┤ GPIO3 ────┬─── GND
                    └────────────────┘
                       (internal pull-up)

  4.7 kΩ resistor between DS18B20 DATA and 3V3
```

---

## Build & Upload

```bash
# Build
pio run

# Upload (hold BOOT while pressing RESET if needed)
pio run -t upload

# Serial monitor
pio device monitor -b 115200
```

---

## Implementation Status

| Step | Description | Status |
|------|-------------|--------|
| 1 | Pin map in config header | ✅ Done |
| 2 | DS18B20 read + serial log | ✅ Done |
| 3 | Adaptive sampling policy | ✅ Done |
| 4 | Deep sleep + wake sources | ✅ Done |
| 5 | OLED display integration | ✅ Done |
| 6 | Button wake + display flow | ✅ Done |
| 7 | Wi-Fi + telemetry | ⏳ Planned |
| 8 | Fault handling | ⏳ Planned |
| 9 | NVS config overrides | ⏳ Planned |
| 10 | Bench validation | ⏳ Planned |
| 11 | Notification stubs | ⏳ Planned |

---

## Original Goals

- Measure temperature of boiler by DS18B20 sensor.
- Readings should be available to household, potentially outside home network.
- Boiler is idle most of the day (temp < 30 °C) → low-frequency reads.
- When boiler is on (temp rises) → increase read frequency.
- Future notification system for high-temperature alerts.
- Board powered by small battery → power-efficient, deep sleep.
- SSD1306 0.96" I2C OLED + button: display only on when button pressed.
- On button press: wake, read, display for 10 s, then sleep.
