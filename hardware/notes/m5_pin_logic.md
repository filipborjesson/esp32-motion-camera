# M5StickS3: Pin Logic and System Role

This document explains the hardware interaction between the M5StickS3 and the external PIR sensor plus ESP32-CAM.

## Essential Pin Map

| Pin Label | Function | Role in Project |
|:---|:---|:---|
| 5V / EXT_5V | Power output | Powers the PIR and ESP32-CAM through the breadboard 5V rail. |
| GND | Ground | Common return path for all components. |
| G5 | Digital input | PIR trigger input. Goes high when motion is detected. |
| G7 | UART TX | Sends the trigger command to the ESP32-CAM. |
| G8 | UART RX | Receives optional status from the ESP32-CAM. |

## How the Pins Work in This Circuit

### 1. Power Distribution

The M5StickS3 can provide external 5V through the Grove/Hat power rail, but M5Unified disables that output by default during initialization. The controller firmware must call:

```cpp
M5.Power.setExtOutput(true);
```

Without that call, the PIR and ESP32-CAM may not receive power from the StickS3 external 5V rail.

When `EXT_5V` is configured as output, power the StickS3 from USB-C or `5VIN`. Do not feed external 5V back into `EXT_5V` or the Grove 5V pin while output mode is enabled.

### 2. Motion Trigger on G5

The PIR sensor output connects to G5 on the Hat2 bus. The firmware configures this pin as `INPUT_PULLDOWN` and watches for a rising edge. The HC-SR501 output is treated as normal logic: low means idle, high means motion.

### 3. Camera UART on G7/G8

The StickS3 Hat2 header gives us enough exposed GPIO to keep the full security-camera wiring on the top connector. This project uses G7 and G8 for the camera UART.

Use this crossover wiring:

| M5StickS3 | ESP32-CAM | Purpose |
|:---|:---|:---|
| G7 / TX | U0R / GPIO3 | M5StickS3 sends trigger commands. |
| G8 / RX | U0T / GPIO1 | ESP32-CAM can send status back. |

The firmware uses `Serial1` for this camera link so USB `Serial` can remain available for debugging.

## Hardware Notes

All data signals are 3.3V logic. Keep a shared ground between the M5StickS3, PIR sensor, and ESP32-CAM, or the UART and PIR signals will be unreliable.

Avoid reusing the StickS3 internal hardware pins for external project signals. The LCD, audio codec, IMU, power management chip, buttons, and IR hardware already consume many GPIOs.

The Hat2 labels read:

```text
G5  G4  G6  G7  G43  G44  G2  G3
GND EXT_5V G0  G1  G8   BAT  3V3 5VIN
```

The current assignment uses `G5`, `G7`, `G8`, `GND`, and `EXT_5V`. Avoid `G6` for this project because it is adjacent to the Boot signal in the Hat2 map, and avoid `G43/G44` unless you have confirmed they are not needed by your upload/debug path.
