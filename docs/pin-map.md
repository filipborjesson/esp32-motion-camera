# Pin Map

This document tracks GPIO assignments for all boards in the motion-triggered camera system.

---

# M5StickCPlus2

| Pin | Function | Connected To | Notes |
|----|----|----|----|
| GPIO26 | PIR input | Schmitt trigger output | Motion detection signal |
| GPIO32 | Camera trigger output | ESP32-CAM trigger pin | Sends capture pulse |
| GND | Ground | Breadboard ground rail | Shared system ground |
| 3.3V | Power | Breadboard power rail | Powers logic circuits |

---

# ESP32-CAM (AI Thinker)

| Pin | Function | Connected To | Notes |
|----|----|----|----|
| GPIO13 | Camera trigger input | M5Stick trigger output | Used to start capture |
| 5V | Power | Power rail | Camera board supply |
| GND | Ground | Breadboard ground rail | Common ground |

---

# PIR Sensor (HC-SR501)

| Pin | Function | Connected To | Notes |
|----|----|----|----|
| VCC | Power | 3.3V rail | Sensor power |
| GND | Ground | Ground rail | Common reference |
| OUT | Motion output | Schmitt trigger input | Raw sensor signal |

---

# Schmitt Trigger (74HC14)

| Pin | Function | Connected To | Notes |
|----|----|----|----|
| Pin 14 | VCC | 3.3V rail | Chip power |
| Pin 7 | GND | Ground rail | Chip ground |
| Pin 1 | Input | PIR OUT | Raw sensor signal |
| Pin 2 | Output | M5Stick GPIO26 | Clean digital signal |

---

# Power Distribution

| Source | Destination |
|----|----|
| USB power module | Breadboard rails |
| 3.3V rail | PIR sensor, Schmitt trigger |
| Ground rail | All components |

---

# Notes

- All devices share a common ground.
- Schmitt trigger cleans the PIR signal before the controller reads it.
- ESP32-CAM receives a digital trigger pulse to capture images.