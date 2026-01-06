# IoT Street Light & Water Level Controller 💡💧

A versatile ESP32-based automation project that combines two essential smart home functions into a single controller: intelligent street light scheduling and precise water tank level monitoring.

## 🚀 Features

### 1. Smart Street Light Control
- **RTC-Based Scheduling**: Syncs time once via NTP (Internet) on startup, then operates completely offline using the internal Real-Time Clock (RTC).
- **Automated Timing**: 
  - **ON**: 5:30 PM (17:30)
  - **OFF**: 10:30 PM (22:30)
- **Manual Override**: Physical switch support to manually toggle lights without disrupting the timer logic.
- **Power Saving**: Automatically disconnects WiFi after time synchronization to save energy.

### 2. Water Level Monitor
- **Real-Time Sensing**: Uses an Ultrasonic Sensor (HC-SR04) to measure tank water levels.
- **Visual Feedback**: 3-stage LED indicators for water status:
  - 🔴 **Low**: < 20%
  - 🟡 **Mid**: 50% - 75%
  - 🟢 **High**: > 75%
- **Tank Full Alert**: Activates a specific relay/buzzer when the tank exceeds 90% capacity to prevent overflow.
- **Smart Switch**: Dedicated switch to enable/disable the monitoring system.

## 🛠️ Hardware Requirements

- **Microcontroller**: ESP32 Development Board
- **Sensors**: HC-SR04 Ultrasonic Sensor
- **Actuators**: 
  - 2x Relay Modules (for Light and Buzzer)
  - 5x LEDs (Indicators)
- **Switches**: 2x Toggle/Push Buttons
- **Power Supply**: 5V DC

## 🔌 Pin Configuration

| Component | ESP32 Pin | Description |
| :--- | :--- | :--- |
| **Ultrasonic Trigger** | GPIO 5 | Sensor Trigger |
| **Ultrasonic Echo** | GPIO 18 | Sensor Echo |
| **Street Light Relay** | GPIO 14 | Main Light Control |
| **Tank Full Relay** | GPIO 22 | Buzzer/Alert Output |
| **Manual Switch** | GPIO 15 | Manual Light Control |
| **Monitor Switch** | GPIO 23 | Enable Water Monitoring |
| **LED Error** | GPIO 13 | Time Sync Failure |
| **LED OK** | GPIO 2 | Time Sync Success |
| **LED Low Level** | GPIO 27 | Water Level < 20% |
| **LED Mid Level** | GPIO 26 | Water Level ~ 50-75% |
| **LED High Level** | GPIO 25 | Water Level > 75% |

## ⚙️ Setup & Installation

1. **Install ESP32 Board**: Add ESP32 board support to your Arduino IDE.
2. **Configure WiFi**: 
   Open the `.ino` file and update your credentials:
   ```cpp
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
Calibrate Tank:
Adjust the tank height variables to match your actual tank:

cpp
const float TANK_HEIGHT_CM = 72.0;    // Total depth of tank
const int TANK_MAX_LEVEL_CM = 57;     // Max water fill level
Set Timings:
Modify the schedule if needed:

cpp
const int RELAY_ON_HOUR = 17;  // 5:00 PM
const int RELAY_OFF_HOUR = 22; // 10:00 PM
🧠 How It Works
Startup: The system connects to WiFi, fetches the current time from in.pool.ntp.org, and disconnects from the network.

Loop:

Checks the internal clock every second.

If time matches 17:30, the Street Light turns ON.

If time matches 22:30, the Light turns OFF.

If the Monitor Switch is active, it reads the water level and updates the LEDs.

If water reaches critical levels (>90%), the alarm relay triggers.