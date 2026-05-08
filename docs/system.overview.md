# System Overview

## Project Title

ESP32 Motion-Triggered Camera System

## Project Goal

Build a motion-triggered embedded camera system using a PIR sensor, an M5StickS3 controller, and an ESP32-CAM module. The system should detect motion, command the camera to capture an image, and store the image locally on a microSD card. Later versions may transmit the image over Wi-Fi to a computer.

## Minimum Viable System

The base version of the project will do the following:

1. Detect motion with a PIR sensor.
2. Send the PIR trigger to the M5StickS3 controller.
3. Have the controller command the ESP32-CAM to capture an image.
4. Save the image to the microSD card on the ESP32-CAM.

If all four steps work reliably, the base version is complete.

## High-Level Architecture

```text
USB Power
   |
Breadboard Power Rails
   |
PIR Sensor -> M5StickS3 Controller -> ESP32-CAM -> microSD Card
```
