# 📡 PIR Sensor: HiLetgo HC-SR501 Logic

This document details the PIR (Passive Infrared) sensor used as the system trigger. Reference: `/hardware/schematics/pir_sensor_schematic.pdf`.

## 📍 Pinout & Wiring (Looking at the front, pins down)

| Pin | Label | Type | Connection |
| :--- | :--- | :--- | :--- |
| **Left** | **VCC** | Power In | **5V Rail** (M5Stick 5V OUT) |
| **Center** | **OUT** | Signal | **M5Stick G26** (Digital Input) |
| **Right** | **GND** | Ground | **GND Rail** |

---

## ⚡ Technical Logic (From Schematic)

### 1. The BISS0001 IC
* **The Brain:** The schematic shows the **BISS0001** Micro-power PIR Motion Detector IC. 
* **The Signal:** When infrared energy changes (heat moves), the IC triggers a "High" output on the Center Pin.
* **Voltage Safety:** Even though we power the chip with 5V, the internal regulator and the BISS0001 chip output a **3.3V logic signal**. This is perfectly safe for the M5Stick's ESP32.



### 2. The Jumper Settings (Yellow Cap)
On the back of the HiLetgo board, there is a yellow jumper with two positions:
* **L (Isolated Trigger):** The sensor triggers once and stays HIGH for the set time, then goes LOW. It won't trigger again until the time is up.
* **H (Repeatable Trigger):** **[RECOMMENDED]** The sensor stays HIGH as long as there is continuous motion. This is better for our camera so we don't miss action.

### 3. Potentiometer Tuning
* **Sensitivity (Left):** Controls the detection distance (approx. 3m to 7m). 
* **Time Delay (Right):** Controls how long the signal stays HIGH (approx. 5s to 200s). 
    * **Pro-Tip:** Turn this **fully counter-clockwise** to the minimum (5s) so the M5Stick can regain control of the logic as quickly as possible.



---

## 🚦 System Integration
* **G26 Pulldown:** In our code, we will set `pinMode(26, INPUT_PULLDOWN)`. This ensures that when no motion is detected, the pin stays at a solid 0V and doesn't "float" or cause false triggers.