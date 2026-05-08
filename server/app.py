import os
import secrets
import sqlite3
import time
from functools import wraps
from pathlib import Path

from flask import Flask, Response, jsonify, redirect, render_template, request, send_from_directory, url_for
from werkzeug.utils import secure_filename


BASE_DIR = Path(__file__).resolve().parent
INSTANCE_DIR = BASE_DIR / "instance"
UPLOAD_DIR = BASE_DIR / "uploads" / "security"
DB_PATH = INSTANCE_DIR / "security_camera.sqlite3"

DASHBOARD_USERNAME = os.environ.get("DASHBOARD_USERNAME")
DASHBOARD_PASSWORD = os.environ.get("DASHBOARD_PASSWORD")
DEVICE_API_KEY = os.environ.get("DEVICE_API_KEY")

app = Flask(__name__)


def now_ms():
    return int(time.time() * 1000)


def db():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn


def init_db():
    INSTANCE_DIR.mkdir(parents=True, exist_ok=True)
    UPLOAD_DIR.mkdir(parents=True, exist_ok=True)
    with db() as conn:
        conn.executescript(
            """
            CREATE TABLE IF NOT EXISTS events (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                created_ms INTEGER NOT NULL,
                device_id TEXT NOT NULL,
                event_type TEXT NOT NULL,
                mode TEXT,
                message TEXT,
                trigger_count INTEGER DEFAULT 0,
                ignored_count INTEGER DEFAULT 0,
                sent_image_count INTEGER DEFAULT 0,
                battery_percent REAL,
                rssi INTEGER,
                image_path TEXT,
                classification TEXT,
                confidence REAL,
                latency_ms INTEGER,
                acknowledged INTEGER DEFAULT 0,
                acknowledged_ms INTEGER
            );

            CREATE TABLE IF NOT EXISTS heartbeats (
                device_id TEXT PRIMARY KEY,
                updated_ms INTEGER NOT NULL,
                mode TEXT,
                battery_percent REAL,
                rssi INTEGER,
                firmware_version TEXT
            );

            CREATE TABLE IF NOT EXISTS commands (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                created_ms INTEGER NOT NULL,
                device_id TEXT NOT NULL,
                command TEXT NOT NULL,
                payload TEXT,
                consumed INTEGER DEFAULT 0,
                consumed_ms INTEGER
            );
            """
        )


def dashboard_auth_required(fn):
    @wraps(fn)
    def wrapper(*args, **kwargs):
        if not DASHBOARD_USERNAME or not DASHBOARD_PASSWORD:
            return Response("Dashboard auth is not configured", 503)
        auth = request.authorization
        valid = (
            auth
            and secrets.compare_digest(auth.username or "", DASHBOARD_USERNAME)
            and secrets.compare_digest(auth.password or "", DASHBOARD_PASSWORD)
        )
        if not valid:
            return Response(
                "Authentication required",
                401,
                {"WWW-Authenticate": 'Basic realm="Security Camera Dashboard"'},
            )
        return fn(*args, **kwargs)

    return wrapper


def device_auth_required(fn):
    @wraps(fn)
    def wrapper(*args, **kwargs):
        if not DEVICE_API_KEY:
            return jsonify({"error": "device auth is not configured"}), 503
        header_key = request.headers.get("X-Device-Key", "")
        bearer = request.headers.get("Authorization", "")
        token = bearer.removeprefix("Bearer ").strip() if bearer.startswith("Bearer ") else ""
        if not (
            secrets.compare_digest(header_key, DEVICE_API_KEY)
            or secrets.compare_digest(token, DEVICE_API_KEY)
        ):
            return jsonify({"error": "unauthorized"}), 401
        return fn(*args, **kwargs)

    return wrapper


def classify_image_stub(image_path):
    # Replace this with TensorFlow/TFLite inference once the upload pipeline is stable.
    return {"classification": "pending_model", "confidence": None}


def row_to_dict(row):
    item = dict(row)
    if item.get("image_path"):
        item["image_url"] = url_for("uploaded_file", filename=Path(item["image_path"]).name)
    return item


@app.route("/")
def root():
    return redirect(url_for("dashboard"))


@app.route("/security")
@dashboard_auth_required
def dashboard():
    return render_template("index.html")


@app.route("/uploads/security/<path:filename>")
@dashboard_auth_required
def uploaded_file(filename):
    return send_from_directory(UPLOAD_DIR, filename)


@app.get("/api/security/events")
@dashboard_auth_required
def list_events():
    limit = min(int(request.args.get("limit", 50)), 200)
    with db() as conn:
        rows = conn.execute(
            "SELECT * FROM events ORDER BY created_ms DESC LIMIT ?",
            (limit,),
        ).fetchall()
    return jsonify([row_to_dict(row) for row in rows])


@app.get("/api/security/status")
@dashboard_auth_required
def status():
    with db() as conn:
        stats = conn.execute(
            """
            SELECT
              COUNT(*) AS total_events,
              COALESCE(SUM(trigger_count), 0) AS triggers,
              COALESCE(SUM(ignored_count), 0) AS ignored,
              COALESCE(SUM(sent_image_count), 0) AS sent_images
            FROM events
            """
        ).fetchone()
        devices = conn.execute("SELECT * FROM heartbeats ORDER BY updated_ms DESC").fetchall()
    return jsonify(
        {
            "stats": dict(stats),
            "devices": [dict(row) for row in devices],
            "server_time_ms": now_ms(),
        }
    )


