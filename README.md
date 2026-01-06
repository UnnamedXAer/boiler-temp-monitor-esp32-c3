# Boiler Temperature Monitor

Battery-powered ESP32-C3 SuperMini that monitors boiler temperature using a DS18B20
sensor, with adaptive sampling, deep sleep, and an on-demand OLED display.
Push notifications via [ntfy.sh](https://ntfy.sh).

## Features

- **Adaptive sampling**: 60s intervals when idle, 5s when boiler is active
- **Deep sleep**: Ultra-low power between readings (< 50 µA)
- **Button wake**: Press to show temperature on OLED for 10 seconds
- **Push notifications**: Alerts via ntfy.sh when temperature exceeds threshold
- **Alert cooldown**: Prevents notification spam (10-minute cooldown)

## Hardware

| Component | GPIO | Notes |
|-----------|------|-------|
| DS18B20 sensor | GPIO4 | Requires 4.7kΩ pull-up to 3.3V |
| SSD1306 OLED | SDA=GPIO8, SCL=GPIO9 | 0.91" 128×32 I2C display |
| Wake button | GPIO3 | Connects to GND (internal pull-up) |

## Quick Start

### 1. Clone and configure secrets

```bash
cd include
cp secrets_template.h secrets.h
# Edit secrets.h with your Wi-Fi credentials and ntfy topic
```

### 2. Build and upload

```bash
# Production build (60s idle interval)
pio run -e esp32-c3-supermini -t upload

# Debug build (10s intervals for testing)
pio run -e esp32-c3-supermini-debug -t upload

# Monitor serial output
pio device monitor -b 115200

# If `pio` is not system-wide available you can try this (powershell)
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e esp32-c3-supermini
```

> **Note:** If upload fails, hold BOOT while pressing RESET, then release both.

## Build Environments

| Environment | Purpose | Sample Interval |
|-------------|---------|-----------------|
| `esp32-c3-supermini` | Production | 5–60s (adaptive) |
| `esp32-c3-supermini-debug` | Bench testing | 10s fixed |

### Build commands

```bash
# Build all environments
pio run

# Build specific environment
pio run -e esp32-c3-supermini

# Upload specific environment
pio run -e esp32-c3-supermini-debug -t upload

# Clean build
pio run -t clean
```

## Configuration

All tuning constants are in [`include/config.h`](include/config.h):

| Setting | Default | Description |
|---------|---------|-------------|
| `TEMP_LOW_THRESHOLD` | 30°C | Below = idle mode |
| `TEMP_HIGH_THRESHOLD` | 60°C | Above = active mode |
| `TEMP_ALERT_THRESHOLD` | 80°C | Trigger push notification |
| `SAMPLE_INTERVAL_IDLE_S` | 60s | Idle sampling period |
| `SAMPLE_INTERVAL_ACTIVE_S` | 5s | Active sampling period |
| `ALERT_COOLDOWN_S` | 600s | Min time between alerts |
| `DISPLAY_ON_MS` | 10000ms | OLED on-time after button |

## Project Structure

```
├── include/
│   ├── config.h          # All configuration constants
│   ├── sampling.h        # Adaptive interval logic with hysteresis
│   ├── power.h           # Deep sleep and wake sources
│   ├── display.h         # SSD1306 OLED driver
│   ├── connectivity.h    # Wi-Fi and ntfy.sh notifications
│   ├── secrets.h         # Wi-Fi/ntfy credentials (gitignored)
│   └── secrets_template.h
├── src/
│   └── main.cpp          # Application entry point
├── docs/
│   ├── requirements.md   # Full project requirements
│   └── validation.md     # Bench testing procedures
├── platformio.ini        # Build configuration
└── README.md             # This file
```

## Notifications

### Setup ntfy.sh

1. Install ntfy app ([Android](https://play.google.com/store/apps/details?id=io.heckel.ntfy))
2. Subscribe to your topic (e.g., `boiler-temp-abc123`)
3. Add the same topic to `secrets.h`

No account required—the topic name acts as simple authentication.

### Notification types

| Condition | Priority | Example |
|-----------|----------|---------|
| Normal reading | 3 (default) | 🌡️ Boiler: 45.2 °C |
| High temp alert | 4 (high) | 🔥 High temp: 82.5 °C |
| Sensor error | 5 (urgent) | ⚠️ Sensor disconnected! |

## Documentation

- [Full requirements](docs/requirements.md) – Hardware, functional specs, wiring diagram
- [Validation guide](docs/validation.md) – Step-by-step bench testing procedures

## Troubleshooting

| Issue | Solution |
|-------|----------|
| No serial output | Check `build_flags` include `-DARDUINO_USB_CDC_ON_BOOT=1` |
| Upload fails | Hold BOOT, press RESET, release both, then upload |
| "sensors found: 0" | Check DS18B20 wiring and 4.7kΩ pull-up resistor |
| Wi-Fi timeout | Verify SSID/password in `secrets.h`; must be 2.4 GHz network |
| No ntfy notification | Ensure topic in `secrets.h` matches app subscription |

## License

MIT
