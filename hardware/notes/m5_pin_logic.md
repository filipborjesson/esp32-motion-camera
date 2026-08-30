# M5StickS3: Pin Logic and System Role

This document explains the hardware interaction between the M5StickS3 and the external PIR sensor plus ESP32-CAM.

## Essential Pin Map

| Pin Label | Function | Role in Project |
|:---|:---|:---|
| 5V / EXT_5V | Power output | Powers the PIR and ESP32-CAM through the breadboard 5V rail. |
| GND | Ground | Common return path for all components. |
| G8 | UART RX | Receives ESP32-CAM status output through the installed resistor. |
| G7 | UART TX | Reserved for the later command path to the ESP32-CAM. |

## How the Pins Work in This Circuit

### 1. Power Distribution

The M5StickS3 can provide external 5V through the Grove/Hat power rail, but M5Unified disables that output by default during initialization. The controller firmware must call:

```cpp
M5.Power.setExtOutput(true);
```

Without that call, external devices may not receive power from the StickS3 external 5V rail.

When `EXT_5V` is configured as output, power the StickS3 from USB-C or `5VIN`. Do not feed external 5V back into `EXT_5V` or the Grove 5V pin while output mode is enabled.

### 2. Current Test: Camera-Owned PIR

In the current integration test, the PIR sensor output connects directly to ESP32-CAM GPIO13. The ESP32-CAM owns motion detection, camera capture, and optional image upload. The StickS3 listens for ESP32-CAM status lines and turns those into local alarm behavior.

### 3. Camera UART on G8/G7

The StickS3 Hat2 header gives us enough exposed GPIO for a UART status link. The current test uses G8 as StickS3 RX.

Use this crossover wiring:

| M5StickS3 | ESP32-CAM | Purpose |
|:---|:---|:---|
| G8 / RX | U0T / GPIO1 | ESP32-CAM sends status/log lines through the installed resistor. |
| G7 / TX | U0R / GPIO3 | Optional future command path from StickS3 to ESP32-CAM. |

The StickS3 firmware uses `Serial1` for this camera link so USB `Serial` can remain available for debugging. The current baud rate is `9600`.

## Hardware Notes

All data signals are 3.3V logic. Keep a shared ground between the M5StickS3, PIR sensor, and ESP32-CAM, or the UART and PIR signals will be unreliable.

Avoid reusing the StickS3 internal hardware pins for external project signals. The LCD, audio codec, IMU, power management chip, buttons, and IR hardware already consume many GPIOs.

The Hat2 labels read:

```text
G5  G4  G6  G7  G43  G44  G2  G3
GND EXT_5V G0  G1  G8   BAT  3V3 5VIN
```

The current integration test uses `G8` and `GND` for ESP32-CAM-to-StickS3 status. `G7` is reserved for a future command path, and `G5` is reserved for a later version where the StickS3 owns the PIR input.
