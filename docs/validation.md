# Bench Validation Guide

This document provides step-by-step testing procedures to validate firmware
functionality before deployment.

---

## Prerequisites

1. ESP32-C3 SuperMini connected via USB
2. DS18B20 sensor wired to GPIO4 with 4.7kΩ pull-up
3. SSD1306 OLED wired to GPIO8 (SDA) / GPIO9 (SCL)
4. Button wired between GPIO3 and GND
5. `secrets.h` configured with valid Wi-Fi credentials and ntfy topic
6. ntfy app installed and subscribed to your topic

---

## Build Modes

Two build environments are configured in `platformio.ini`:

| Environment | Command | Interval |
|-------------|---------|----------|
| Production | `pio run -e esp32-c3-supermini -t upload` | 5–60s adaptive |
| Debug/Test | `pio run -e esp32-c3-supermini-debug -t upload` | 10s fixed |

### Quick commands

```bash
# Upload production firmware
pio run -e esp32-c3-supermini -t upload

# Upload debug firmware (recommended for bench testing)
pio run -e esp32-c3-supermini-debug -t upload

# Monitor serial output
pio device monitor -b 115200
```

Debug mode prints `[DEBUG MODE ENABLED]` on boot and uses shorter intervals
for faster testing cycles.

---

## Test Procedures

### Test 1: Basic Boot & Serial Output

**Objective:** Verify USB-CDC serial output works.

**Steps:**
1. Upload firmware
2. Open serial monitor: `pio device monitor -b 115200`
3. Press RESET button on board

**Expected:**
```
=== Boiler Temperature Monitor ===
Wake reason: POWER_ON
DS18B20 sensors found: 1
Temp: XX.XX C  [IDLE]  sleep for 60 s
Entering deep sleep for 60 s...
```

**Pass criteria:** Serial output appears within 2 seconds of reset.

---

### Test 2: Temperature Sensor Reading

**Objective:** Verify DS18B20 reads correctly.

**Steps:**
1. With sensor connected, observe serial output
2. Hold sensor between fingers (should rise)
3. Release and observe cooling

**Expected:**
- Room temperature reading (18-25°C typical)
- Temperature rises when held
- No "DEVICE_DISCONNECTED" errors

**Troubleshooting:**
- If "sensors found: 0" → check wiring and 4.7kΩ pull-up resistor
- If readings are -127°C → sensor data line issue

---

### Test 3: Sensor Disconnect Handling

**Objective:** Verify graceful handling of missing sensor.

**Steps:**
1. Disconnect DS18B20 data wire (or whole sensor)
2. Press RESET and observe serial

**Expected:**
```
DS18B20 sensors found: 0
WARNING: No DS18B20 detected – check wiring & pull-up resistor
ERROR: Sensor disconnected – retrying in 10 s
```

**Pass criteria:** No crash, retries every 10 seconds.

---

### Test 4: Button Wake & OLED Display

**Objective:** Verify button wakes from sleep and shows temperature.

**Steps:**
1. Let device enter deep sleep (wait for "Entering deep sleep...")
2. Press the button (GPIO3)
3. Observe OLED and serial output

**Expected:**
- Device wakes immediately
- Serial shows: `Wake reason: BUTTON`
- OLED displays temperature for 10 seconds
- OLED turns off, device returns to sleep

**Pass criteria:** Display shows within 1 second of button press.

---

### Test 5: Deep Sleep & Timer Wake

**Objective:** Verify device sleeps and wakes on schedule.

**Steps:**
1. In DEBUG_MODE, interval is 10 seconds
2. Observe serial output across multiple wake cycles

**Expected:**
```
Entering deep sleep for 10 s...
[10 second pause]
=== Boiler Temperature Monitor ===
Wake reason: TIMER
```

**Pass criteria:** Device wakes automatically after configured interval.

---

### Test 6: Wi-Fi Connection

**Objective:** Verify Wi-Fi connects successfully.

**Steps:**
1. Ensure `secrets.h` has correct SSID/password
2. Temporarily set `NOTIFY_ON_EACH_READ = true` in config.h
3. Upload and observe serial

**Expected:**
```
Wi-Fi 'YourSSID' attempt 1/3...
Wi-Fi connected! IP: 192.168.x.x
```

**Troubleshooting:**
- "timeout" on all attempts → wrong SSID/password
- Check router is 2.4 GHz (ESP32-C3 doesn't support 5 GHz)

---

### Test 7: ntfy.sh Notification

**Objective:** Verify push notifications arrive on phone.

**Steps:**
1. Set `NOTIFY_ON_EACH_READ = true` temporarily
2. Subscribe to your topic in ntfy app
3. Reset device and wait for Wi-Fi + notification

**Expected:**
- Serial shows: `ntfy: Notification sent`
- Phone receives push notification with temperature

**Pass criteria:** Notification arrives within 5 seconds of send.

---

### Test 8: High Temperature Alert

**Objective:** Verify alert triggers at threshold.

**Steps:**
1. Temporarily set `TEMP_ALERT_THRESHOLD = 25.0f` (below room temp)
2. Set `NOTIFY_ON_EACH_READ = false`
3. Upload and observe

**Expected:**
- Device sends notification because temp > threshold
- Notification has 🔥 icon and "Boiler Alert" title

---

### Test 9: Adaptive Sampling

**Objective:** Verify sampling interval changes with temperature.

**Steps:**
1. Observe interval at room temperature (should be 60s in idle mode)
2. Heat sensor (hold between fingers or use warm water in bag)
3. When temp rises above 30°C, interval should decrease

**Expected:**
- Below 30°C: 60 second intervals, mode = IDLE
- Above 30°C: Interval decreases proportionally
- Above 60°C: 5 second intervals, mode = ACTIVE

---

### Test 10: Sleep Current Measurement (Optional)

**Objective:** Verify low power consumption in deep sleep.

**Steps:**
1. Use USB power meter or multimeter in series
2. Disconnect OLED (if possible) to measure MCU-only current
3. Wait for device to enter deep sleep

**Expected:**
- Active (Wi-Fi on): 80-150 mA
- Deep sleep: < 50 µA (may show 0 on basic meters)

---

## Quick Validation Checklist

| Test | Status |
|------|--------|
| [ ] Serial output on boot |
| [ ] Temperature reading accurate |
| [ ] Sensor disconnect handled |
| [ ] Button wakes from sleep |
| [ ] OLED displays temperature |
| [ ] Timer wake works |
| [ ] Wi-Fi connects |
| [ ] ntfy notification received |
| [ ] Alert threshold works |
| [ ] Adaptive sampling verified |

---

## Common Issues

| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| No serial output | USB-CDC not enabled | Check `build_flags` in platformio.ini |
| Upload fails | Not in download mode | Hold BOOT, press RESET, release both |
| "sensors found: 0" | Wiring issue | Check 4.7kΩ pull-up, data wire |
| Wi-Fi timeout | Wrong credentials | Check secrets.h, ensure 2.4 GHz network |
| No ntfy notification | Topic mismatch | Verify topic in secrets.h matches app |
| OLED blank | I2C address wrong | Try 0x3D instead of 0x3C |

---

## Resetting to Production Mode

After testing, remember to:

1. Set `NOTIFY_ON_EACH_READ = false`
2. Set `TEMP_ALERT_THRESHOLD` back to desired value (e.g., 70°C)
3. Remove `-DDEBUG_MODE` from build_flags
4. Rebuild and upload: `pio run -t upload`
