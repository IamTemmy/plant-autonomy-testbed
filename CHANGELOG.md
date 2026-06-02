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

### Validated (continued)
- BH1750 light sensor — calibrated lux output, responds correctly to light changes, coexists cleanly on the I²C bus with BME280 (see DL-021)

### Added (continued)
- \`firmware/test-sketches/04-bh1750/\` — PlatformIO project for BH1750 bench test (platformio.ini, src/main.cpp, README.md)

### Validated (continued)
- Float switch — clean state transitions on orientation change, orientation-to-state mapping recorded for firmware reference (see DL-023)

### Added (continued)
- \`firmware/test-sketches/05-float-switch/\` — PlatformIO project for float switch bench test (platformio.ini, src/main.cpp, README.md)

### Validated (continued)
- SSD1306 OLED display — splash and mock dashboard render cleanly, uptime counter confirms active refresh, three-device I²C bus (BME280 + BH1750 + OLED) validated healthy (see DL-024)

### Added (continued)
- \`firmware/test-sketches/06-oled/\` — PlatformIO project for OLED bench test with Phase 2 dashboard preview (platformio.ini, src/main.cpp, README.md)

### Validated (continued)
- User-feedback subsystem (2 LEDs + 3 buttons + active buzzer) — validated across three iterative sketches with a four-state state machine; safety property (FAULT latching, MANUAL inert during fault) demonstrated (see DL-025)

### Added (continued)
- \`firmware/test-sketches/09-buzzer/\` — PlatformIO project for active buzzer test integrating LEDs, buttons, and CRITICAL state (platformio.ini, src/main.cpp, README.md)

### Changed
- Button C moved from GPIO35 to GPIO26 (GPIO35 lacks an internal pull-up; GPIO26 was freed by DL-010's grow-light architecture change)

### Validated (continued)
- DIYables leak sensor — clean dry baseline at 0 ADC counts, ~50% practical maximum at full water contact, sensitive trip-wire character suitable for binary leak detection (see DL-026)

### Added (continued)
- \`firmware/test-sketches/10-leak-sensor/\` — PlatformIO project for leak sensor bench test and calibration (platformio.ini, src/main.cpp, README.md)

### Added
- \`hub/01-pi-setup/\` — Raspberry Pi flashing and bootstrap procedure
- \`hub/02-mosquitto-install/\` — Mosquitto broker installation (configuration and testing pending)
- DL-027 — Hub bootstrap milestone

### Decided
- DL-028 — Campus deployment network deferred; project on home WiFi until IT consultation can confirm a campus path that satisfies the four criteria for IoT-class device-to-device communication

### Validated
- Mosquitto broker on Pi — loopback pub/sub verified, broker correctly routes messages between independent clients on localhost (see DL-029)

### Added
- \`docs/images/03-mosquitto-loopback-verification.png\` — screenshot of two SSH sessions during loopback test
- Broker verification section appended to \`hub/02-mosquitto-install/README.md\`

### Resolved
- DL-028 partially resolved — JSU_DEVICE confirmed viable for deployment after second re-flash succeeded; client-isolation concern ruled out by ARP visibility analysis; original Pi-join failure attributed to silent Imager password typo

### Documentation
- Updated DL-028 with empirical findings from 2026-06-01 campus network testing

### Added
- \`hub/03-broker-config/\` — Mosquitto broker LAN configuration, authentication, and verification procedure
- DL-030 — Broker LAN configuration milestone

### Validated
- Mosquitto broker reachable from external client (Mac) over JSU_DEVICE with authentication; three-test verification pattern (anonymous rejected, loopback authenticated, external client authenticated) all passed

### Validated
- Shelly Plus Plug US ("basilplug") paired and joined JSU_DEVICE on 10.6.17.32; third device confirming JSU_DEVICE viability for project use (see DL-031)

### Resolved
- DL-028 fully resolved — JSU_DEVICE is the deployment network; campus deferral closed

### Validated
- Shelly Plus Plug US MQTT integration with the project's Mosquitto broker — bidirectional control validated end-to-end from the Mac; the Shelly publishes status and events under \`plant/grow-light/\` and responds to commands; tested at deployment location near the plant (see DL-032)

### Changed
- MQTT username renamed from \`basilpi\` to \`basilmqtt\` to avoid collision with the Linux user account name
- DL-030 updated with the username rename note for accuracy

### Changed
- Adopted credential-file workflow on developer Mac (\`~/.mqtt/plant-broker.conf\` with environment variables); MQTT passwords no longer passed on command lines
- DL-032 updated with the credential workflow rationale
- Verification commands in \`hub/03-broker-config/README.md\` and DL-030 updated to use environment variables instead of literal credentials
