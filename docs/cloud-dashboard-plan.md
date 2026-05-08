# Cloud Dashboard Plan

The persistent deployment should follow the same pattern as the Assignment 5 cloud deployment:

```text
Browser / StickS3 / ESP32-CAM
  -> HTTPS domain
  -> Cloudflare DNS
  -> Google Cloud VM
  -> Caddy
  -> Flask backend on localhost:5001
  -> SQLite + uploads + future TensorFlow inference
```

## Domain

You do not need a new purchased domain. Use a subdomain on the domain you already own, for example:

```text
camera.voiceassistant.stream
```

Cloudflare DNS should point that subdomain to the VM public IP with an `A` record.

## Backend Responsibilities

The Flask backend in `server/app.py` provides:

| Area | Routes |
|:---|:---|
| Browser dashboard | `GET /security` |
| Dashboard data | `GET /api/security/status`, `GET /api/security/events` |
| Browser commands | `POST /api/security/commands`, `POST /api/security/events/<id>/ack` |
| Device events | `POST /api/device/security/events` |
| Device images | `POST /api/device/security/images` |
| Device heartbeat | `POST /api/device/security/heartbeat` |
| Device command polling | `GET /api/device/security/commands?device_id=stick-s3-01` |

## Auth Model

Browser routes use HTTP Basic Auth:

```text
DASHBOARD_USERNAME
DASHBOARD_PASSWORD
```

Use strong VM-only values. Do not keep the example/default passwords in production.

Embedded routes use a device API key:

```text
X-Device-Key: <DEVICE_API_KEY>
```

or:

```text
Authorization: Bearer <DEVICE_API_KEY>
```

## Data Model

SQLite stores event metadata:

```text
event id
timestamp
device id
mode
trigger count
ignored count
sent image count
image path
classification
confidence
acknowledged state
```

Images are stored as files under:

```text
server/uploads/security/
```

## AI Hook

The image endpoint currently calls a `classify_image_stub()` placeholder. Replace that function with TensorFlow or TFLite inference after the image upload pipeline is stable.

Recommended order:

1. Rule-based filtering on StickS3.
2. Cloud image upload.
3. Cloud TensorFlow classification.
4. Local Edge Impulse or TensorFlow Lite Micro comparison.

## VM Deployment Sketch

On the VM:

```bash
cd ~
git clone <repo-url> esp32-motion-camera
cd esp32-motion-camera/server
python3 -m venv .venv
. .venv/bin/activate
pip install -r requirements.txt
```

Install the systemd service from `server/security-camera.service.example`, then configure Caddy using `server/Caddyfile.example`.
