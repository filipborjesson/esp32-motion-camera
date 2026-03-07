# System Overview

## Project Title
ESP32 Motion-Triggered Camera System

## Project Goal
Build a motion-triggered embedded camera system using a PIR sensor, signal conditioning, a controller, and an ESP32-CAM module. The system should detect motion, generate a clean trigger signal, command the camera to capture an image, and store the image locally on a microSD card. Later versions may transmit the image over WiFi to a computer.

## Minimum Viable System
The base version of the project will do the following:

1. Detect motion with a PIR sensor
2. Clean the PIR output using a Schmitt trigger
3. Send the cleaned trigger to the controller
4. Have the controller command the ESP32-CAM to capture an image
5. Save the image to the microSD card on the ESP32-CAM

If all five steps work reliably, the base version is complete.

## High-Level Architecture

```text
USB Power
   ↓
Breadboard Power Rails
   ↓
PIR Sensor → Schmitt Trigger → M5StickCPlus2 Controller → ESP32-CAM → microSD Card