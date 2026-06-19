# hub/09-camera — camera image receiver

The vision node (Seeed XIAO ESP32-S3 Sense) captures a periodic JPEG of the
basil plant and uploads it to the hub. This component is the **Pi side of that
transport**: it receives the image, stores it, measures a greenness value, and
records the result for the dashboard.

## Transport: bytes over HTTP, event over MQTT (DL-076)

Images are too large to push cleanly through the MQTT broker, so the bytes
travel over HTTP while the lightweight *capture event* and the node's presence
travel over MQTT (the listener handles that side). This keeps MQTT for small
telemetry/presence messages and avoids giant payloads on the broker, while the
XIAO stays a first-class networked node alongside the WROVER.

`image_receiver.py` is a small always-on HTTP service (Python stdlib
`http.server`, threaded — no web framework):

- `POST /image` — raw JPEG body. Stores the file, computes greenness, writes a
  `camera_readings` row, returns `{ts, path, bytes, width, height, greenness}`.
- `GET /health` — liveness check, returns `{"status": "ok"}`.

It writes the JPEG to `IMAGE_DIR` as `cam-YYYYMMDD-HHMMSS-ffffff.jpg`
(microsecond suffix so rapid captures never collide) and inserts
`(ts, path, greenness)` into the `camera_readings` table, opened **WAL +
busy_timeout** so the dashboard can read while the receiver writes (the
multi-reader rule from DL-066).

## Greenness — Excess Green, not OpenCV (DL-076)

Greenness is the fraction of pixels whose **Excess-Green index**
(`ExG = 2g − r − b`, on per-pixel normalized RGB) exceeds `GREEN_EXG_THRESHOLD`.
ExG is a standard vegetation index, robust to overall brightness. It is computed
with numpy + Pillow rather than OpenCV: greenness is simple colour math and does
not justify cv2's weight on the Pi. OpenCV stays deferred until real CV/ML work
needs it. Note the metric tracks **foliage area in frame**, not health stress —
a dense canopy reads as high greenness regardless of vigour, which matters when
reading the dashboard trend.

## Env knobs (in `/etc/plant-hub/credentials`)

- `IMAGE_RECEIVER_PORT` (8080)
- `IMAGE_DIR` (`/home/basilpi/plant-hub/images`)
- `PLANT_DB` (`/home/basilpi/plant-hub/plant.db`)
- `GREEN_EXG_THRESHOLD` (0.10)

## Deploy

The repo holds source + docs; the **running copy is Pi-local** at
`/home/basilpi/plant-hub/image_receiver.py`. numpy and Pillow must be present in
the hub venv. Smoke-test from any machine on the network with a real JPEG:

```bash
curl -X POST --data-binary @plant.jpg -H "Content-Type: image/jpeg" \
  http://10.6.19.139:8080/image
```

A successful response returns the stored path and the greenness value; the row
lands in `camera_readings` for the dashboard to chart.

## Calibration reference

`test-fixtures/greenness-reference-chart.jpg` is a fixed colour-swatch chart
(greens, non-greens, neutrals) kept as a regression check on the greenness
metric. POSTing it to the receiver yields **greenness ≈ 0.4817** at the default
`GREEN_EXG_THRESHOLD = 0.10` (DL-076). If that value drifts after a change to
the metric or threshold, the maths moved — re-check before trusting new
captures. It is a deliberate fixture, never a real capture, so it lives here and
not in the live `images/` stream.
