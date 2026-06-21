# firmware/camera-node — XIAO ESP32-S3 Sense vision node

Production firmware for the camera node (the XIAO ESP32-S3 Sense chosen in DL-034, brought up in DL-077). Sibling to `firmware/integrated/` (the WROVER's firmware) — one production firmware project per node.

**v1 scope (DL-078):** on a timer, capture a JPEG and **HTTP POST it to the Pi image receiver** (`hub/09-camera`), which stores the file and computes greenness. That's it. Image bytes go over HTTP; the small MQTT capture-event / presence message (joining the DL-059 watchdog) is a deliberately separate later slice, as is the deployment cadence and photoperiod gating.

## Structure (modular, mirroring the WROVER)

- `main.cpp` — orchestration: init camera + WiFi, then on a cadence capture → POST.
- `net_wifi.{h,cpp}` — bounded blocking connect at boot, non-blocking reconnects; WiFi loss is non-fatal (captures skip while down, resume on reconnect).
- `camera.{h,cpp}` — XIAO `CAMERA_MODEL_XIAO_ESP32S3` pin map; init + capture (SVGA JPEG, framebuffer in PSRAM).
- `poster.{h,cpp}` — `HTTPClient` POST of the raw JPEG to `IMAGE_POST_URL`; logs the receiver's JSON reply (incl. greenness).
- `config.h` — non-secret config: Pi endpoint URL, capture cadence, WiFi/HTTP timeouts.
- `secrets.h.example` — template for `secrets.h` (gitignored).

## Credentials

WiFi credentials live in `secrets.h`, which is **gitignored** and never committed. Create it from the template:

```bash
cd firmware/camera-node/src
cp secrets.h.example secrets.h
# edit secrets.h: set WIFI_SSID / WIFI_PASSWORD to the JSU_DEVICE values
```

## Build, flash, validate

The U.FL **antenna must be seated** (this is the first slice that uses WiFi). Flash over a USB-C **data** cable (the XIAO auto-resets; port is `/dev/cu.usbmodem*`):

```bash
pio run -t upload --upload-port /dev/cu.usbmodemXXXX
pio device monitor --port /dev/cu.usbmodemXXXX -b 115200
```

Expected serial:

```
Camera node starting
Camera ready
WiFi: connecting to JSU_DEVICE....
WiFi: connected, IP 10.6.x.x  RSSI -NN
captured 800x600  ~16000 bytes -> POST
POST 200: {"ts": ..., "greenness": 0.xx, ...}
```

A `POST 200` with a greenness value — and a matching new row in the Pi's `camera_readings` (and a `cam-*.jpg` in `hub`'s image dir) — confirms the end-to-end path. The capture cadence in `config.h` is a short **test** value; the deployment cadence (hourly, photoperiod-gated) is a later slice.

## Not yet (later slices)

MQTT capture-event + presence (DL-059 watchdog), hourly/photoperiod-gated cadence, top-down tripod mounting, dashboard latest-image panel and greenness trend, rolling image retention.
