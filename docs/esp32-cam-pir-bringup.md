# ESP32-CAM PIR Bring-Up Test

This is the first standalone hardware test for the ESP32-CAM and PIR sensor. It intentionally ignores the M5StickS3 so the motion/camera path can be verified by itself.

## Firmware

Use this Arduino sketch for the main Arduino IDE bring-up:

```text
firmware/esp32-cam/esp32-cam.ino
```

## Wiring

For the AI-Thinker ESP32-CAM:

| PIR Pin | ESP32-CAM Connection | Notes |
|:---|:---|:---|
| VCC | 5V | Use the ESP32-CAM 5V pin, not 3.3V. |
| GND | GND | Shared ground is required. |
| OUT | GPIO13 | Motion signal input. High means motion. |

## Why GPIO13

The AI-Thinker camera itself uses these GPIOs:

```text
PWDN=32, XCLK=0, SIOD=26, SIOC=27
Y9=35, Y8=34, Y7=39, Y6=36
Y5=21, Y4=19, Y3=18, Y2=5
VSYNC=25, HREF=23, PCLK=22
```

GPIO13 is exposed on common AI-Thinker ESP32-CAM boards and is not used by the camera sensor. It is one of the microSD-card lines, so this bring-up test does not use the SD card.

Avoid these for this test:

| Pin | Reason |
|:---|:---|
| GPIO0 | Boot/camera clock path; also used for flashing mode. |
| GPIO1/GPIO3 | USB serial programming/debug UART. |
| GPIO4 | Flash LED and SD-card line. |
| GPIO12 | Boot strapping risk and SD-card line. |
| GPIO14/GPIO15 | SD-card lines; GPIO15 is also a strapping-sensitive pin. |

## Functions Verified

The bring-up sketch verifies:

1. The ESP32-CAM boots and opens serial logs at `115200`.
2. The PIR output is readable on GPIO13.
3. The PIR signal is normally low and goes high on motion.
4. A stable high signal longer than `250 ms` is counted as a trigger.
5. Rapid repeated triggers are ignored during a `5 s` cooldown.
6. The flash LED blinks when a valid event is accepted.
7. The OV2640 camera initializes.
8. A JPEG frame can be captured into memory after motion.
9. Serial logs report trigger count, ignored count, capture count, and captured JPEG byte size.

## Expected Serial Output

On boot:

```text
--- ESP32-CAM PIR Capture Test ---
Wiring: PIR VCC->5V, PIR GND->GND, PIR OUT->GPIO13.
Camera init OK.
PIR warmup for 30 seconds...
```

On motion:

```text
PIR_EDGE: rising edge candidate.
PIR_TRIGGERED: trigger=1 pin=GPIO13
CAPTURE_OK: capture=1 bytes=xxxxx format=4 width=640 height=480
EVENT_DONE: trigger=1 captured=true ignored=0 total_captures=1
```

## Next Step After This Works

The next firmware phase adds Wi-Fi upload from the ESP32-CAM to:

```text
POST /api/device/security/images
```

The upload is implemented in `firmware/esp32-cam/esp32-cam.ino`. Copy
`camera_secrets.h.example` to `camera_secrets.h`, then configure the 2.4 GHz
Wi-Fi credentials, the Flask server's LAN IPv4 address, and the same
`DEVICE_API_KEY` used by the server. The local secrets file is ignored by Git.

This first upload test uses plain HTTP on the local network. Run Flask on a
laptop or Raspberry Pi reachable by the ESP32-CAM; do not use `localhost` as
the camera's server host. If configuration, Wi-Fi, or the server is unavailable,
the firmware continues PIR detection, flash, capture, and UART notification.
Successful uploads report `UPLOAD_OK`; network/server errors report
`UPLOAD_FAIL` or `UPLOAD_SKIPPED`.

That lets the ESP32-CAM do the image work while the M5StickS3 remains the local
assistant/controller. HTTPS and model inference should be added after repeated
local uploads are reliable.
