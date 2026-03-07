# 📍 System Pin Map: Motion-Triggered Camera (Direct Connection)

This document tracks the physical wiring and logic assignments for the M5StickC Plus2 and ESP32-CAM. This configuration bypasses the Schmitt trigger for a direct PIR-to-controller setup.

---

## ⚡ Power Distribution (The "Why" of the Rails)

| Rail | Source | Used By | Why? |
|:---|:---|:---|:---|
| **5V Rail** | M5Stick (5V/VIN Pin) | ESP32-CAM (5V), HC-SR501 | The ESP32-CAM and HC-SR501 both have internal 3.3V regulators but require a 5V "headroom" to operate stably. |
| **GND Rail** | M5Stick (GND) | All Components | **Shared Ground:** Without a common ground, the UART signals will have no reference and the data will be corrupted. |

---

## 🏗️ M5StickC Plus2 (Master Controller)

| Pin | Function | Connected To | Notes |
|:---|:---|:---|:---|
| **G26** | PIR Signal In | HC-SR501 (OUT) | **Direct Input:** High (3.3V) = Motion. Since the Schmitt trigger is removed, the logic is "Normal" (not inverted). |
| **G32** | UART TX | ESP32-CAM (U0R) | **Command Line:** Sends the serial trigger to the camera. |
| **G33** | UART RX | ESP32-CAM (U0T) | **Feedback Line:** Receives status updates or image confirmation. |
| **5V / VIN** | Power Out | 5V Breadboard Rail | Passes through the USB power from the M5Stick's internal battery/USB port. |
| **GND** | Ground | Ground Rail | System-wide common ground. |

---

## 📸 ESP32-CAM (AI-Thinker / OV2640)

| Pin | Function | Connected To | Notes |
|:---|:---|:---|:---|
| **5V** | Power In | 5V Breadboard Rail | **Critical:** Do NOT use the 3.3V pin. The camera module draws high current spikes during Wi-Fi bursts that 3.3V rails often can't handle. |
| **GND** | Ground | Ground Rail | System-wide common ground. |
| **GPIO 3 (U0R)**| UART RX | M5Stick TX (G32) | Listens for the "Take Photo" command. |
| **GPIO 1 (U0T)**| UART TX | M5Stick RX (G33) | Reports back to the M5Stick (useful for debugging). |
| **GPIO 0** | Flash Mode | GND (
