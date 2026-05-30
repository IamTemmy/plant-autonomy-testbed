# Changelog

All notable changes to this repository are documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/). The project will adopt [Semantic Versioning](https://semver.org/) once a first version is tagged.

This file tracks **repository-level changes** (files, structure, tooling). Engineering decisions and the reasoning behind them live in [`docs/decision-log.md`](docs/decision-log.md).

## [Unreleased]

### Added
- Initial repository structure with `docs/` and `firmware/test-sketches/01-bme280/`
- `README.md` — project overview, engineering framing, architecture summary, build notes
- `LICENSE` — MIT
- `CHANGELOG.md` — this file
- `.gitignore` — macOS, PlatformIO, Python, and editor artifacts
- `docs/decision-log.md` — engineering decisions and rationale, seeded with project baseline and Phase 1 refinements

### Validated
- BME280 component test passed — sensor detected at I2C address 0x76/0x77, plausible indoor readings, responsive to breath stimulus (see `docs/images/01-bme280-validation.png`)

### Added (continued)
- `firmware/test-sketches/01-bme280/` — PlatformIO project for BME280 bench test (platformio.ini, src/main.cpp, README.md)
- `docs/images/01-bme280-validation.png` — serial-monitor screenshot capturing baseline + breath-response readings

### Validated (continued)
- Peristaltic pump + IRLB8721 MOSFET driver — clean ON/OFF transitions, flyback diode behaving correctly, no ESP32 resets, water flow confirmed in wet test (see DL-018)

### Added (continued)
- \`firmware/test-sketches/02-pump-mosfet/\` — PlatformIO project for pump driver bench test (platformio.ini, src/main.cpp, README.md)
- \`docs/images/02-pump-mosfet-validation.png\` — serial-monitor screenshot of pump cycling

### Re-validated
- BME280 — re-mounted onto the integrated breadboard (alongside pump + buck) and re-tested against the original sketch; no behavioral regression (see DL-019)

### Validated (continued)
- Capacitive soil moisture sensor — clean monotonic response (dry air → dry soil → wet soil), low noise (~15 ADC counts after averaging), operational range of ~570 counts between dry-soil and wet-soil conditions (see DL-020)

### Added (continued)
- \`firmware/test-sketches/03-soil-moisture/\` — PlatformIO project for soil moisture bench test and calibration (platformio.ini, src/main.cpp, README.md)