@app.post("/api/security/events/<int:event_id>/ack")
@dashboard_auth_required
def acknowledge_event(event_id):
    with db() as conn:
        conn.execute(
            "UPDATE events SET acknowledged = 1, acknowledged_ms = ? WHERE id = ?",
            (now_ms(), event_id),
        )
    return jsonify({"ok": True})


@app.post("/api/security/commands")
@dashboard_auth_required
def create_command():
    data = request.get_json(force=True)
    device_id = data.get("device_id", "stick-s3-01")
    command = data.get("command")
    if not command:
        return jsonify({"error": "command is required"}), 400
    with db() as conn:
        cursor = conn.execute(
            "INSERT INTO commands (created_ms, device_id, command, payload) VALUES (?, ?, ?, ?)",
            (now_ms(), device_id, command, data.get("payload")),
        )
    return jsonify({"ok": True, "command_id": cursor.lastrowid})


@app.post("/api/device/security/events")
@device_auth_required
def device_event():
    data = request.get_json(force=True)
    with db() as conn:
        cursor = conn.execute(
            """
            INSERT INTO events (
                created_ms, device_id, event_type, mode, message, trigger_count,
                ignored_count, sent_image_count, battery_percent, rssi, latency_ms
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (
                data.get("created_ms", now_ms()),
                data.get("device_id", "unknown-device"),
                data.get("event_type", "motion"),
                data.get("mode"),
                data.get("message"),
                data.get("trigger_count", 0),
                data.get("ignored_count", 0),
                data.get("sent_image_count", 0),
                data.get("battery_percent"),
                data.get("rssi"),
                data.get("latency_ms"),
            ),
        )
    return jsonify({"ok": True, "event_id": cursor.lastrowid})


@app.post("/api/device/security/images")
@device_auth_required
def device_image():
    image = request.files.get("image")
    if not image:
        return jsonify({"error": "multipart field 'image' is required"}), 400

    event_id = request.form.get("event_id", type=int)
    device_id = request.form.get("device_id", "unknown-device")
    filename = secure_filename(image.filename or f"{device_id}_{now_ms()}.jpg")
    filename = f"{now_ms()}_{filename}"
    path = UPLOAD_DIR / filename
    image.save(path)

    result = classify_image_stub(path)
    with db() as conn:
        if event_id:
            conn.execute(
                """
                UPDATE events
                SET image_path = ?, classification = ?, confidence = ?, sent_image_count = sent_image_count + 1
                WHERE id = ?
                """,
                (str(path.relative_to(BASE_DIR)), result["classification"], result["confidence"], event_id),
            )
        else:
            cursor = conn.execute(
                """
                INSERT INTO events (
                    created_ms, device_id, event_type, mode, message, sent_image_count,
                    image_path, classification, confidence
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
                """,
                (
                    now_ms(),
                    device_id,
                    "image_upload",
                    request.form.get("mode"),
                    request.form.get("message"),
                    1,
                    str(path.relative_to(BASE_DIR)),
                    result["classification"],
                    result["confidence"],
                ),
            )
            event_id = cursor.lastrowid

    return jsonify({"ok": True, "event_id": event_id, **result})


@app.post("/api/device/security/heartbeat")
@device_auth_required
def device_heartbeat():
    data = request.get_json(force=True)
    with db() as conn:
        conn.execute(
            """
            INSERT INTO heartbeats (
                device_id, updated_ms, mode, battery_percent, rssi, firmware_version
            ) VALUES (?, ?, ?, ?, ?, ?)
            ON CONFLICT(device_id) DO UPDATE SET
                updated_ms = excluded.updated_ms,
                mode = excluded.mode,
                battery_percent = excluded.battery_percent,
                rssi = excluded.rssi,
                firmware_version = excluded.firmware_version
            """,
            (
                data.get("device_id", "unknown-device"),
                now_ms(),
                data.get("mode"),
                data.get("battery_percent"),
                data.get("rssi"),
                data.get("firmware_version"),
            ),
        )
    return jsonify({"ok": True})


@app.get("/api/device/security/commands")
@device_auth_required
def device_commands():
    device_id = request.args.get("device_id", "stick-s3-01")
    with db() as conn:
        rows = conn.execute(
            """
            SELECT * FROM commands
            WHERE device_id = ? AND consumed = 0
            ORDER BY created_ms ASC
            LIMIT 10
            """,
            (device_id,),
        ).fetchall()
        ids = [row["id"] for row in rows]
        if ids:
            placeholders = ",".join("?" for _ in ids)
            conn.execute(
                f"UPDATE commands SET consumed = 1, consumed_ms = ? WHERE id IN ({placeholders})",
                (now_ms(), *ids),
            )
    return jsonify([dict(row) for row in rows])


init_db()


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=int(os.environ.get("PORT", 5000)))
