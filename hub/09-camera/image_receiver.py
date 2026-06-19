#!/usr/bin/env python3
"""Plant Autonomy Testbed - camera image receiver (hub/09-camera).

Transport option B (DL-076): image bytes arrive over HTTP, while the capture
event / presence travels over MQTT (handled by the listener, added later).

This service accepts an HTTP POST of a JPEG, stores it on the Pi filesystem,
computes an Excess-Green vegetation ratio ("greenness"), and records
(ts, path, greenness) in SQLite. All image processing is Pi-side; the XIAO
only captures and uploads.

Greenness = fraction of pixels whose Excess-Green index (ExG = 2g - r - b on
per-pixel normalized RGB) exceeds GREEN_EXG_THRESHOLD. ExG is a standard
vegetation index; the ratio tracks foliage area in frame, not health stress.

Config via environment (systemd EnvironmentFile=/etc/plant-hub/credentials):
  IMAGE_RECEIVER_PORT   default 8080
  IMAGE_DIR             default /home/basilpi/plant-hub/images
  PLANT_DB              default /home/basilpi/plant-hub/plant.db
  GREEN_EXG_THRESHOLD   default 0.10

Endpoints:
  POST /image   raw JPEG body  -> {ts, path, bytes, width, height, greenness}
  GET  /health                 -> {status: ok}
"""
import os
import json
import sqlite3
import datetime
from io import BytesIO
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

import numpy as np
from PIL import Image

PORT = int(os.environ.get("IMAGE_RECEIVER_PORT", "8080"))
IMAGE_DIR = os.environ.get("IMAGE_DIR", "/home/basilpi/plant-hub/images")
DB_PATH = os.environ.get("PLANT_DB", "/home/basilpi/plant-hub/plant.db")
EXG_THRESHOLD = float(os.environ.get("GREEN_EXG_THRESHOLD", "0.10"))
MAX_BYTES = 5 * 1024 * 1024


def init_db():
    conn = sqlite3.connect(DB_PATH, timeout=30)
    try:
        conn.execute("PRAGMA journal_mode=WAL;")
        conn.execute("PRAGMA busy_timeout=5000;")
        conn.execute(
            "CREATE TABLE IF NOT EXISTS camera_readings ("
            "ts TEXT PRIMARY KEY, path TEXT NOT NULL, greenness REAL NOT NULL)"
        )
        conn.commit()
    finally:
        conn.close()


def compute_greenness(jpeg_bytes):
    img = Image.open(BytesIO(jpeg_bytes)).convert("RGB")
    arr = np.asarray(img, dtype=np.float64)
    r, g, b = arr[..., 0], arr[..., 1], arr[..., 2]
    total = r + g + b
    total[total == 0] = 1.0
    rn, gn, bn = r / total, g / total, b / total
    exg = 2.0 * gn - rn - bn
    return float((exg > EXG_THRESHOLD).mean()), img.size


def record(ts, path, greenness):
    conn = sqlite3.connect(DB_PATH, timeout=30)
    try:
        conn.execute("PRAGMA busy_timeout=5000;")
        conn.execute(
            "INSERT OR REPLACE INTO camera_readings (ts, path, greenness) "
            "VALUES (?, ?, ?)",
            (ts, path, greenness),
        )
        conn.commit()
    finally:
        conn.close()


class Handler(BaseHTTPRequestHandler):
    def _json(self, code, payload):
        body = json.dumps(payload).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path == "/health":
            self._json(200, {"status": "ok"})
        else:
            self._json(404, {"error": "not found"})

    def do_POST(self):
        if self.path != "/image":
            self._json(404, {"error": "not found"})
            return
        try:
            length = int(self.headers.get("Content-Length", 0))
        except ValueError:
            length = 0
        if length <= 0 or length > MAX_BYTES:
            self._json(400, {"error": "bad or missing content-length"})
            return
        data = self.rfile.read(length)
        if data[:2] != b"\xff\xd8":
            self._json(400, {"error": "not a jpeg"})
            return
        try:
            greenness, (w, h) = compute_greenness(data)
        except Exception as exc:
            self._json(400, {"error": "decode failed: %s" % exc})
            return

        now = datetime.datetime.now()
        ts = now.isoformat(timespec="microseconds")
        fname = "cam-" + now.strftime("%Y%m%d-%H%M%S-%f") + ".jpg"
        path = os.path.join(IMAGE_DIR, fname)
        with open(path, "wb") as fh:
            fh.write(data)
        record(ts, path, greenness)

        print("stored %s  %dx%d  %d bytes  greenness=%.4f"
              % (fname, w, h, len(data), greenness), flush=True)
        self._json(200, {"ts": ts, "path": path, "bytes": len(data),
                         "width": w, "height": h,
                         "greenness": round(greenness, 4)})

    def log_message(self, fmt, *args):
        return


def main():
    os.makedirs(IMAGE_DIR, exist_ok=True)
    init_db()
    server = ThreadingHTTPServer(("0.0.0.0", PORT), Handler)
    print("image-receiver on :%d  images=%s  db=%s  exg_threshold=%.2f"
          % (PORT, IMAGE_DIR, DB_PATH, EXG_THRESHOLD), flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
