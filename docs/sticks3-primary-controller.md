# StickS3 Primary Controller

The StickS3 is the local assistant/controller for the motion-camera system. In the current test, the ESP32-CAM owns the PIR input and image capture while the StickS3 owns local alarm behavior, counters, button controls, and the UART status display.

## Firmware

Use this Arduino sketch:

```text
firmware/m5stick-controller/m5stick-controller.ino
```

## Wiring

| Signal | StickS3 Pin | Connected To |
|:---|:---|:---|
| Camera UART RX | G7 | Resistor, then ESP32-CAM U0T / GPIO1 |
| Camera UART TX | G8 | Optional ESP32-CAM U0R / GPIO3 command path |
| Power | EXT_5V | 5V rail for PIR and ESP32-CAM |
| Ground | GND | Shared ground rail |

When `EXT_5V` is output mode, power the StickS3 from USB-C or `5VIN`. Do not backfeed external 5V into `EXT_5V`.

## Modes

| Mode | Meaning |
|:---|:---|
| `ARMED` | Waiting for ESP32-CAM status messages. |
| `ALARM` | Motion message received; screen flashes red and high beeps repeat. |
| `COOLDOWN` | Recent accepted event; repeated triggers are ignored. |
| `DISARMED` | PIR is ignored until rearmed. |
| `CAM ERROR` | ESP32-CAM did not respond or reported failure. |

## Buttons

| Button | Action |
|:---|:---|
| BtnA | Acknowledge active alarm or camera error. |
| BtnB | Toggle alarm sound mute. |

## Camera Command Protocol

The current StickS3 firmware listens over `Serial1` at `9600` baud:

```text
ESP32-CAM U0T/GPIO1 -> resistor -> StickS3 G7/RX
```

It reacts to line-based ESP32-CAM status messages such as:

```text
MOTION:<trigger_count>
PIR_TRIGGERED:<trigger_count>
CAPTURE_OK:<trigger_count>
CAPTURE_FAIL:<trigger_count>
UPLOAD_OK:<trigger_count>
UPLOAD_FAIL:<trigger_count>
```

The optional G8/TX to ESP32-CAM U0R/GPIO3 command path is reserved for a later version where the StickS3 tells the camera when to capture.

## What This Version Verifies

1. StickS3 can power the Hat2 `EXT_5V` rail.
2. StickS3 can receive ESP32-CAM serial status on G7.
3. Motion messages from the ESP32-CAM trip the local alarm state.
4. StickS3 displays monitoring/alarm state, sound state, last trip time, and last status.
5. StickS3 flashes its screen red during alarm.
6. StickS3 plays repeating alarm tones unless muted.
7. BtnA acknowledges an alarm.
8. BtnB toggles local alarm sound.
