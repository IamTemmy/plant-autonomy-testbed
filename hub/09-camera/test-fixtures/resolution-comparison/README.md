# Resolution comparison — capture-size decision (DL-080)

Three captures of the same mounted basil under the grow light, one per camera frame size, kept as the evidence behind choosing **UXGA** as the deployment resolution. The OV3660 supports all three; the question was the trade-off between image detail (useful for a future ML plant-health classifier) and payload size over a weak WiFi link (RSSI ≈ −79 dBm) plus long-term storage.

| File | Frame size | Megapixels | Typical JPEG size |
|------|-----------|-----------|-------------------|
| `svga-800x600.jpg`   | 800 × 600   | 0.5 MP | ~33 KB  |
| `uxga-1600x1200.jpg` | 1600 × 1200 | 1.9 MP | ~110 KB |
| `qxga-2048x1536.jpg` | 2048 × 1536 | 3.1 MP | ~170 KB |

(All at `jpeg_quality = 10`. The XIAO captures the full scene; greenness is measured Pi-side on the isolated plant region, DL-079.)

## What the images show

- **SVGA → UXGA is a dramatic jump.** SVGA is soft; leaf veins, edges, and surface texture are blurred. UXGA resolves individual leaf veins and edges clearly — the difference is night-and-day, and it is the detail a leaf-health classifier would train on.
- **UXGA → QXGA is marginal.** QXGA is only slightly sharper than UXGA. The sensor is small and the light is indoor, so beyond ~2 MP the extra pixels add little real detail — while the file is ~55% larger.

## Decision: UXGA

UXGA is the sweet spot. Reasons:

- **Detail:** UXGA captures the bulk of the usable detail (the big gain is SVGA → UXGA). QXGA's extra detail is barely visible.
- **Transfer:** both UXGA and QXGA posted to the Pi at 100% over the (good-cable) link, but QXGA's larger payload has less headroom on a marginal −79 dBm signal; UXGA is the safer steady-state choice.
- **Storage:** at the deployment cadence (hourly), UXGA is comfortably small over a year; QXGA is ~55% more for no visible benefit.
- **Metric stability:** the Pi-side `green_area`/`green_ratio` metrics are fractions and held steady across all three sizes (DL-079), so resolution choice does not disturb the greenness trend — the decision is purely about image quality vs cost.

The deployment firmware sets `FRAMESIZE_UXGA` accordingly (recorded where that change lands). These images are reference evidence only, not a runtime fixture.
