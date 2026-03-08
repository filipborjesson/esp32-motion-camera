# 📷 ESP32-CAM: Pin Logic & System Role

This document explains the hardware configuration for the **AI-Thinker ESP32-CAM** module. Reference: `/hardware/schematics/espcamSchematic.pdf`.

## 📍 Essential Pin Map (Connection to M5Stick/FTDI)

| Pin Label | Schematic ID | Function | Connection |
| :--- | :--- | :--- | :--- |
| **5V** | `5V` | Power Input | Connects to M5Stick **5V OUT** or FTDI **VCC**. |
| **GND** | `GND` | Ground | Common return path for all signals. |
| **U0R** | `GPIO 3` | UART RX | **"Ear":** Receives data from M5Stick G32 (TX). |
| **U0T** | `GPIO 1` | UART TX | **"Mouth":** Sends data to M5Stick G33 (RX). |
| **IO0** | `GPIO 0` | Boot Mode | **Flash Pin:** Must be grounded to GND to upload code. |

---

## ⚡ Why These Pins? (Schematic Breakdown)

### 1. Power Logic: The 5V Pin vs. 3.3V Pin
* **From the Schematic:** Look for component **U2 (AMS1117-3.3)**. 
* **The Reason:** The ESP32-CAM is notoriously power-hungry when the camera is active. The 5V pin goes through this internal regulator, which "cleans" the power. 
* **The Rule:** Always use the **5V pin**. Using the 3.3V pin bypasses the regulator; if the M5Stick's battery dips slightly, the camera sensor will fail and cause "Brownout" resets.

### 2. Communication Logic: U0R (G3) and U0T (G1)
* **From the Schematic:** These are labeled **U0TXD** and **U0RXD**.
* **The Reason:** These are the "Hardware Serial" pins. While the ESP32 can technically move Serial to other pins, using the default G1/G3 is preferable because:
    1. It’s what the **FTDI adapter** uses to talk to the chip.
    2. It keeps other GPIOs free for the MicroSD card.
* **The Crossover:** Remember that M5Stick **TX** goes to Cam **RX** (U0R).

### 3. The "Flash" Pin: IO0 (GPIO 0)
* **From the Schematic:** Notice that **IO0** is connected to the **XCLK** (External Clock) of the Camera.
* **The Conflict:** Because it's tied to the camera's clock, you **cannot** use this pin for a button or sensor in this project. 
* **The Purpose:** Its only job is to be grounded during startup to tell the chip: "Don't run the camera yet, I want to upload new code."

---

## 🚫 Avoid These Pins (Hardware Conflicts)

The ESP32-CAM is very "cramped." Based on the schematic, we avoid these pins:

* **GPIO 4 (Flash LED):** Labeled `DATA1`. It is shared with the SD Card. If we use this for a signal, the bright White LED will flash every time the SD card saves a photo.
* **GPIO 12/13/14/15:** These are all part of the **HSPI (SD Card Slot)**. If we plug anything into these, we lose the ability to save photos to the SD card.
* **GPIO 33 (Red LED):** This is the tiny red LED on the back. It is "Active LOW." We will use this in our code as a "Status Light" to show when the camera is busy.

---

## 🛠️ Programming Checklist (For tomorrow's FTDI)
1. **Bridge:** Connect **IO0** to **GND** with a jumper wire.
2. **Power:** Connect FTDI **VCC** to Cam **5V** and **GND** to **GND**.
3. **Data:** Connect FTDI **TX** to Cam **U0R** and FTDI **RX** to Cam **U0T**.
4. **Action:** Press the small **RESET** button on the back of the Cam before clicking "Upload" in VSCode.