# 14 — XIAO ESP32-S3 Sense Camera Bring-up

Proves the vision node (Seeed XIAO ESP32-S3 Sense, the camera node chosen in DL-034) is alive before any capture/transport firmware is written: the board enumerates over USB-C, PSRAM comes up, the camera initializes, and frames capture at sensible sizes. This is the XIAO counterpart to `11-esp32-cam` — and a deliberately separate, minimal slice (board + camera only, no WiFi/SD/HTTP) so a failure here points at hardware, not networking.

## Setup

Just the XIAO ESP32-S3 with the camera daughter-board seated on the B2B connector (press until it clicks), connected to the Mac with a **USB-C data cable**. No FTDI, no jumpers, no external power — the native USB does power + flash + serial, and the board auto-resets into the bootloader.

**Cable caveat:** a power-only USB-C cable powers the board but never enumerates (no serial port appears). Use a known-good data cable to flash. The long power-only cable is for *deployment* (XIAO on the tripod, powered from a wall adapter, talking to the Pi over WiFi) — it cannot program the board.

The WiFi antenna is **not** needed for this sketch (no WiFi); it matters only for the later capture-and-POST slice.

## Run

`pio run -t upload --upload-port /dev/cu.usbmodemXXXX`, then
`pio device monitor --port /dev/cu.usbmodemXXXX -b 115200`.

The port enumerates as `/dev/cu.usbmodem*` (not the WROVER's `usbserial-*`) and the number can shift across resets — find the current one with `ls /dev/cu.usbmodem*`. If an upload ever fails to connect, the manual bootloader fallback is: hold **BOOT**, tap **RESET**, release **BOOT**, then re-run upload. (The two buttons are tiny, beside the USB-C connector on the main board.)

The header lines print once at boot, so if the monitor opens mid-stream you'll join at the `Frame N` lines — power-cycle or RESET with the monitor open to reprint from the top.

## What success looks like

```
XIAO ESP32-S3 Sense — camera bring-up
PSRAM: found
Camera init OK
Sensor PID: 0x3660  (OV2640=0x26, OV3660=0x3660, OV5640=0x5640)
Capturing frames...
Frame 1  800x600  15551 bytes
Frame 2  800x600  15684 bytes
...
```

- **PSRAM: found** — the framebuffer lives in PSRAM (needs the `qio_opi` + `BOARD_HAS_PSRAM` build flags).
- **Camera init OK** + a sensor **PID** — the OV-series sensor is reachable over its control bus and identified (0x3660 = OV3660 on this unit).
- **Frames incrementing** at a steady ~16 KB SVGA — the camera captures real JPEGs; the counter climbing with no resets or `NULL` grabs means a stable board.

Validated 2026-06-19 (DL-077).

## What this test does not verify

WiFi (antenna seating, association, the campus-network path), the HTTP-POST transport to the Pi receiver (`hub/09-camera`), SD storage, image quality/focus/exposure under the grow light, or the eventual capture cadence. Those belong to the capture-and-POST slice that follows this bring-up.
