# Changelog

All notable changes to this repository are documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/). The project will adopt [Semantic Versioning](https://semver.org/) once a first version is tagged.

This file tracks **repository-level changes** — files, structure, and tooling. The reasoning behind each change lives in [`docs/decision-log.md`](docs/decision-log.md), which is the authoritative engineering record; entries below cite the relevant `DL-NNN` for that rationale.

## [Unreleased]

### Added

- Rolling camera-image retention (DL-092): the nightly retention job now prunes camera JPEG files older than 90 days (env-overridable) while keeping the small `camera_readings` metric rows, bounding SD-card growth without losing the growth signal.

- Dashboard "Plant camera" panel (DL-090): latest capture with `green_ratio`/`green_area` cards and a 24h/7d greenness trend showing the DL-087 baseline band. Also taught the dashboard the `maintenance` FSM state so it renders as a labelled amber banner.

- FSM maintenance mode (DL-089): an intentional, NVS-persisted watering pause toggled by a long-press of the MANUAL button. Distinct from a fault (steady green+yellow LEDs, `maintenance` state, no alert), it blocks watering while leaving leak/stop safety active, and survives reboots — for dry-downs, maintenance, or absences.

- Silent-camera alert (DL-088): `alerter.py` now sends an ntfy push if no camera image arrives during the lit photoperiod window (2 h default), catching a dead camera node or a down image-receiver — with a dawn hold-off to avoid false alarms and a recovery notification when images resume.

- Documented the camera's 7-day observed baseline ranges in `hub/09-camera/METRICS.md` (descriptive ranges for the current fixed framing; no alert thresholds yet) (DL-087).

**Project scaffolding**
- `README.md`, `LICENSE` (MIT), `.gitignore` (macOS / PlatformIO / Python / editor artifacts), and this `CHANGELOG.md`.
- `docs/decision-log.md` — the authoritative engineering decision record (DL-001 onward).
- `docs/explainers/phase3-hub.md` — narrative explainer for the hub phase.

**Phase 1 — component bench tests** under `firmware/test-sketches/`, each a standalone PlatformIO project with its own README:
- `01-bme280` (DL-015; re-validated on the integrated bench in DL-019), `02-pump-mosfet` (DL-018), `03-soil-moisture` (DL-020), `04-bh1750` (DL-021), `05-float-switch` (DL-023), `06-oled` (DL-024), `07-feedback-io` / `08-buttons` / `09-buzzer` (user-feedback subsystem, DL-025), `10-leak-sensor` (DL-026).
- `11-esp32-cam` — camera bring-up sketch, committed as a starting point; valid but unvalidated by execution pending a hardware replacement (DL-034).
- `12-traffic-light-leds` — three-LED status validation after the GPIO rewire (DL-045).
- `13-pump-calibration` — pump flow-rate calibration, ~1.0 mL/s (DL-048).
- `14-xiao-cam` — XIAO ESP32-S3 Sense camera bring-up; validated PSRAM + camera init + frame capture, sensor OV3660 (DL-077).

**Phase 3 — Raspberry Pi hub** under `hub/`, each step numbered with its own README:
- `01-pi-setup` and `02-mosquitto-install` — Pi bootstrap and broker install (DL-027, DL-029).
- `03-broker-config` — authenticated LAN broker configuration (DL-030).
- `04-listener` — Python MQTT→SQLite listener, six-table schema, run-aware (DL-035).
- `05-listener-service` and `07-dashboard-service` — systemd units for auto-start / auto-restart at boot (DL-036).
- `06-dashboard` — read-only Streamlit dashboard (cream botanical theme, Plotly charts, UTC storage with America/Chicago display); LAN- and Tailscale-accessible (DL-037, DL-038).
- `08-grow-light` — Shelly device-side RPC scheduler for the grow-light photoperiod (DL-054).
- `08-grow-light` — Shelly self-recovery after it went unreachable unattended (WiFi drop, no reconnect): on-device `wifi-watchdog.js` reboots the device after sustained WiFi loss, plus a daily 02:00 reboot backstop; installed via local RPC with `install_wifi_watchdog.py` (DL-085).
- `09-camera` — Pi-side image receiver: HTTP `POST /image` → filesystem + Excess-Green greenness → new `camera_readings` table; the hub half of the vision-node transport (bytes over HTTP, capture event/presence over MQTT). Includes `test-fixtures/greenness-reference-chart.jpg`, a greenness regression reference (≈ 0.4817) (DL-076).
- `09-camera` — greenness now measured on the **largest connected green blob** (self-locating, ignores stray green specks); records `green_area` (plant fraction of frame) and `green_ratio` (green density in the plant region) alongside full-frame `greenness`; adds a scipy dependency (DL-079).
- `09-camera` — captures outside the grow-light window are silently discarded (shares the enforcer's `GROW_ON_HOUR`/`GROW_OFF_HOUR`/`LOCAL_TZ`); receiver installed as the `plant-image-receiver` systemd service (auto-start, restart-on-failure) for unattended operation (DL-082).
- `09-camera/METRICS.md` — interpretation guide for the greenness metrics: what each means, reference readings, how to read a trend, and the honest scope (vigour/growth, not defect detection); absolute thresholds deferred to the baseline run (DL-083).
- `09-camera` — receiver now stores `camera_readings.ts` in UTC (with `Z`), matching the hub-wide standard so the dashboard renders camera timestamps in correct local time; was naive local (DL-084).
- `09-camera/test-fixtures/resolution-comparison/` — SVGA/UXGA/QXGA captures of the mounted plant kept as evidence for the deployment resolution decision: **UXGA** (detail sweet spot vs payload/storage on a weak link) (DL-080).
- `04-listener/alerter.py` — push-notification alerting layer via [ntfy](https://ntfy.sh): leak, watering-fault, reservoir-empty, prolonged-offline, and flapping-reboot alerts with recovery pings and a daily heartbeat summary, polled and edge-triggered from the listener; the topic is configured off-repo via the systemd `EnvironmentFile` (DL-061).

**Vision node** under `firmware/camera-node/` (XIAO ESP32-S3 Sense) — v1: capture a JPEG on a timer and HTTP POST it to the Pi receiver (`hub/09-camera`); modular WiFi/camera/poster, gitignored `secrets.h`; validated end-to-end (DL-078).
  - Deployment settings: UXGA 1600×1200, `jpeg_quality` 10, hourly capture cadence (DL-081).

**Phase 2 — integrated WROVER firmware** under `firmware/integrated/` (one PlatformIO project):
- Full firmware: WiFi + MQTT transport with presence/Last-Will, all sensor modules (BME280, BH1750, soil, float, leak), a seven-state safety-first watering state machine, pump dosing (pulse/settle) with a daily volume cap, a leak-only buzzer alarm, an OLED status display, and traffic-light status LEDs (DL-040 through DL-049).
- Watering-effectiveness watchdog with a dedicated `watering_fault` state (DL-053).

**Evidence images** under `docs/images/`:
- `bme280-validation` (DL-015), `pump-mosfet-validation` (DL-018), `mosquitto-loopback-verification` (DL-029), `esp32-cam-ftdi-wiring` (DL-034), `dashboard-desktop-{1,2,3}` and `dashboard-mobile-{1,2}` (DL-037), `float-mount-{1,2,3,4}` (DL-050).

### Changed

- Dashboard performance (DL-091): slowed the auto-refresh from 10 s to 30 s and downscaled+cached the camera image, fixing image-load and scroll failures on Safari and mobile caused by the full-page rerun against a heavier page.

- Recorded field validation of the Shelly self-recovery (DL-085) over a multi-day absence: the daily 02:00 reboot is confirmed firing nightly and the watchdog is reboot-persistent; the reactive WiFi-loss recovery is still pending a real outage to exercise it.

- `hub/04-listener` — routes FSM state (`plant/state/wrover`) and device presence (`plant/status/<device>`) into the database (DL-052, DL-055); added a device-presence timeout watchdog that catches a device hung-but-still-connected, which the Last-Will misses (DL-059), and reboot detection from `uptime_s` resets, recorded as `system_status` markers (DL-060).
- `hub/06-dashboard` — added a watering-system state banner, a `watering_fault` display, and device-presence/offline awareness: the banner and header pill reflect "WROVER offline" when its Last-Will fires (DL-052, DL-053, DL-055); greys stale environment cards when the WROVER is offline (DL-056), added a soil-moisture trend with a watering-episode overlay (DL-057), switched the daily-watering banner from seconds to mL (DL-058), and surfaced a last-reboot indicator with a flapping warning at ≥ 2 reboots/24h (DL-060).
- Firmware daily water cap re-expressed from pump run-time to volume (`MAX_DAILY_PUMP_ML = 200`), and the daily reset moved from a boot-relative rolling 24h window to true local-calendar midnight via NTP (DL-058).
- Renamed `hub/grow-light` → `hub/08-grow-light` and added its README, for consistency with the numbered hub steps.
- Renamed every file in `docs/images/` from the collision-prone `NN-` counter scheme to stable descriptive names (e.g. `05-float-mount-1.jpg` → `float-mount-1.jpg`), and updated all references across the decision log, this changelog, and the READMEs.
- Added READMEs to test-sketches `11`–`13`, bringing them to parity with `01`–`10`.
- MQTT username renamed `basilpi` → `basilmqtt` to avoid colliding with the Linux account, and adopted a credential-file workflow so passwords are no longer passed on command lines (DL-030, DL-032).
- Button C reassigned GPIO35 → GPIO26 (GPIO35 has no internal pull-up; GPIO26 was freed by the DL-010 grow-light architecture).

### Fixed

- Repo audit fixes (DL-086): corrected a dangling `docs/hub-setup.md` reference in this changelog, made the `alerter.py` grow-light photoperiod check wrap-aware (consistent with `photoperiod.py`/`image_receiver.py`; no change for the current 07–19 window), and moved the `camera_readings` table definition into `hub/04-listener/schema.sql`.

- Repaired this changelog: removed a leaked heredoc delimiter and a duplicated listener-service entry, stripped escape artifacts from an earlier bad paste (literal backslash-escaped backticks and `\u2014` em-dash sequences), restructured to the grouped Keep a Changelog format, and brought it current through DL-055.
