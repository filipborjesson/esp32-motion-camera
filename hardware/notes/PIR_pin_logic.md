# PIR Sensor: HiLetgo HC-SR501 Logic

This document details the PIR sensor used as the system trigger.

## Pinout and Wiring

Looking at the front of the HC-SR501 with pins facing down:

| Pin | Label | Connection |
|:---|:---|:---|
| Left | VCC | Shared 5V rail from the M5StickS3 external 5V output. |
| Center | OUT | M5StickS3 G5. |
| Right | GND | Shared ground rail. |

For the standalone ESP32-CAM bring-up test, ignore the M5StickS3 and wire the PIR directly:

| PIR Pin | ESP32-CAM Connection |
|:---|:---|
| VCC | 5V |
| OUT | GPIO13 |
| GND | GND |

## Signal Logic

When infrared energy changes, the PIR output goes high. The controller sketch configures M5StickS3 G5 as `INPUT_PULLDOWN` and attaches a rising-edge interrupt, so low means idle and high means motion.

The HC-SR501 output is suitable as a 3.3V logic signal for the M5StickS3 input.

## Jumper Settings

| Mode | Behavior | Recommendation |
|:---|:---|:---|
| L | Triggers once, stays high for the set delay, then goes low. | Usable, but can miss continuous motion during the delay window. |
| H | Stays high while motion continues. | Recommended for the camera trigger. |

## Potentiometer Tuning

| Control | Purpose | Starting Point |
|:---|:---|:---|
| Sensitivity | Detection distance. | Start mid-range, then tune for the room. |
| Time delay | How long OUT remains high. | Start near minimum so the controller regains control quickly. |
