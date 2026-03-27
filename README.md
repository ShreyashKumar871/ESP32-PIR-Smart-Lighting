# ESP32 Smart Lighting System with PIR Motion Detection

A time-aware, motion-activated lighting controller with visual feedback and web-based control. Perfect for quick room entries — automatically turns on tubelights when you enter and turns them off when you leave.

![Status](https://img.shields.io/badge/status-active-success.svg)
![Arduino](https://img.shields.io/badge/Arduino-Compatible-00979D?logo=arduino)
![ESP32](https://img.shields.io/badge/ESP32-Supported-E7352C?logo=espressif)

## 🎯 Project Overview

This project solves a common problem: **lighting control for quick room visits**. Instead of manually switching tubelights on/off for brief entries, this system:

- ✅ Automatically detects motion and turns on lights
- ✅ Keeps lights on for 5 seconds after last motion
- ✅ Works only during useful hours (6 PM to 6:45 AM) to save energy
- ✅ Provides manual web control for extended stays
- ✅ Shows status with a mesmerizing LED comet pattern
- ✅ Includes 24/7 override mode for maintenance

**Perfect for:** Storage rooms, garages, pantries, utility areas, or any space with frequent quick visits.

---

## 🌟 Features

### Core Functionality
- **Single PIR sensor** controls **2 relay switches** (for 2 tubelights)
- **Time-gated automation**: PIR triggers only between 18:00–06:45 IST
- **5-second auto-off** after last motion detected
- **Web dashboard** for manual control from any device
- **Override mode** forces lights ON 24/7 (bypasses all automation)

### Visual Feedback
- **Comet LED pattern** with 1 red + 2 blue LEDs
- Smooth tail fade effect shows system is active
- Real-time status indicators on web dashboard

### Smart Controls
- Manual ON/OFF for individual relays
- Master "ALL ON" / "ALL OFF" buttons
- Override toggle for continuous operation
- Live clock display in IST timezone

---

## 📦 Hardware Requirements

### Core Components
| Component | Quantity | Notes |
|-----------|----------|-------|
| ESP32 Development Board | 1 | Any variant (DevKitC, NodeMCU-32S, etc.) |
| PIR Motion Sensor (HC-SR501) | 1 | 3-pin module |
| 2-Channel 5V Relay Module | 1 | Active LOW trigger |
| LEDs | 3 | 1× Red, 2× Blue (5mm) |
| Resistors (220Ω) | 3 | For LED current limiting |
| Jumper Wires | ~15 | Male-to-female recommended |
| Breadboard | 1 | Optional for prototyping |
| 5V Power Supply | 1 | 2A minimum for ESP32 + relays |

### Tubelight Integration
- **2× Tubelights** (AC load)
- Electrical wiring materials (as per local safety codes)
- Junction box / enclosure for relay module

> ⚠️ **Safety Warning**: Relay switching involves AC mains voltage. If unfamiliar with electrical work, consult a licensed electrician for tubelight wiring.

---

## 🔌 Wiring Diagram

### ESP32 Pin Connections

```
ESP32          →  PIR Sensor (HC-SR501)
├─ 3.3V/5V     →  VCC
├─ GND         →  GND
└─ GPIO 14     →  OUT

ESP32          →  2-Channel Relay Module
├─ 5V          →  VCC
├─ GND         →  GND
├─ GPIO 33     →  IN1 (Relay 1)
└─ GPIO 32     →  IN2 (Relay 2)

ESP32          →  LED Indicators
├─ GPIO 25     →  Red LED    (through 220Ω resistor → GND)
├─ GPIO 26     →  Blue LED 1 (through 220Ω resistor → GND)
└─ GPIO 27     →  Blue LED 2 (through 220Ω resistor → GND)
```

### Relay to Tubelight Wiring

```
AC Mains (Phase) → Relay COM terminals
                    ├─ Relay 1 NO → Tubelight 1 Phase
                    └─ Relay 2 NO → Tubelight 2 Phase

AC Mains (Neutral) → Direct to both tubelights
                      (Common neutral connection)
```

**Relay States:**
- `NO` (Normally Open): Used for switching
- `COM` (Common): Connected to AC phase
- When relay activates (LOW signal), COM connects to NO → Light turns ON

---

## 🛠️ Step-by-Step Assembly

### Step 1: Prepare the ESP32
1. Install ESP32 board support in Arduino IDE:
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Tools → Board → Boards Manager → Search "ESP32" → Install

2. Select your board:
   - Tools → Board → ESP32 Arduino → Your board model
   - Tools → Port → Select correct COM port

### Step 2: Wire the PIR Sensor
1. Connect PIR VCC to ESP32 **5V** pin
2. Connect PIR GND to ESP32 **GND**
3. Connect PIR OUT to ESP32 **GPIO 14**
4. Adjust PIR sensitivity (small potentiometers on module):
   - **Sensitivity knob**: Detection range (3-7 meters)
   - **Time delay knob**: Minimum position (we handle timing in code)

### Step 3: Wire the Relay Module
1. Connect Relay VCC to ESP32 **5V**
2. Connect Relay GND to ESP32 **GND**
3. Connect Relay IN1 to ESP32 **GPIO 33**
4. Connect Relay IN2 to ESP32 **GPIO 32**
5. Verify relay jumper is set to **LOW trigger** (common on most modules)

### Step 4: Wire the LED Indicators
For each LED:
1. Longer leg (anode) → GPIO pin
2. Shorter leg (cathode) → 220Ω resistor → GND

Connections:
- Red LED: GPIO 25
- Blue LED 1: GPIO 26
- Blue LED 2: GPIO 27

### Step 5: Upload the Code
1. Open the `.ino` file in Arduino IDE
2. **CRITICAL**: Update WiFi credentials (lines 12-13):
   ```cpp
   const char* ssid     = "YourWiFiName";
   const char* password = "YourPassword";
   ```
3. Click **Upload** (→ button)
4. Open Serial Monitor (115200 baud) to see connection status
5. Note the IP address displayed (e.g., `192.168.1.100`)

### Step 6: Connect Tubelights (AC Wiring)
> ⚠️ **DISCONNECT MAINS POWER BEFORE WIRING**

1. **Relay 1 (Tubelight 1):**
   - Cut the phase wire going to Tubelight 1
   - Connect incoming phase → Relay 1 COM
   - Connect Relay 1 NO → Tubelight 1 phase terminal

2. **Relay 2 (Tubelight 2):**
   - Cut the phase wire going to Tubelight 2
   - Connect incoming phase → Relay 2 COM
   - Connect Relay 2 NO → Tubelight 2 phase terminal

3. Leave **neutral wires** uninterrupted (direct connection)
4. Ensure all connections are secure and insulated
5. Mount relay module in an electrical enclosure

### Step 7: Test the System
1. Power on the ESP32
2. Wait for WiFi connection (check Serial Monitor)
3. Open web browser on your phone/computer
4. Navigate to the IP address shown (e.g., `http://192.168.1.100`)
5. You should see the control dashboard

---

## 🖥️ Web Dashboard Usage

### Dashboard Features

**Time Banner:**
- 🟢 Green: PIR automation active (18:00–06:45 IST)
- 🔴 Red: PIR inactive (06:45–18:00 IST)
- Displays current time in IST

**Individual Relay Control:**
- Each relay shows status: ACTIVE (green) or STANDBY (gray)
- Manual ON/OFF buttons override PIR automation
- Manual state persists until changed

**Master Controls:**
- **ALL ON**: Turns both relays on
- **ALL OFF**: Turns both relays off

**Override Mode:**
- Forces both relays ON continuously
- Bypasses PIR sensor and time gate
- Works 24/7 until manually disabled
- Useful for maintenance or extended work sessions

### Typical Usage Scenarios

**Quick Room Entry (Auto Mode):**
1. Between 6 PM–6:45 AM, walk into the room
2. PIR detects motion → Lights turn ON
3. Leave the room
4. After 5 seconds of no motion → Lights turn OFF

**Extended Stay (Manual Mode):**
1. Open web dashboard on your phone
2. Click "ALL ON"
3. Lights stay on regardless of motion
4. Click "ALL OFF" when leaving

**Continuous Operation (Override Mode):**
1. Enable "⚡ OVERRIDE MODE" on dashboard
2. Both lights stay ON 24/7
3. Disable override to return to normal operation

---

## ⚙️ Configuration Options

### Time Window Adjustment
Modify the `isWithinPirWindow()` function to change active hours:

```cpp
// Current: 18:00–06:45 IST
// Example: Change to 20:00–07:00
bool isWithinPirWindow() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return true;
  int h = timeinfo.tm_hour;
  int m = timeinfo.tm_min;
  if (h >= 20) return true;        // Changed from 18
  if (h < 7)   return true;         // Changed from 6
  if (h == 7 && m <= 0) return true; // Changed from 6:45
  return false;
}
```

### Auto-Off Duration
Change the 5-second delay (line 23):
```cpp
#define RELAY_ON_DURATION 5000  // 5000ms = 5 seconds
// Example: 10 seconds
#define RELAY_ON_DURATION 10000
```

### LED Comet Speed
Adjust the pattern speed (line 41):
```cpp
const int COMET_SPEED = 150; // milliseconds between moves
// Faster: 100ms  |  Slower: 300ms
```

### Timezone
Change the GMT offset (lines 14-15):
```cpp
// IST (UTC+5:30)
const long gmtOffset = 19800;  // seconds

// Example: EST (UTC-5:00)
const long gmtOffset = -18000;
```

---

## 🐛 Troubleshooting

### WiFi Connection Issues
**Symptoms:** ESP32 won't connect, Serial Monitor shows dots indefinitely

**Solutions:**
1. Verify SSID and password are correct (case-sensitive)
2. Ensure router broadcasts on **2.4 GHz** (ESP32 doesn't support 5 GHz)
3. Check router firewall settings (allow new devices)
4. Move ESP32 closer to router during initial setup
5. Try a mobile hotspot to isolate router issues

### Relays Not Switching
**Symptoms:** Clicking sound but no light action

**Solutions:**
1. Verify relay module is powered (5V, sufficient current)
2. Check relay trigger mode (should be LOW-level trigger)
3. Confirm relay wiring: COM → mains, NO → tubelight
4. Test relays manually via web dashboard
5. Use multimeter to check continuity when relay activates

### PIR Not Detecting
**Symptoms:** No response when walking past sensor

**Solutions:**
1. Check PIR wiring (VCC, GND, OUT)
2. Wait 30-60 seconds after power-on (PIR calibration period)
3. Adjust sensitivity potentiometer (clockwise = more sensitive)
4. Verify time is within active window (18:00–06:45)
5. Check Serial Monitor for "PIR triggered" messages

### Time Display Shows `--:--`
**Symptoms:** Clock not syncing

**Solutions:**
1. Verify internet connection (NTP requires internet)
2. Check router firewall (allow NTP on port 123)
3. Wait up to 1 minute for initial sync
4. Try alternative NTP server: `"time.google.com"`
5. System still functions, just no time display

### LEDs Not Lighting
**Symptoms:** No comet pattern visible

**Solutions:**
1. Check LED polarity (long leg = anode → GPIO)
2. Verify resistor values (220Ω recommended)
3. Test individual LEDs with multimeter
4. Confirm GPIO pins 25, 26, 27 not used elsewhere
5. Check connections are secure

---

## 📊 System Behavior Reference

| Scenario | Time Window | PIR State | Manual Control | Relays | Override |
|----------|-------------|-----------|----------------|--------|----------|
| Motion detected | 18:00–06:45 | Active | Not set | ON (5s) | - |
| No motion | 18:00–06:45 | Idle | Not set | OFF | - |
| Motion detected | 06:45–18:00 | Ignored | Not set | OFF | - |
| Any | Any | Any | ON clicked | ON (持续) | - |
| Any | Any | Any | OFF clicked | OFF | - |
| Any | Any | Any | Any | ON | Active |

---

## 🔒 Safety Considerations

1. **AC Voltage Hazards:**
   - Always disconnect mains power before wiring
   - Use proper wire gauge for tubelight current
   - Install relay module in grounded electrical box
   - Never touch exposed terminals when powered

2. **Load Capacity:**
   - Check relay rating (typically 10A @ 250VAC)
   - Calculate total tubelight wattage
   - Example: 2× 40W tubelights = 80W ÷ 230V ≈ 0.35A (safe)

3. **Fire Prevention:**
   - Secure all wire connections (no loose strands)
   - Use heat-shrink tubing or electrical tape
   - Keep ESP32/relay module away from flammable materials
   - Ensure adequate ventilation in enclosure

4. **Electrical Codes:**
   - Follow local electrical codes
   - Consider hiring licensed electrician for AC wiring
   - Use appropriate circuit breaker protection

---

## 🚀 Future Enhancements

### Potential Upgrades
- [ ] Add multiple PIR zones for larger rooms
- [ ] Integrate with Home Assistant / Google Home
- [ ] MQTT support for IoT ecosystems
- [ ] Ambient light sensor (disable automation in daylight)
- [ ] Occupancy tracking (log room usage patterns)
- [ ] OTA (Over-The-Air) firmware updates
- [ ] Battery backup for continued operation during power cuts
- [ ] Mobile app with push notifications

### Advanced Features
- **Machine Learning:** Learn usage patterns and predict when to activate
- **Energy Monitoring:** Track power consumption via ACS712 current sensor
- **Multi-Room System:** ESP-NOW mesh network for synchronized control
- **Voice Control:** Alexa/Google Assistant integration

---

## 📝 License

This project is released under the **MIT License**. You are free to:
- Use commercially
- Modify and distribute
- Use privately

**Attribution appreciated but not required.**

---

## 🤝 Contributing

Contributions welcome! Here's how:

1. Fork this repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

### Areas for Contribution
- Support for additional relay modules (4ch, 8ch)
- Alternative PIR sensor integration (AM312, HC-SR505)
- Web UI themes (dark mode, custom colors)
- Multi-language support
- Documentation improvements

---

## 📧 Support

**Issues?** Open a GitHub issue with:
- ESP32 model and Arduino IDE version
- Serial Monitor output
- Wiring photos (if applicable)
- Steps to reproduce the problem

**Questions?** Start a discussion in the repository.

---

## 🙏 Acknowledgments

- **ESP32 Community** for excellent Arduino support
- **WebServer library** maintainers
- Inspiration from home automation enthusiasts

---

## 📸 Gallery

### Hardware Setup
*Add photos of your assembled system here*

### Web Dashboard Screenshots
*Add dashboard screenshots in different states*

### LED Pattern Demo
*Add video/GIF of comet LED effect*

---

## 📚 Related Resources

- [ESP32 Official Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [Arduino ESP32 Reference](https://github.com/espressif/arduino-esp32)
- [PIR Sensor Datasheet](https://www.mpja.com/download/31227sc.pdf)
- [Home Automation Best Practices](https://www.home-assistant.io/docs/)

---

**Made with ❤️ for smart homes | ESP32 + PIR Motion Detection**

*Star ⭐ this repository if you found it useful!*
