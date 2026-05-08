# ESP32-CAM: Pin Logic and System Role

This document explains the hardware configuration for the AI-Thinker ESP32-CAM module.

## Essential Pin Map

| Pin Label | Function | Connection |
|:---|:---|:---|
| 5V | Power input | Connects to the shared 5V rail from the M5StickS3 external 5V output. |
| GND | Ground | Common return path for all signals. |
| U0R / GPIO3 | UART RX | Receives data from M5StickS3 G7 / TX. |
| U0T / GPIO1 | UART TX | Sends optional status to M5StickS3 G8 / RX. |
| IO0 / GPIO0 | Boot mode | Must be grounded only while uploading code. |

## Power Logic: 5V Pin vs. 3.3V Pin

Use the ESP32-CAM 5V pin. The camera module draws high current spikes during Wi-Fi and camera activity, and the onboard regulator is the safer path for this project. Avoid powering the ESP32-CAM through its 3.3V pin.

## UART Logic

Use crossover wiring:

| M5StickS3 | ESP32-CAM |
|:---|:---|
| G7 / TX | U0R / GPIO3 |
| G8 / RX | U0T / GPIO1 |

The controller sketch uses `Serial1` for this link, leaving USB `Serial` free for logs.

## Flash Pin: IO0 / GPIO0

Ground IO0 only when uploading firmware. Remove the jumper and reset the ESP32-CAM for normal boot. Do not use IO0 for project sensors or buttons.

## Avoid These Pins

The ESP32-CAM has a cramped pin map. Avoid these for external signals:

| Pin | Reason |
|:---|:---|
| GPIO4 | Flash LED and shared SD-card line. |
| GPIO12, GPIO13, GPIO14, GPIO15 | Used by the microSD slot. |
| GPIO33 | Onboard red status LED, active low. |

## Programming Checklist

1. Bridge ESP32-CAM IO0 to GND.
2. Connect 5V and GND.
3. Connect FTDI TX to ESP32-CAM U0R and FTDI RX to ESP32-CAM U0T.
4. Press RESET on the ESP32-CAM before uploading.
5. Remove the IO0-to-GND bridge and reset again for normal boot.
