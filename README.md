# Boiler Temperature Monitor – ESP32-C3

Battery-powered ESP32-C3 SuperMini that monitors boiler temperature using a DS18B20 sensor, with adaptive sampling, deep sleep power management, and push notifications via ntfy.sh.

## Features

- **Adaptive Sampling**: Automatically adjusts reading frequency based on temperature
  - Idle (< 30°C): reads every 60 seconds
  - Active (≥ 60°C): reads every 5 seconds
- **Ultra-Low Power**: Deep sleep between readings (< 50 µA idle current)
- **On-Demand Display**: SSD1306 OLED activates on button press, shows temp for 10s
- **Push Notifications**: Alerts via ntfy.sh when temperature exceeds threshold
- **Fault Handling**: Automatic retry logic for sensor errors and Wi-Fi failures

## Hardware Requirements

| Component | Specification | Connection |
|-----------|---------------|------------|
| MCU | ESP32-C3 SuperMini | – |
| Temperature Sensor | DS18B20 (OneWire) | GPIO4 + 4.7kΩ pull-up to 3.3V |
| Display | SSD1306 128×64 I2C OLED | SDA=GPIO8, SCL=GPIO9 |
| Button | Momentary push button (NO) | GPIO3 to GND (internal pull-up) |
| Power | 3.3V battery | – |

See [docs/requirements.md](docs/requirements.md) for complete hardware details and wiring diagram.

## Quick Start

### 1. Setup Secrets (First Time Only)

⚠️ **IMPORTANT**: Never commit credentials to version control!

```bash
# Copy the template file
cp include/secrets_copy.h include/secrets.h

# Edit with your actual credentials
# Use your favorite editor to update:
#   - WIFI_SSID: Your 2.4 GHz Wi-Fi network name
#   - WIFI_PASSWORD: Your Wi-Fi password  
#   - NTFY_TOPIC: A random string (e.g., "boiler-temp-x7k9m2a")
```

The `secrets.h` file is automatically ignored by git and will never be committed.

### 2. Install PlatformIO

If you haven't already, install PlatformIO:
```bash
# Using pip
pip install platformio

# Or via VSCode extension
# Install "PlatformIO IDE" from the VSCode marketplace
```

### 3. Build and Upload

```bash
# Build the firmware
pio run

# Upload to ESP32-C3 (hold BOOT button while pressing RESET if needed)
pio run -t upload

# Monitor serial output
pio device monitor -b 115200
```

## Project Structure

```
.
├── include/              # Header files
│   ├── config.h         # Hardware pins and timing constants
│   ├── secrets_copy.h   # Template for credentials (COPY to secrets.h)
│   ├── connectivity.h   # Wi-Fi and ntfy.sh notification logic
│   ├── display.h        # OLED display driver
│   ├── power.h          # Deep sleep and wake management
│   └── sampling.h       # Adaptive sampling policy
├── src/
│   └── main.cpp         # Main application logic
├── docs/
│   ├── requirements.md  # Detailed specifications
│   └── validation.md    # Testing and troubleshooting guide
├── platformio.ini       # PlatformIO configuration
└── .gitignore          # Excludes secrets.h and sensitive files
```

## Configuration

All hardware pins and timing parameters are centralized in `include/config.h`:

| Parameter | Default | Description |
|-----------|---------|-------------|
| `TEMP_LOW_THRESHOLD` | 30.0°C | Below this = idle mode (slow sampling) |
| `TEMP_HIGH_THRESHOLD` | 60.0°C | Above this = active mode (fast sampling) |
| `TEMP_ALERT_THRESHOLD` | 80.0°C | Send high-temperature alert notification |
| `SAMPLE_INTERVAL_IDLE_S` | 60s | Polling interval when boiler is idle |
| `SAMPLE_INTERVAL_ACTIVE_S` | 5s | Polling interval when boiler is active |
| `DISPLAY_ON_MS` | 10000ms | OLED on-time after button press |

## Notifications (ntfy.sh)

This project uses [ntfy.sh](https://ntfy.sh) for push notifications – no account required!

1. Choose a random topic name (e.g., `boiler-temp-x7k9m2a`)
2. Add it to your `secrets.h` as `NTFY_TOPIC`
3. Subscribe to the topic in the ntfy app:
   - **Android/iOS**: Install ntfy app, subscribe to your topic
   - **Web**: Visit `https://ntfy.sh/your-topic-name`

The device will send notifications when:
- Temperature exceeds `TEMP_ALERT_THRESHOLD` (80°C by default)
- Sensor fails 3 consecutive times
- (Optional) On every reading if `NOTIFY_ON_EACH_READ` is enabled in config.h

## Development

### Debug Mode

For bench testing with shorter intervals:

```bash
# Build and upload debug build
pio run -e esp32-c3-supermini-debug -t upload

# Monitor output
pio device monitor -b 115200
```

Debug mode changes:
- Sampling interval: 10s (instead of 60s idle)
- Display on-time: 5s (instead of 10s)
- Additional serial logging

### Testing

See [docs/validation.md](docs/validation.md) for complete testing procedures.

## Security

### Secrets Management

- **secrets.h** contains Wi-Fi credentials and ntfy topic – **NEVER commit this file**
- Use the provided `secrets_copy.h` template with placeholder values
- `.gitignore` excludes `secrets.h`, `.env*`, and `platformio_override.ini`
- No credentials should ever appear in source code or version control

### What is gitignored?

The following files are automatically excluded from git:
- `include/secrets.h` – Your actual credentials
- `platformio_override.ini` – Per-developer build settings
- `.env` and `.env.*` – Environment files
- `*.bak`, `*~`, `*.swp` – Backup and temp files
- `.pio/` – PlatformIO build artifacts

## Troubleshooting

### "secrets.h: No such file or directory"

You need to create your secrets file:
```bash
cp include/secrets_copy.h include/secrets.h
# Then edit include/secrets.h with your actual credentials
```

### Wi-Fi Connection Timeout

- Verify SSID and password in `secrets.h`
- Ensure you're using a 2.4 GHz network (ESP32-C3 doesn't support 5 GHz)
- Check signal strength at the installation location

### No ntfy Notifications

- Verify topic name matches in `secrets.h` and ntfy app
- Check Wi-Fi connection is successful in serial logs
- Test the topic manually: `curl -d "Test" https://ntfy.sh/your-topic-name`

### Display Not Working

- Verify I2C address (default 0x3C, some displays use 0x3D)
- Check SDA/SCL connections and pull-up resistors
- Test button press – display should activate

For more detailed troubleshooting, see [docs/validation.md](docs/validation.md).

## License

This project is provided as-is for educational and personal use.

## References

- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP32-C3 Datasheet](https://www.espressif.com/en/products/socs/esp32-c3)
- [DS18B20 Datasheet](https://www.analog.com/en/products/ds18b20.html)
- [ntfy.sh Documentation](https://docs.ntfy.sh/)
