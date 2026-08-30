# System Pin Map: Motion-Triggered Camera (StickS3)

This document tracks the physical wiring and logic assignments for the M5StickS3, PIR sensor, and ESP32-CAM. The current integration test uses the ESP32-CAM as the PIR/camera worker and the StickS3 as the local alarm assistant.

## Power Distribution

| Rail | Source | Used By | Notes |
|:---|:---|:---|:---|
| 5V Rail | M5StickS3 external 5V output | ESP32-CAM 5V, HC-SR501 VCC | StickS3 external 5V output is disabled by default after `M5.begin()`. The controller sketch calls `M5.Power.setExtOutput(true)`. |
| GND Rail | M5StickS3 GND | All components | All UART and sensor signals need a shared ground reference. |

When `EXT_5V` is configured as output, power the StickS3 from USB-C or `5VIN`. Do not feed external 5V back into `EXT_5V` or the Grove 5V pin while output mode is enabled.

## M5StickS3 Controller

| Pin | Function | Connected To | Notes |
|:---|:---|:---|:---|
| G8 | UART RX | ESP32-CAM U0T / GPIO1 through resistor | Current test path. Receives ESP32-CAM status logs such as `PIR_TRIGGERED` and `UPLOAD_OK`. |
| G7 | UART TX | Optional ESP32-CAM U0R / GPIO3 | Future command path from StickS3 to camera. Not required for the current PIR-to-camera test. |
| EXT_5V | Power out | 5V breadboard rail | Enable in software with `M5.Power.setExtOutput(true)`. |
| GND | Ground | Ground rail | System-wide common ground. |

## M5StickS3 Hat2 Physical Pin Order

The StickS3 Hat2 pins, read from left to right starting at the top as labeled on the board, are:

```text
G5  G4  G6  G7  G43  G44  G2  G3
GND EXT_5V G0  G1  G8   BAT  3V3 5VIN
```

For the current ESP32-CAM-owned PIR test, use `G8` and `GND` between the ESP32-CAM and StickS3. `G7` is reserved for the later StickS3-to-camera command path.

## ESP32-CAM (AI-Thinker / OV2640)

| Pin | Function | Connected To | Notes |
|:---|:---|:---|:---|
| 5V | Power in | 5V breadboard rail | Use 5V, not the 3.3V pin, for camera stability. |
| GND | Ground | Ground rail | System-wide common ground. |
| GPIO1 / U0T | UART TX | StickS3 G8 / RX through resistor | Sends status/debug output to the StickS3. |
| GPIO3 / U0R | UART RX | Optional StickS3 G7 / TX | Future command input from StickS3. Not required for the current PIR-to-camera test. |
| GPIO0 | Flash mode | GND only while uploading | Remove the GND jumper for normal boot. |

## PIR Sensor (HC-SR501)

| Pin | Connection | Notes |
|:---|:---|:---|
| VCC | 5V rail | The module accepts 5V input. |
| OUT | ESP32-CAM GPIO13 for current test | Outputs a 3.3V logic high on motion. |
| GND | Ground rail | Must share ground with the M5StickS3 and ESP32-CAM. |

## Important Differences From M5StickC Plus2

| Area | M5StickC Plus2 | M5StickS3 |
|:---|:---|:---|
| Arduino library | `M5StickCPlus2.h` | `M5Unified.h` |
| Board target | M5StickC Plus2 / ESP32 | M5StickS3 / ESP32-S3 |
| Grove signal pins | G32, G33 | Grove is G9/G10, but this project uses Hat2 G7/G8 |
| PIR pin used here | G26 | G5 on Hat2 bus |
| Camera UART in code | `Serial` | ESP32-CAM `U0T/GPIO1` to StickS3 `Serial1` RX on G8 at 9600 baud |
| External 5V behavior | Available from previous wiring assumptions | Must be enabled with `M5.Power.setExtOutput(true)` |
