#!/usr/bin/env python3
"""Plant Autonomy Testbed - camera image receiver (hub/09-camera).

Transport option B (DL-076): image bytes arrive over HTTP, while the capture
event / presence travels over MQTT (handled by the listener, added later).

This service accepts an HTTP POST of a JPEG, stores it on the Pi filesystem,
measures the plant's greenness, and records the result in SQLite. All image
processing is Pi-side; the XIAO only captures and uploads.

Metrics (DL-079): the full image is kept, but greenness is measured on the
*largest connected green blob* (the plant), so it self-locates if the pot shifts
and ignores small stray green specks (a status LED, a reflection). Recorded:
  greenness   - full-frame Excess-Green ratio (ExG = 2g - r - b on per-pixel
                normalized RGB, fraction over threshold; kept for continuity).
  green_area  - largest green blob as a fraction of the frame ("how much
                plant"); resolution-independent, the growth signal.
  green_ratio - green fraction within that blob's bounding box ("how green the
                plant region is").

Config via environment (systemd EnvironmentFile=/etc/plant-hub/credentials):
  IMAGE_RECEIVER_PORT   default 8080
  IMAGE_DIR             default /home/basilpi/plant-hub/images
  PLANT_DB              default /home/basilpi/plant-hub/plant.db
  GREEN_EXG_THRESHOLD   default 0.10
  LOCAL_TZ              default America/Chicago   } photoperiod gate, shared with
  GROW_ON_HOUR          default 7                 } the grow-light enforcer
  GROW_OFF_HOUR         default 19                } (hub/08-grow-light)

Captures POSTed outside the GROW_ON_HOUR..GROW_OFF_HOUR window are silently
discarded (DL-082), so the dataset holds only lit daytime frames.

Endpoints:
  POST /image   raw JPEG body  -> {ts, path, bytes, width, height,
                                   greenness, green_area, green_ratio}
                                   or {skipped} if outside the photoperiod
  GET  /health                 -> {status: ok}
"""
import os
import json
import sqlite3
import datetime
from zoneinfo import ZoneInfo
from io import BytesIO
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

import numpy as np
from PIL import Image

PORT = int(os.environ.get("IMAGE_RECEIVER_PORT", "8080"))
IMAGE_DIR = os.environ.get("IMAGE_DIR", "/home/basilpi/plant-hub/images")
DB_PATH = os.environ.get("PLANT_DB", "/home/basilpi/plant-hub/plant.db")
EXG_THRESHOLD = float(os.environ.get("GREEN_EXG_THRESHOLD", "0.10"))

# Photoperiod gating (DL-082): only daytime/lit captures are kept. These read the
# SAME env as the grow-light enforcer (hub/08-grow-light/photoperiod.py), loaded
# from /etc/plant-hub/credentials, so the window is defined once and cannot drift.
LOCAL_TZ = ZoneInfo(os.environ.get("LOCAL_TZ", "America/Chicago"))
GROW_ON_HOUR = int(os.environ.get("GROW_ON_HOUR", "7"))
GROW_OFF_HOUR = int(os.environ.get("GROW_OFF_HOUR", "19"))


def within_photoperiod(now=None):
    """True if the local hour is inside the grow-light window. Mirrors the
    enforcer's desired_on() exactly, including the overnight-wrap case."""
    if now is None:
        now = datetime.datetime.now(LOCAL_TZ)
    hour = now.hour
    if GROW_ON_HOUR < GROW_OFF_HOUR:
        return GROW_ON_HOUR <= hour < GROW_OFF_HOUR
    return hour >= GROW_ON_HOUR or hour < GROW_OFF_HOUR
MAX_BYTES = 5 * 1024 * 1024


def init_db():
    conn = sqlite3.connect(DB_PATH, timeout=30)
    try:
        conn.execute("PRAGMA journal_mode=WAL;")
        conn.execute("PRAGMA busy_timeout=5000;")
        conn.execute(
            "CREATE TABLE IF NOT EXISTS camera_readings ("
            "ts TEXT PRIMARY KEY, path TEXT NOT NULL, greenness REAL NOT NULL, "
            "green_area REAL, green_ratio REAL)"
        )
        # Add the dual-metric columns to a pre-existing table (DL-079).
        for col in ("green_area REAL", "green_ratio REAL"):
            try:
                conn.execute("ALTER TABLE camera_readings ADD COLUMN %s" % col)
            except sqlite3.OperationalError:
                pass  # column already exists
        conn.commit()
    finally:
        conn.close()


