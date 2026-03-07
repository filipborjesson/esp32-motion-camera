# Pin Map

This document tracks GPIO assignments and electrical connections for the motion-triggered camera system.

The goal of this file is to prevent pin conflicts and document how signals move between hardware components.

---

# System Components

- M5StickCPlus2 (controller)
- ESP32-CAM (AI Thinker with OV2640 camera)
- PIR Motion Sensor (HC-SR501)
- Schmitt Trigger Inverter (74HC14)
- Breadboard Power Rails

---

# M5StickCPlus2

| Pin | Function | Connected To | Notes |
|----|----|----|----|
| GPIO26 | Motion input | Schmitt trigger output | Reads cleaned PIR signal |
| GPIO32 | UART TX | ESP32-CAM GPIO3 (U0R) | Sends capture command |
| GPIO33 | UART RX (optional) | ESP32-CAM GPIO1 (U0T) | Receives debug/status |
| GND | Ground | Breadboard ground rail | Shared ground reference |
| 3.3V | Power | Breadboard power rail | Powers logic components |

---

# ESP32-CAM (AI Thinker)

| Pin | Function | Connected To | Notes |
|----|----|----|----|
| GPIO3 (U0R) | UART RX | M5Stick TX (GPIO32) | Receives capture command |
| GPIO1 (U0T) | UART TX | M5Stick RX (GPIO33) | Optional status/debug |
| 5V | Power | Breadboard power rail | Preferred board supply |
| GND | Ground | Breadboard ground rail | Common ground |

---

## Important Pin Restriction

Originally the design considered using **GPIO13 on the ESP32-CAM as a trigger input**.

However this pin **cannot be safely used** when the microSD card is active.

### Why GPIO13 cannot be used

On the AI-Thinker ESP32-CAM board:

- GPIO12
- GPIO13
- GPIO14
- GPIO15
- GPIO2

are used by the **microSD card interface**.

If we connect external trigger logic to GPIO13 while using the SD card, we risk:

- SD card malfunction
- unstable boot behavior
- data corruption

### Solution

Instead of using a GPIO trigger line, the **M5StickCPlus2 will send a UART command** to the ESP32-CAM.

Communication path:
