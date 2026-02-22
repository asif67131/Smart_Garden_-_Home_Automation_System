# 🌿 Smart Garden & Home Automation System

A robust, non-blocking automation system built for **ESP8266 (NodeMCU)**. This project manages a 4-channel relay for smart home lighting and an automated irrigation system with safety protocols and Blynk IoT integration.

---

## 📑 Table of Contents
1. [Key Features](#key-features)
2. [Software Prerequisites](#software-prerequisites)
3. [System Architecture](#system-architecture)
4. [Wiring Summary](#wiring-summary)
5. [Blynk App Structure](#blynk-app-structure)
6. [Operating Instructions](#operating-instructions)
7. [Installation](#installation)
8. [Critical Troubleshooting](#critical-troubleshooting)
9. [Product Gallery](#product-gallery)
10. [Demonstration Video](#demonstration-video)
11. [Future Roadmap](#future-roadmap)
12. [Source Code](#source-code)
13. [License](#license)

---

## 🚀
## Key Features

* **Intelligent Irrigation:** Reads soil moisture every 5 minutes and provides precise water bursts.
* **Safety Lockout:** Automatically locks the pump if moisture doesn't rise after 3 attempts (prevents flooding/pump burnout).
* **Non-Blocking Logic:** Responsive LEDs and buttons even during pump/soak cycles using `millis()`.
* **Dual-Control Sync:** Physical buttons (via PCF8574) and Blynk app buttons stay perfectly synchronized.
* **Adjustable Flow:** Real-time adjustment of pump duration (0.5s to 10s) via a Blynk slider.
* **Hardware Stability:** Relays moved to direct GPIOs to eliminate I2C interference.

---

## 💻
## Software Prerequisites

To compile this project, you will need the **Arduino IDE** and the following libraries:

* **Blynk** (by Volodymyr Shymanskyy): For IoT connectivity.
* **ESP8266 Board Package**: Add to your IDE via Preferences.
* **Wire**: (Built-in) For I2C communication with the PCF8574.

---

## 🗺️
## System Architecture

The system operates on a **Non-Blocking State Machine**. This ensures that the user can toggle lights or the pump manually at any time without waiting for the automation timers to finish.



### Irrigation Logic Flow:
1. **Idle State**: Wait for 5-minute timer or Manual Override.
2. **Check Phase**: Power up sensor via **D5** and read moisture.
3. **Action Phase**: If moisture < 30%, trigger pump for `pumpDuration`.
4. **Soak Phase**: Wait 30 seconds for water to reach the roots.
5. **Verification Phase**: Re-read moisture. If no increase, add a "Strike."
6. **Safety Phase**: If 3 strikes occur, enter **LOCK** mode and sound alarm.

---

## 🔌
## Wiring Summary

### 1. NodeMCU (ESP8266) - High Load / Data
| Pin | Component | Description |
| :--- | :--- | :--- |
| **D0** | Relay IN1 | LED 1 Control |
| **D7** | Relay IN2 | LED 2 Control |
| **D8** | Relay IN3 | LED 3 Control (**Note: Unplug for code upload**) |
| **D6** | Relay IN4 | Water Pump Control |
| **D5** | Soil Sensor VCC | Switched Power (prevents corrosion) |
| **A0** | Soil Signal | Analog Moisture Input |
| **D1** | PCF8574 SCL | I2C Clock Line |
| **D2** | PCF8574 SDA | I2C Data Line |



### 2. PCF8574 I/O Expander - Inputs / Feedback
| Pin | Component | Description |
| :--- | :--- | :--- |
| **P0** | Push Button | Physical Manual Pump Toggle (Syncs with V4) |
| **P4** | Push Button | Physical LED 1 Toggle (Syncs with V1) |
| **P5** | Push Button | Physical LED 2 Toggle (Syncs with V2) |
| **P6** | Push Button | Physical LED 3 Toggle (Syncs with V3) |
| **P7** | Buzzer (+) | Audible Feedback/Alarms |

---

## 📱
## Blynk App Structure

| Widget | Virtual Pin | Configuration | Purpose |
| :--- | :--- | :--- | :--- |
| **Gauge** | **V0** | 0 to 100 | Live Soil Moisture % |
| **Button** | **V1 - V3** | Switch Mode | LED Remote Controls |
| **Button** | **V4** | Switch Mode | Manual Pump Override |
| **Slider** | **V5** | Min: 500, Max: 10000, Step: 500 | Adjustable Burst Time |
| **Terminal**| **V10** | Input Line: ON | Live Dashboard & Reset Input |

---

## 🛠️
## Operating Instructions

### 1. Adjusting Water Flow
Use the **V5 Slider** on the app. The system supports 0.5s increments. This determines how long the pump stays on when the soil is dry.

### 2. Manual Override
* **Physical:** Press the button on **P0** to toggle the pump.
* **Remote:** Toggle the **V4** button in the Blynk app.

### 3. Resolving System Lock (3-Strike Rule)
If the pump runs but moisture doesn't increase, the system locks to protect the hardware.
* **To Unlock:** Check your water tank, then type `RESET` in the Blynk Terminal (**V10**).

---

## ⚙️
## Installation

1. **Blynk Setup**: Create a template in the Blynk Console and add Datastreams V0-V10.
2. **Token Config**: Replace `BLYNK_AUTH_TOKEN`, `ssid`, and `pass` in the code.
3. **Hardware Prep**: **IMPORTANT:** Disconnect the wire from **D8** before plugging in USB.
4. **Upload**: Select **NodeMCU 1.0** and hit Upload.
5. **Reconnect**: Reconnect D8 after the upload is finished.

---

## ⚠️
## Critical Troubleshooting

* **Code Upload Fails:** Pin **D8** must be LOW during boot. Disconnect its relay wire during upload.
* **I2C/Button Glitches:** Add a $1000\mu\text{F}$ capacitor across 5V and GND. If buttons are erratic, add $4.7\text{k}\Omega$ pull-up resistors to **D1/D2**.
* **Fluctuating Moisture:** Ensure a **Common Ground** between the Relay power supply and the NodeMCU.

---

## 📸
## Product Gallery

| System Overview | Control Box Interior | Soil Sensor Setup |
| :---: | :---: | :---: |
| ![Main](https://via.placeholder.com/300x200?text=Main+System) | ![Wiring](https://via.placeholder.com/300x200?text=Wiring+Detail) | ![Sensor](https://via.placeholder.com/300x200?text=Soil+Sensor) |

---

## 🎥
## Demonstration Video

[![Demo](https://img.youtube.com/vi/YOUR_VIDEO_ID/0.jpg)](https://www.youtube.com/watch?v=YOUR_VIDEO_ID)

---

## 📈
## Future Roadmap

- [ ] Add an I2C OLED display for local moisture monitoring.
- [ ] Implement Water Reservoir level sensing (Ultrasonic).
- [ ] Design a 3D-printable weather-resistant enclosure.
- [ ] OTA (Over-the-Air) firmware updates.

---

## 💻
## Source Code

The complete firmware for this project is maintained in a separate file for clean version control. You can access the latest stable build below:

### 📂 [Click here to view the Full Code File](code.ino)

> **Note:** Ensure you have your `BLYNK_AUTH_TOKEN`, `WiFi SSID`, and `Password` ready to paste into the code before uploading.

---

## 📜
## License
This project is open-source. Feel free to modify and expand!