def compute_metrics(jpeg_bytes):
    """Return (greenness, green_area, green_ratio, (w, h)).

    greenness   - full-frame Excess-Green ratio (kept for continuity and the
                  regression fixture).
    green_area  - largest green blob as a fraction of the frame ("how much
                  plant"). A fraction, so it is resolution-independent and
                  survives a capture-size change; self-locating, so it tracks the
                  plant even if the pot shifts within the frame.
    green_ratio - green fraction within that blob's bounding box ("how green the
                  plant region is").

    The largest connected green blob is used so small stray green specks (a
    status LED, a reflection) are ignored. Falls back to the full-frame ratio if
    scipy is unavailable.
    """
    img = Image.open(BytesIO(jpeg_bytes)).convert("RGB")
    arr = np.asarray(img, dtype=np.float64)
    r, g, b = arr[..., 0], arr[..., 1], arr[..., 2]
    total = r + g + b
    total[total == 0] = 1.0
    rn, gn, bn = r / total, g / total, b / total
    exg = 2.0 * gn - rn - bn
    mask = exg > EXG_THRESHOLD

    greenness = float(mask.mean())
    green_area = greenness
    green_ratio = greenness
    try:
        from scipy import ndimage
        labels, n = ndimage.label(mask)
        if n > 0:
            sizes = ndimage.sum(mask, labels, index=np.arange(1, n + 1))
            largest = int(np.argmax(sizes)) + 1
            blob = labels == largest
            green_area = float(blob.sum()) / float(mask.size)
            ys, xs = np.where(blob)
            region = mask[ys.min():ys.max() + 1, xs.min():xs.max() + 1]
            green_ratio = float(region.mean())
    except ImportError:
        pass

    return greenness, green_area, green_ratio, img.size


def record(ts, path, greenness, green_area, green_ratio):
    conn = sqlite3.connect(DB_PATH, timeout=30)
    try:
        conn.execute("PRAGMA busy_timeout=5000;")
        conn.execute(
            "INSERT OR REPLACE INTO camera_readings "
            "(ts, path, greenness, green_area, green_ratio) "
            "VALUES (?, ?, ?, ?, ?)",
            (ts, path, greenness, green_area, green_ratio),
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

        if not within_photoperiod():
            # Outside the grow-light window: silently discard (DL-082) — no file,
            # no row, no log. Keeps the dataset to lit daytime captures. A dark
            # image *during* the window is still kept, so a light failure shows as
            # an anomalously low daytime greenness rather than being hidden.
            self._json(200, {"skipped": "outside photoperiod"})
            return

        try:
            greenness, green_area, green_ratio, (w, h) = compute_metrics(data)
        except Exception as exc:
            self._json(400, {"error": "decode failed: %s" % exc})
            return

        # Store ts in UTC with a trailing Z, matching the rest of the hub
        # (listener's utc_now_iso). The dashboard reads ts as UTC and converts
        # to the display zone; a naive local ts would be misread. Filename keeps
        # the same UTC instant for a stable, sortable name.
        now = datetime.datetime.now(datetime.timezone.utc)
        ts = now.strftime("%Y-%m-%dT%H:%M:%S.%fZ")
        fname = "cam-" + now.strftime("%Y%m%d-%H%M%S-%f") + ".jpg"
        path = os.path.join(IMAGE_DIR, fname)
        with open(path, "wb") as fh:
            fh.write(data)
        record(ts, path, greenness, green_area, green_ratio)

        print("stored %s  %dx%d  %d bytes  greenness=%.4f area=%.4f ratio=%.4f"
              % (fname, w, h, len(data), greenness, green_area, green_ratio),
              flush=True)
        self._json(200, {"ts": ts, "path": path, "bytes": len(data),
                         "width": w, "height": h,
                         "greenness": round(greenness, 4),
                         "green_area": round(green_area, 4),
                         "green_ratio": round(green_ratio, 4)})

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
