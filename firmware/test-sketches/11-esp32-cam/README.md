# 11 — ESP32-CAM Bench Test

Phase 1 bring-up for the ESP32-CAM (OV2640 sensor). A bare-bones camera init and frame-capture test — no WiFi, no streaming — that confirms the module is alive before any of the later time-lapse / color-analysis work is built on it. Groundwork for the deferred vision feature (camera module replacement tracked in DL-034).

## Wiring (FTDI → ESP32-CAM)

| FTDI | ESP32-CAM |
|---|---|
| VCC (5V) | 5V |
| GND | GND |
| TX | U0R (RX) |
| RX | U0T (TX) |
| — | IO0 → GND (during flashing only; remove after) |

The ESP32-CAM has no USB; it is flashed through an FTDI USB-to-TTL adapter (on this machine at `/dev/cu.usbserial-A5069RR4`, distinct from the WROVER's port). A photo of this wiring is at `docs/images/esp32-cam-ftdi-wiring.png`.

## Run

1. Jumper IO0 → GND (puts the board in flash mode), then power/connect the FTDI.
2. `pio run -t upload`
3. Remove the IO0 → GND jumper and reset the board.
4. `pio device monitor`

## What it checks

1. The board enumerates correctly through the FTDI programmer.
2. The OV2640 camera initializes without errors.
3. Frame capture works and returns sensible image data (non-zero, plausible size).
4. PSRAM is detected and usable for the framebuffer.

## What success looks like

- Serial reports `PSRAM detected: yes`
- Camera init succeeds (no error code from `esp_camera_init`)
- Frames capture with a plausible byte length, and the frame counter increments steadily

## What this test does not verify

WiFi connectivity, the HTTP/MQTT image path, image quality (focus, exposure, color accuracy), and any color analysis — all of which belong to the integration phase once the replacement camera module is in hand. This sketch only answers "is the camera module fundamentally working." It also uses the standard AI-Thinker pin map, copied inline; a differently-pinned board would need that map adjusted.
