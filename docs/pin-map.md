# System Pin Map: Motion-Triggered Camera (StickS3)

This document tracks the physical wiring and logic assignments for the M5StickS3, PIR sensor, and ESP32-CAM. The current setup uses a direct PIR-to-controller signal and a UART command line from the M5StickS3 to the ESP32-CAM.

## Power Distribution

| Rail | Source | Used By | Notes |
|:---|:---|:---|:---|
| 5V Rail | M5StickS3 external 5V output | ESP32-CAM 5V, HC-SR501 VCC | StickS3 external 5V output is disabled by default after `M5.begin()`. The controller sketch calls `M5.Power.setExtOutput(true)`. |
| GND Rail | M5StickS3 GND | All components | All UART and sensor signals need a shared ground reference. |

When `EXT_5V` is configured as output, power the StickS3 from USB-C or `5VIN`. Do not feed external 5V back into `EXT_5V` or the Grove 5V pin while output mode is enabled.

## M5StickS3 Controller

| Pin | Function | Connected To | Notes |
|:---|:---|:---|:---|
| G5 | PIR signal in | HC-SR501 OUT | Hat2 bus pin. High = motion. Configured as `INPUT_PULLDOWN`. |
| G7 | UART TX | ESP32-CAM U0R / GPIO3 | Hat2 bus pin. Sends the camera trigger. |
| G8 | UART RX | ESP32-CAM U0T / GPIO1 | Hat2 bus pin. Receives optional camera status. |
| EXT_5V | Power out | 5V breadboard rail | Enable in software with `M5.Power.setExtOutput(true)`. |
| GND | Ground | Ground rail | System-wide common ground. |

## M5StickS3 Hat2 Physical Pin Order

The StickS3 Hat2 pins, read from left to right starting at the top as labeled on the board, are:

```text
G5  G4  G6  G7  G43  G44  G2  G3
GND EXT_5V G0  G1  G8   BAT  3V3 5VIN
```

For this project, use `G5`, `G7`, `G8`, `GND`, and `EXT_5V`.

## ESP32-CAM (AI-Thinker / OV2640)

| Pin | Function | Connected To | Notes |
|:---|:---|:---|:---|
| 5V | Power in | 5V breadboard rail | Use 5V, not the 3.3V pin, for camera stability. |
| GND | Ground | Ground rail | System-wide common ground. |
| GPIO3 / U0R | UART RX | M5StickS3 G7 / TX | Receives the trigger command. |
| GPIO1 / U0T | UART TX | M5StickS3 G8 / RX | Optional status/debug response line. |
| GPIO0 | Flash mode | GND only while uploading | Remove the GND jumper for normal boot. |

## PIR Sensor (HC-SR501)

| Pin | Connection | Notes |
|:---|:---|:---|
| VCC | 5V rail | The module accepts 5V input. |
| OUT | M5StickS3 G5 | Outputs a 3.3V logic high on motion. |
| GND | Ground rail | Must share ground with the M5StickS3 and ESP32-CAM. |

## Important Differences From M5StickC Plus2

| Area | M5StickC Plus2 | M5StickS3 |
|:---|:---|:---|
| Arduino library | `M5StickCPlus2.h` | `M5Unified.h` |
| Board target | M5StickC Plus2 / ESP32 | M5StickS3 / ESP32-S3 |
| Grove signal pins | G32, G33 | Grove is G9/G10, but this project uses Hat2 G7/G8 |
| PIR pin used here | G26 | G5 on Hat2 bus |
| Camera UART in code | `Serial` | `Serial1` on Hat2 G7/G8 |
| External 5V behavior | Available from previous wiring assumptions | Must be enabled with `M5.Power.setExtOutput(true)` |
