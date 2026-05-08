# Schematics

This folder contains hardware reference schematics for the motion-camera build.

## Current Status

| File | Status | Notes |
|:---|:---|:---|
| `espcamSchematic.pdf` | Keep | ESP32-CAM reference schematic. |
| `PIRschematic.pdf` | Keep | HC-SR501 PIR reference schematic. |
| `M5Schematic.pdf` | Replace | This PDF appears to reference an older ESP32-PICO-V3 based M5Stick, not the StickS3 ESP32-S3-PICO-1-N8R8 board. Replace it with the official StickS3 schematic PDF before using this folder as the hardware source of truth. |

## StickS3 Project Pins

Use the StickS3 Hat2 header for this project:

```text
G5  G4  G6  G7  G43  G44  G2  G3
GND EXT_5V G0  G1  G8   BAT  3V3 5VIN
```

Assigned pins:

| StickS3 Pin | Project Signal |
|:---|:---|
| G5 | PIR signal input |
| G7 | UART TX to ESP32-CAM U0R / GPIO3 |
| G8 | UART RX from ESP32-CAM U0T / GPIO1 |
| EXT_5V | 5V rail output |
| GND | Shared ground |
