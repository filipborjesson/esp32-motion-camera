# 🧠 M5StickC Plus2: Pin Logic & System Role

This document explains the hardware interaction between the **M5StickC Plus2** and our external components (PIR Sensor & ESP32-CAM). Reference: `/hardware/schematics/M5StickCPlus2_schematic.pdf`.

## 📍 Essential Pin Map

| Pin Label | Schematic ID | Function | Role in Project |
| :--- | :--- | :--- | :--- |
| **5V OUT** | `+5VOUT` | Power Output | Powers the PIR and the Camera via Breadboard Red Rail. |
| **GND** | `GND` | Ground | Common return path for electricity (Common Ground). |
| **G26** | `GPIO26` | Digital Input | **PIR Trigger:** Goes HIGH when motion is detected. |
| **G32** | `GPIO32` | UART TX | **Command Line:** Sends "Take Photo" signal to Camera. |
| **G33** | `GPIO33` | UART RX | **Response Line:** Listens for Camera confirmation. |

---

## ⚡ How the Pins Work in this Circuit

### 1. Power Distribution (5V & GND)
* **The Flow:** The M5Stick acts as the power station. 5V flows out of **Pin 2** of the header into the breadboard. 
* **The Ground:** **Pin 1** must be connected to the breadboard's black rail. Without this "Common Ground," the data signals on G26/G32 will be "noisy" or won't work at all.



### 2. The Motion Trigger (G26)
* **Internal Logic:** According to the schematic, **GPIO26** is Pin 14 on the ESP32 chip. 
* **Safety:** It has a **PESDNC2FD5VB** protection diode (D4). This allows us to plug in sensors safely without static frying the internal CPU.
* **Function:** When the PIR sensor sees movement, it sends 3.3V to this pin. Our code watches for this "Rising Edge."

### 3. The Camera Highway (G32 & G33)
* **UART Communication:** These pins allow the two "brains" to talk.
* **The Crossover:** We connect M5Stick **G32 (TX)** to the Camera's **RX**, and M5Stick **G33 (RX)** to the Camera's **TX**.



---

## 🛠️ Hardware Notes
* **3.3V Logic:** While we use **5V** to power the devices, all *data signals* (G26, G32, G33) operate at **3.3V**. 
* **Internal Conflict Check:** G26 is "Free"—it is not used by the internal LCD (ST7789V2), the IMU (MPU6886), or the Mic (SPM1423).