# Decision Log

This log records the engineering decisions made on the Plant Autonomy Testbed and the reasoning behind each. It is **append-only and chronological**: existing entries are never rewritten or deleted, even when superseded by a later decision.

Each entry has a unique ID (DL-NNN). When a later decision modifies or replaces an earlier one, the later entry explicitly says so, and the earlier entry's status is updated to reflect that.

The README and code describe *what* and *how*. This file documents *why*.

## Entry format

- **ID and title** — short, scannable
- **Date** — when the decision was made (or recorded, for retroactive baseline entries)
- **Context** — what prompted the decision
- **Decision** — what was chosen
- **Rationale** — why this option won
- **Status** — Active, Superseded by DL-XXX, or Reversed
- **Alternatives considered** — what was passed on, and why

## Index

| ID | Date | Title | Status |
|---|---|---|---|
| [DL-001](#dl-001) | 2026-05-16 | Plant choice: basil | Active |
| [DL-002](#dl-002) | 2026-05-16 | Pot size and growing medium | Active |
| [DL-003](#dl-003) | 2026-05-16 | Sensor suite | Active |
| [DL-004](#dl-004) | 2026-05-16 | Actuator switching: MOSFET, not relay | Active |
| [DL-005](#dl-005) | 2026-05-16 | Telemetry hub and stack | Active |
| [DL-006](#dl-006) | 2026-05-16 | Firmware framework | Active |
| [DL-007](#dl-007) | 2026-05-16 | Vision as a separate node | Active |
| [DL-008](#dl-008) | 2026-05-16 | PCB deferred, protoboard as final form | Active |
| [DL-009](#dl-009) | 2026-05-25 | MOSFET part substitution: IRLZ44N → IRLB8721 | Active |
| [DL-010](#dl-010) | 2026-05-25 | Grow light strategy: LBW lamp + Shelly smart plug | Active |
| [DL-011](#dl-011) | 2026-05-25 | Protoboard deferred from immediate Phase 1 | Active |
| [DL-012](#dl-012) | 2026-05-25 | Power distribution discipline | Active |
| [DL-013](#dl-013) | 2026-05-25 | Leak sensor added as a fault input | Active |
| [DL-014](#dl-014) | 2026-05-25 | Reservoir construction and float switch mounting | Active |
| [DL-015](#dl-015) | 2026-05-26 | BME280 bench test validated | Active |
| [DL-016](#dl-016) | 2026-05-26 | Power architecture: keep buck converter | Active |
| [DL-017](#dl-017) | 2026-05-26 | LM2596 buck converter validated | Active |
| [DL-018](#dl-018) | 2026-05-27 | Peristaltic pump + IRLB8721 MOSFET driver validated | Active |
| [DL-019](#dl-019) | 2026-05-29 | BME280 re-validated on integrated bench | Active |
| [DL-020](#dl-020) | 2026-05-29 | Soil moisture sensor validated and calibrated | Active |
| [DL-021](#dl-021) | 2026-05-29 | BH1750 light sensor validated | Active |
| [DL-022](#dl-022) | 2026-05-29 | Grow-light control strategy: deferred | Active |
| [DL-023](#dl-023) | 2026-05-29 | Float switch validated and orientation mapped | Active |
| [DL-024](#dl-024) | 2026-05-30 | OLED display validated, three-device I²C bus confirmed | Active |
| [DL-025](#dl-025) | 2026-05-31 | User-feedback subsystem validated (LEDs, buttons, buzzer, state machine) | Active |
| [DL-026](#dl-026) | 2026-05-31 | Leak sensor validated and calibrated | Active |
| [DL-027](#dl-027) | 2026-05-31 | Hub bootstrap: Pi online, Mosquitto installed | Active |
| [DL-028](#dl-028) | 2026-05-31 | Campus deployment network deferred; project on home WiFi for now | Active |
| [DL-029](#dl-029) | 2026-05-31 | Mosquitto broker verified via loopback pub/sub | Active |
| [DL-030](#dl-030) | 2026-06-02 | Mosquitto broker configured for LAN access with authentication | Active |
| [DL-031](#dl-031) | 2026-06-02 | Shelly Plug US Gen4 ("basilplug") paired and joined JSU_DEVICE | Active |
| [DL-032](#dl-032) | 2026-06-02 | Shelly Plug US Gen4 validated as MQTT client; MQTT username renamed | Active |
| [DL-033](#dl-033) | 2026-06-03 | Pi power supply: CanaKit 5.1V/3.5A for permanent independent operation | Active |
| [DL-034](#dl-034) | 2026-06-03 | ESP32-CAM bench validation deferred pending hardware replacement | Active |
| [DL-035](#dl-035) | 2026-06-03 | Phase 3 hub services kickoff (Python listener, SQLite, Streamlit) | Active |
| [DL-036](#dl-036) | 2026-06-04 | Promote listener to systemd service; credentials in root-only EnvironmentFile | Active |
| [DL-037](#dl-037) | 2026-06-04 | Streamlit dashboard with light cream theme; UTC storage, America/Chicago display | Active |
| [DL-038](#dl-038) | 2026-06-04 | Tailscale for remote dashboard access; tailnet IP adopted as canonical URL | Active |
| [DL-039](#dl-039) | 2026-06-05 | Shelly unexpected reboot diagnosed from telemetry; "Restore last state" mitigation applied | Active |
| [DL-040](#dl-040) | 2026-06-05 | Phase 2 WROVER integrated firmware: architectural principles | Active |
| [DL-041](#dl-041) | 2026-06-06 | WROVER telemetry convention: topics, presence + Last Will, per-sensor payload, EAV projection | Active |
| [DL-042](#dl-042) | 2026-06-06 | Soil moisture: % mapping from 3-condition data, single far-from-inflow probe, closed-loop handles growth | Active |
| [DL-043](#dl-043) | 2026-06-06 | Float + leak sensors: report-only modules, fault interpretation deferred to FSM | Active |
| [DL-044](#dl-044) | 2026-06-06 | Third status LED (yellow, GPIO23): green/yellow/red traffic-light state indication | Active |
| [DL-045](#dl-045) | 2026-06-06 | Traffic-light LEDs bench-validated after rewire (green 18 / yellow 19 / red 23) | Active |
| [DL-046](#dl-046) | 2026-06-06 | Watering state machine: safety-first FSM, stubbed pump, validated on bench | Active |
| [DL-047](#dl-047) | 2026-06-06 | Buzzer alarm (leak only) + OLED status display, FSM-driven | Active |
| [DL-048](#dl-048) | 2026-06-06 | Pump flow calibrated at ~1.0 mL/s; daily cap set to 200 mL | Active |
| [DL-049](#dl-049) | 2026-06-06 | Real pump enabled (GPIO25); dosing tuned (~5 mL pulse / 10 s settle); autonomous loop validated | Active |
| [DL-050](#dl-050) | 2026-06-06 | Float switch mounting: carbon-fiber rod through lid, center-vertical | Active |
| [DL-051](#dl-051) | 2026-06-06 | Pot tube routing: drilled inlet for gentle surface delivery | Active |
| [DL-052](#dl-052) | 2026-06-06 | Dashboard state banner: FSM state surfaced via plant/state/wrover | Active |
| [DL-053](#dl-053) | 2026-06-06 | Watering-effectiveness watchdog: fault if soil doesn't respond to pumping | Active |
| [DL-054](#dl-054) | 2026-06-06 | Grow-light daily photoperiod (07:00\u201319:00) via Shelly device-side scheduler | Active |
| [DL-055](#dl-055) | 2026-06-06 | Device-silence detection: WROVER presence via Last-Will, offline-aware dashboard | Active |
| [DL-056](#dl-056) | 2026-06-08 | Stale-flag WROVER environment cards on the dashboard when offline | Active |
| [DL-057](#dl-057) | 2026-06-08 | Soil-moisture trend chart with watering episodes overlaid on the dashboard | Active |
| [DL-058](#dl-058) | 2026-06-09 | True mL daily cap + NTP calendar-midnight reset (rolling-window fallback) | Active |
| [DL-059](#dl-059) | 2026-06-10 | Hub-side timeout watchdog: detect a device silent but still connected | Active |
| [DL-060](#dl-060) | 2026-06-10 | Reboot detection from uptime resets, surfaced on the dashboard | Active |
| [DL-061](#dl-061) | 2026-06-10 | ntfy push alerting: faults, offline, flapping reboots, recoveries, daily heartbeat | Active |
| [DL-062](#dl-062) | 2026-06-11 | ESP32-CAM bring-up: prior failures; sketch 11 validated | Active |
| [DL-063](#dl-063) | 2026-06-12 | Grow-light photoperiod verification (lux vs schedule); threshold in data-collection phase | Active |
| [DL-064](#dl-064) | 2026-06-12 | External-watering detection: flag a soil rise the pump didn't cause | Active |
| [DL-065](#dl-065) | 2026-06-12 | Fix timestamp-windowing in hub time-range queries (string compare → julianday) | Active |
| [DL-066](#dl-066) | 2026-06-14 | Fix listener crash-loop & dashboard DB-locks (WAL + loop hardening + busy_timeout) | Active |
| [DL-067](#dl-067) | 2026-06-14 | Make windowed queries sargable (use existing (sensor,ts)/(device,ts) indexes) | Active |
| [DL-068](#dl-068) | 2026-06-14 | Nightly DB retention — prune high-frequency tables to rolling windows | Active |
| [DL-069](#dl-069) | 2026-06-15 | Validate grow-light lux threshold (30) against 10 days of data — confirmed | Active |
| [DL-070](#dl-070) | 2026-06-15 | Instrument Shelly uptime/RSSI — separate reboots from WiFi dropouts | Active |
| [DL-071](#dl-071) | 2026-06-15 | Power-chart gridlines + land the DL-067 sargable code (omitted from 5d717e6) | Active |
| [DL-072](#dl-072) | 2026-06-15 | Housekeeping — unify Shelly device label to grow-light; dedup import; gitignore handoff | Active |
| [DL-073](#dl-073) | 2026-06-15 | Lower soil watering trigger to ~30% (was ~21.6%) | Active |
| [DL-074](#dl-074) | 2026-06-16 | Hub-side photoperiod enforcement — self-heal Shelly reboots/misfires | Active |
| [DL-075](#dl-075) | 2026-06-17 | Shelly reboot cause investigation — not WiFi/RAM/power; flaky unit, masked by DL-074 | Active |
| [DL-076](#dl-076) | 2026-06-19 | Camera vision arc, slice 1 — Pi image receiver: HTTP bytes, ExG greenness, no OpenCV | Active |
| [DL-077](#dl-077) | 2026-06-19 | XIAO ESP32-S3 Sense vision-node bring-up validated (PSRAM, OV3660, frames); data-cable lesson | Active |
| [DL-078](#dl-078) | 2026-06-20 | Camera node v1 — capture + HTTP POST to the Pi receiver, validated end-to-end | Active |
| [DL-079](#dl-079) | 2026-06-21 | Receiver dual-metric: greenness on largest green blob (self-locating), green_area + green_ratio | Active |
| [DL-080](#dl-080) | 2026-06-21 | Deployment capture resolution: UXGA chosen (SVGA/UXGA/QXGA compared); evidence images committed | Active |
| [DL-081](#dl-081) | 2026-06-21 | Deployment firmware: UXGA + jpeg_quality 10 + hourly cadence; photoperiod gating left to the receiver | Active |

---

## Part 1 — Project Baseline (Project Definition v2.0, 16 May 2026)

These entries record the original decisions from the project definition so the log is self-contained — a reader can understand the project from this file alone.

<a id="dl-001"></a>
### DL-001 — Plant choice: basil

**Date:** 2026-05-16 · **Status:** Active

**Context.** The project needs a plant that gives fast visible feedback for control-loop tuning, has real-world parallel, is hardy enough to survive bench testing, and fits a single small pot.

**Decision.** Basil (*Ocimum basilicum*).

**Rationale.** Fast growth means the system's effectiveness shows quickly. Basil is a common commercial indoor crop, giving the project real-world parallel. It is hardy enough for an imperfect first build, edible, and fits a 5–6" pot.

**Alternatives considered.** Succulents (too slow for control-loop feedback). Snake plant or peace lily (too large, slow growth). Mint (invasive). Parsley (slow germination).

---

<a id="dl-002"></a>
### DL-002 — Pot size and growing medium

**Date:** 2026-05-16 · **Status:** Active

**Context.** Pot size affects both root development and the dynamics of the soil-moisture control loop. Too small and sensor noise dominates; too large and the soil never reaches the sensor in a meaningful way.

**Decision.** 5–6" pot with mandatory drainage hole. Plain indoor potting mix, no moisture-retention additives.

**Rationale.** 5–6" smooths moisture dynamics enough to give the control loop margin while still being small enough that under-rooted soil does not stay waterlogged. The drainage hole prevents root rot. Plain mix means the sensor reads actual water content rather than the buffered reservoir of polymer crystals.

**Alternatives considered.** 4–5" pot (twitchier control problem). 6–8" pot (oversized for basil). Moisture-control soil with water-retention crystals (would fight the control system).

---

<a id="dl-003"></a>
### DL-003 — Sensor suite

**Date:** 2026-05-16 · **Status:** Active

**Context.** The system needs to perceive soil moisture, air conditions, light, and reservoir level.

**Decision.** Capacitive soil moisture sensor; BME280 for air (temperature, humidity, pressure); BH1750 for light (calibrated lux); float switch for reservoir level.

**Rationale.** Capacitive sensors do not corrode (resistive probes do, defeating long-term reliability). BME280 covers three useful air parameters on one I²C device. BH1750 returns calibrated lux and covers the full indoor range. A float switch is the simplest reliable low-water detector.

**Alternatives considered.** Resistive soil sensor (corrodes). DHT22 air sensor (no pressure, lower precision). TSL2591 (wider light range, judged unnecessary indoors). Ultrasonic water-level sensor (more complex than required for a binary low-water signal).

---

<a id="dl-004"></a>
### DL-004 — Actuator switching: MOSFET, not relay

**Date:** 2026-05-16 · **Status:** Active. See also DL-009 (specific part) and DL-010 (grow light no longer uses this method).

**Context.** The pump and grow light both need to be switched by the ESP32. The two main options are mechanical relays and logic-level MOSFETs.

**Decision.** Logic-level N-channel MOSFETs in a low-side configuration, with a flyback diode across the pump.

**Rationale.** MOSFETs are silent (no relay click), have no mechanical wear, switch fast enough for PWM dimming if needed, and tolerate frequent cycling. Relays would have meant audible clicking every watering cycle and limited operational life.

**Alternatives considered.** Mechanical relay (noise, wear, no PWM). Solid-state relay (overkill for low-voltage DC). Direct GPIO drive (insufficient current).

---

<a id="dl-005"></a>
### DL-005 — Telemetry hub and stack

**Date:** 2026-05-16 · **Status:** Active

**Context.** The system needs remote telemetry, persistent logging, a dashboard, and push alerts. Options span paid cloud SaaS to fully self-hosted local.

**Decision.** Raspberry Pi 4 hub running Mosquitto (MQTT broker), a Python data service, SQLite database, and a Streamlit web dashboard. Push alerts via ntfy.sh; non-urgent summaries by email.

**Rationale.** The Pi 4 is already owned; the stack has no recurring cost and no message limits. MQTT is the industry-standard telemetry protocol, so the skills are transferable. Streamlit gives a browser dashboard with minimal code. ntfy.sh is free and self-hostable.

**Alternatives considered.** AWS IoT or Azure IoT Hub (recurring cost, vendor lock-in). Blynk or Adafruit IO (free tier message limits). Custom mobile app (out of scope for a portfolio piece).

---

<a id="dl-006"></a>
### DL-006 — Firmware framework

**Date:** 2026-05-16 · **Status:** Active

**Context.** ESP32 firmware can be written against ESP-IDF (native), the Arduino framework, or in MicroPython.

**Decision.** C++ on the Arduino framework, built and managed by PlatformIO.

**Rationale.** Arduino keeps the firmware approachable while running bare-metal. PlatformIO pins library versions in `platformio.ini` (in the repo), giving reproducible builds — anyone who clones the repo gets an identical environment. This matters for a project whose goal includes replicability.

**Alternatives considered.** ESP-IDF (more capable, harder to replicate). MicroPython (slower, less suited to real-time sensor reads). Arduino IDE without PlatformIO (no environment lock — versions drift across machines).

---

<a id="dl-007"></a>
### DL-007 — Vision as a separate node

**Date:** 2026-05-16 · **Status:** Active

**Context.** The system needs a camera for time-lapse and basic visual health observation. The main ESP32 already manages many sensors and actuators.

**Decision.** ESP32-CAM as a dedicated second node for capture and upload. All image processing happens on the Pi.

**Rationale.** The ESP32-CAM uses most of its GPIO pins for the camera bus and cannot simultaneously drive sensors and actuators. Dedicating it keeps responsibilities clean (single-responsibility per node) and matches the broader architectural principle of separation of concerns. Pi-side processing has access to more compute and Python's image ecosystem.

**Alternatives considered.** Camera on the main ESP32 (impossible — pin conflict). USB camera on the Pi (no — vision should be at the plant). On-device ML on the ESP32-CAM (deferred to v2 per the project definition).

---

<a id="dl-008"></a>
### DL-008 — PCB deferred, protoboard as final form

**Date:** 2026-05-16 · **Status:** Active. Refined further by DL-011 (deferred even out of Phase 1).

**Context.** The final assembly needs to be cleaner than a breadboard for a portfolio-quality demonstration.

**Decision.** A clean protoboard build is the acceptable final form. A custom KiCad PCB is optional, pursued only if time allows.

**Rationale.** Protoboard transcription of a verified breadboard gives a professional finish without the PCB design cycle (schematic, layout, fabrication, assembly, debug). The engineering content of this project is in the firmware, system design, and autonomy logic — not the board layout — so PCB time is not the highest-leverage investment.

**Alternatives considered.** Stay on breadboard (looks unfinished). PCB now (significant time cost). Wire-wrap (legible but slow).

---

## Part 2 — Phase 1 Refinements (25 May 2026)

These entries record decisions made after the project definition was published, during procurement and bench-test planning. Where they modify the PDF baseline, that supersession is called out explicitly.

<a id="dl-009"></a>
### DL-009 — MOSFET part substitution: IRLZ44N → IRLB8721

**Date:** 2026-05-25 · **Status:** Active. Supersedes the IRLZ44N specification in PDF §2 and §7.

**Context.** The PDF specifies two IRLZ44N MOSFETs. The MOSFET actually ordered (Adafruit's standard logic-level N-channel) is the IRLB8721.

**Decision.** Use IRLB8721 as the pump driver. Pin order, gate resistor, and flyback-diode configuration are identical to the original IRLZ44N plan.

**Rationale.** IRLB8721 is also a logic-level N-channel MOSFET fully turned on at 3.3 V gate drive. Rds(on) at logic-level drive is actually lower than IRLZ44N (~16 mΩ vs ~22 mΩ), giving slightly less heat dissipation. Pinout (G/D/S) is identical. No design changes required.

**Alternatives considered.** Order IRLZ44N separately (extra shipping and delay). Use a dedicated MOSFET driver IC (overkill at this current).

---

<a id="dl-010"></a>
### DL-010 — Grow light strategy: LBW lamp + Shelly smart plug

**Date:** 2026-05-25 · **Status:** Active. **Supersedes** the grow light specification in PDF §2 (Concluded Decisions), §6 (Wiring — grow-light MOSFET on GPIO26), and §7 (BOM grow light entry).

**Context.** The PDF specified a full-spectrum 12 V DC LED grow light panel switched and PWM-dimmed by a MOSFET, with Daily Light Integral (DLI) ramping for graceful supplementation. The grow light actually purchased is an AC-powered floor lamp with an integrated driver and a mechanical on/off switch (LBW Grow Light with adjustable tripod stand, 15–48").

**Decision.** Keep the LBW lamp. Control it via a Shelly smart plug speaking MQTT to the existing Mosquitto broker on the Pi. The 12 V DC MOSFET-controlled panel is deferred to "later, if needed."

**Rationale.** The LBW lamp cannot be MOSFET-switched or PWM-dimmed because its driver is internal and runs on AC mains. The Shelly smart plug speaks MQTT natively, so it integrates cleanly into the existing telemetry stack: the ESP32 (or the Pi-side orchestrator) publishes a `light/cmd` topic, the Shelly subscribes and acts. The DLI control logic adapts from a smooth PWM ramp to an on-until-target-then-off pattern — the requirement (supplement light when the day's accumulated DLI will fall short) is still satisfied, just with coarser control granularity.

**Alternatives considered.** Return the LBW lamp and order a 12 V DC panel (restores original design, but discards a part already in hand). SSR or mechanical relay on the AC line (works, but adds AC mains complexity to the build — undesirable for a portfolio piece unless the demonstration intent is AC switching). Keep the LBW lamp manual or always-on (defeats the autonomy framing for the lighting subsystem).

---

<a id="dl-011"></a>
### DL-011 — Protoboard deferred from immediate Phase 1

**Date:** 2026-05-25 · **Status:** Active. Refines DL-008.

**Context.** The PDF (§7 BOM, §8 Phase 1) included the protoboard in the Phase 1 shopping list. On reflection, breadboard testing is sufficient until integration begins, and protoboard transcription belongs to a later phase.

**Decision.** Remove the protoboard from the immediate Phase 1 BOM. Treat it as deferred or optional, to be revisited when Phase 2 integration begins (if at all).

**Rationale.** Phase 1's goal is component validation, not permanence. Breadboard is faster to iterate on, easier to rewire when a sensor needs to move. Protoboard transcription belongs to Phase 2, when the integrated wiring is stable and ready to be made semi-permanent.

**Alternatives considered.** Build to protoboard from day one (slows iteration). Skip protoboard entirely in favor of PCB (re-opens the PCB time-cost discussion).

---

<a id="dl-012"></a>
### DL-012 — Power distribution discipline

**Date:** 2026-05-25 · **Status:** Active. Refines PDF §6 (Wiring).

**Context.** The PDF describes the power tree at a logical level (12 V → buck → 5 V to ESP32; 12 V → pump and light) but does not specify the physical hardware for cleanly splitting the 12 V rail among multiple loads, nor does it state explicitly that actuator current must not flow through the breadboard or the ESP32 board.

**Decision.** Use WAGO / lever-nut connectors for clean 12 V rail and ground distribution. Explicit ground-discipline rule: all grounds (12 V supply, MOSFET source, buck converter output, ESP32) are tied together at a single common ground. The ESP32 only sources gate signals; actuator current never flows through breadboard traces or through the ESP32 board's regulator.

**Rationale.** WAGO lever connectors are professional, reusable, and far more reliable than twisted-wire splices or breadboard rails for the kind of current the pump and grow-light circuits carry. Stating the ground-discipline rule in writing makes it inspectable and replicable. Forgetting common ground is a classic bug; documenting it prevents it.

**Alternatives considered.** Soldered splices (less reusable, harder to reconfigure). Terminal strip (acceptable, bulkier). Running actuator current through the breadboard (unsafe — breadboard contact resistance and current limits make this risky).

---

<a id="dl-013"></a>
### DL-013 — Leak sensor added as a fault input

**Date:** 2026-05-25 · **Status:** Active. Extends PDF §4 (Control State Machine, Fault state).

**Context.** The PDF defines fault detection for sensor failure and closed-loop verification of watering, but does not include a sensor for detecting water *outside* the reservoir — leaks, overflow, or burst tubing.

**Decision.** Add a water-detection pad (leak sensor) under the reservoir/pump area, wired as a digital input to the ESP32. Firmware behavior: wet detected outside the reservoir → immediately stop the pump → enter Fault state → alert the operator.

**Rationale.** This fits the project's engineering framing directly. Detecting a leak before it floods is exactly the kind of failure mode an autonomous system should handle gracefully. The cost is small, the safety upside is significant, and it makes the closed-loop fault detection story stronger.

**Alternatives considered.** No leak sensor (the original PDF position — relies on tubing not failing). Pressure sensor on the reservoir (more complex, less direct signal). Camera-based leak detection (deferred — falls in the ML scope for v2).

---

<a id="dl-014"></a>
### DL-014 — Reservoir construction and float switch mounting

**Date:** 2026-05-25 · **Status:** Active. Refines PDF §5 (Physical Setup) and §7 (BOM — reservoir entry).

**Context.** The PDF specifies a ~2 L food-grade reservoir with a float switch mounted through the lid or wall, but does not detail tube routing, float-switch mount placement, or the relationship between the lid and the internal components.

**Decision.** Select a food-safe reservoir container with a drillable lid, target cost under ~$15. Drill the lid for: (1) the pump inlet tube, which extends down to reach near (but not flush against) the bottom of the reservoir, and (2) the float switch, which hangs from the lid into the water. The pump outlet tube routes to the pot. The float switch's NO/NC orientation is tested with a multimeter before firmware logic is committed.

**Rationale.** A drilled lid keeps the assembly clean, contains splashes, suppresses evaporation, and provides a stable mount point for the float switch. Inlet tube reaching near the bottom (but not flush) lets the pump draw nearly all the water without the tube end being blocked by the container floor. Testing NO/NC orientation up front prevents inverted-logic bugs in firmware.

**Alternatives considered.** Open reservoir with side-mounted float (less clean, more evaporation, weaker float mount). Inlet tube fixed flat against the bottom (flow blockage risk). Skip the lid entirely (no float mount, evaporation, debris ingress).

---

<a id="dl-015"></a>
### DL-015 — BME280 bench test validated

**Date:** 2026-05-25 · **Status:** Active

**Context.** Phase 1 component validation requires every sensor to be exercised individually before integration. BME280 is the first sensor under test.

**Decision.** BME280 bench test passes. The sensor is detected on the ESP32's default I²C bus, returns plausible indoor temperature, humidity, and pressure readings, and responds correctly to a breath stimulus (temperature and humidity both rise within seconds and recover to baseline within ~60 seconds). The sensor is approved for integration.

**Rationale.** The sketch confirms: (1) the I²C bus is wired correctly and the auto-detect across addresses 0x76/0x77 works as written; (2) the Adafruit library returns calibrated values, not raw counts; (3) values pass plausibility-bound checks (no `[WARN: implausible reading]` lines); (4) the sensor is not stuck — the breath response proves real-time measurement, not a constant fake value. This last point matters more than the absolute accuracy: a sensor that always reads "24°C, 45%" looks healthy but is useless to a control loop.

**Alternatives considered.** None — this is a validation outcome, not a design choice. Evidence: `docs/images/bme280-validation.png`.

---

<a id="dl-016"></a>
### DL-016 — Power architecture: keep buck converter ahead of the extension board

**Date:** 2026-05-26 · **Status:** Active

**Context.** The Freenove ESP32-WROVER kit ships with an extension board that the WROVER module plugs into. The extension board has a DC jack and an onboard regulator that handles input across a range (5–12V, verified by direct test — plugging the 12V adapter straight into the DC jack powers the system and it runs). All power to the ESP32-WROVER module flows through the extension board, not into the WROVER module directly.

This made the buck converter genuinely optional. The 12V supply could be tied directly to the extension board's DC jack and the system would work. The question was whether the buck still earns its place in the design or should be removed.

**Decision.** Keep the buck converter. The 12V supply feeds the buck; the buck's 5V output feeds the extension board's DC jack; the extension board powers the ESP32-WROVER module and produces EXT 3.3V for any 3.3V peripherals.

**Rationale.**

1. **Thermal.** The extension board's onboard regulator is the type of linear regulator that dissipates the voltage difference as heat. With 12V at its input and 3.3V at its output, the regulator drops 8.7V — at the ESP32's typical ~150 mA draw, that is over a watt of heat in a small package. Run a buck ahead of the extension board and the regulator only has to handle the 5V → 3.3V drop, which is roughly five times less power to dissipate. The buck itself converts at ~85–90% efficiency regardless of input voltage, so the heat comes off the regulator without going somewhere worse.

2. **Reliability under transient load.** The ESP32 draws sudden current spikes during WiFi transmits (up to ~500 mA). Onboard linear regulators stretched across a large input-output gap are more brownout-prone under those spikes than the same regulator running off a closer-to-target input. Brownouts manifest as random resets and unreliable WiFi — exactly the class of failure that undermines an autonomy system.

3. **The buck is already paid for and validated.** It's owned, on the bench, and passed its Phase 1 validation (see DL-017). Removing it from the design now would not save any time, money, or board space worth caring about.

4. **It is the standard pattern for mixed-voltage embedded systems.** A single high-voltage input (12V in our case, to drive the pump) branching through a buck to a logic-level rail is what reviewers expect to see. Tying the high-voltage rail directly to a dev board with an onboard regulator is functionally fine but reads as a corner-cut.

**Alternatives considered.** Tie 12V directly to the extension board's DC jack (rejected for the thermal and reliability reasons above — the system would run, just hotter and more brownout-prone). Use a 5V wall adapter for the extension board and a separate 12V adapter for the pump (works, but adds a second wall wart, a second point of failure, and forfeits the single-supply intent). Power the extension board over USB during the build and a separate 12V for actuators (fine for the bench, but doesn't generalize to the deployed system).

---

<a id="dl-017"></a>
### DL-017 — LM2596 buck converter validated

**Date:** 2026-05-26 · **Status:** Active

**Context.** Phase 1 component validation for the LM2596 step-down converter that supplies 5V to the extension board's DC jack from the 12V system rail (see DL-016 for the power-architecture decision).

**Decision.** LM2596 bench test passes. The converter is approved for integration into the power tree.

**Rationale.** Bench measurements:

- Input voltage from 12V adapter: **12.34V**
- Output voltage before trim adjustment: **11.5V** (significantly above target — see safety note below)
- Output voltage after trim adjustment: **5.0V**
- Stability over 30 seconds, no load: no drift observed
- Power-cycle recheck (unplug, replug adapter): output returned to 5.0V — trim setting holds
- Load test with 220Ω resistor (~23 mA at 5V), measured with the load connected: no voltage sag observed under load

**Safety observation worth recording.** The pre-adjustment output of 11.5V confirms the standard practice of trimming the output to target *before* connecting any load. Connecting the extension board's DC jack to an uncalibrated 11.5V output would have placed the extension board's onboard regulator under a much larger voltage drop than the design intends — well inside its tolerated input range, but exactly the high-dissipation, brownout-prone condition the buck exists to avoid (see DL-016). The procedure of adjust-then-load remains correct discipline, but the failure mode it prevents here is reliability degradation, not exceeding spec. The procedure of adjust-then-load is now part of the project's bench-test discipline.

**What this test verified.** The LM2596 module is functional, the trim pot is adjustable and holds its setting across power cycles, the output is stable under no-load and at a small (~23 mA) load, and the 12V adapter delivers within spec at the buck's input.

**What this test did not verify** (deferred to Phase 2 integration). Behavior under the realistic load of the ESP32-WROVER plus extension board plus any peripherals on the EXT 3.3V rail — that includes the ~500 mA WiFi-transmit current spikes. Cross-load behavior when the pump is also drawing from the 12V rail upstream of the buck. Long-term thermal behavior under sustained load. The LM2596 is rated to 3A and the 12V adapter to 3A, so these should be well within spec, but they are not yet directly measured.

**Alternatives considered.** None — this is a validation outcome, not a design choice.

---

<a id="dl-018"></a>
### DL-018 — Peristaltic pump and IRLB8721 MOSFET driver validated

**Date:** 2026-05-26 · **Status:** Active

**Context.** Phase 1 component validation for the actuator driver pattern that the entire system depends on. The IRLB8721 N-channel MOSFET switches a 12V peristaltic pump from a 3.3V GPIO signal, with a 1N4007 diode catching the inductive turn-off spike. This is the canonical pattern for any future 12V actuator added to the system.

**Decision.** Pump and MOSFET driver bench test passes. The subsystem is approved for integration.

**Rationale.** Bench observations across three or more full ON/OFF cycles:

- Pump runs cleanly during commanded ON intervals; silent during commanded OFF intervals
- Stop transitions are clean — no continued dribble or coast after the OFF command, confirming the flyback diode is catching the inductive spike as intended
- ESP32 does not reset across pump transitions, confirming common ground is correctly established and the 12V load is electrically isolated from the ESP32's power domain
- IRLB8721 runs warm but not hot under continuous cycling — consistent with low Rds(on) at logic-level gate drive and the modest pump current (~100 mA)
- Wet test (suction tube in water vessel, output tube to empty container) confirms the pump actually moves water and the direction is consistent with the polarity wiring

**On flow rate.** Observed flow rate is slow. This is expected and correct: the Adafruit peristaltic pump is specified at roughly 100 mL/min at 12V. Peristaltic pumps trade flow rate for dose precision, which is the right tradeoff for the system's actual use case ("deliver N mL of water on a watering decision"). A faster pump would deliver more water per second but with less control over total dose volume.

**Pre-power continuity verification.** Before power was applied, the wiring was verified by a continuity-test matrix covering common ground, gate signal path through the 220Ω series and 10kΩ pull-down resistors, drain path through the pump, diode orientation (forward voltage measured at ~0.596V — within normal range for a silicon diode), and absence of a 12V-to-ground short. An apparent +12V-to-drain short was correctly diagnosed as conduction through the pump's motor winding, not a wiring fault — verified by disconnecting one pump lead and re-running the relevant check. This diagnostic discipline (verify continuity before power, distinguish real shorts from coil-mediated paths) is now part of the project's bench-test practice. Evidence: `docs/images/pump-mosfet-validation.png`.

**What this test did not verify** (deferred to Phase 2 integration). Closed-loop verification — running the pump and confirming via the soil moisture sensor that water actually reached the soil. Dose calibration — measuring mL delivered per second of runtime so the firmware can convert a target volume into a runtime. Long-duration thermal behavior of the MOSFET under realistic duty cycles. Behavior when the buck converter is sharing the 12V rail under load simultaneously with the pump.

**Alternatives considered.** None — this is a validation outcome, not a design choice.

---

<a id="dl-019"></a>
### DL-019 — BME280 re-validated on integrated bench

**Date:** 2026-05-29 · **Status:** Active. Re-validates DL-015 after physical re-mounting.

**Context.** Phase 1 was originally tested component-by-component on a clear breadboard, with the BME280 removed after its first validation (DL-015). The current Phase 1 approach is to grow a single integrated test bench organically: each new component is wired onto the same breadboard alongside the previously-validated ones, tested with its own sketch, and left in place. This makes the breadboard the natural integration platform by the end of Phase 1, without a "big bang" reorganization step.

For this approach to be sound, re-mounting a previously-validated component must not silently regress its behavior. The BME280 was re-wired onto the breadboard already populated by the pump + MOSFET driver and the LM2596 buck converter, and re-validated against the same criteria as DL-015.

**Decision.** BME280 re-validation passes. The sensor remains approved for integration, now permanently placed on the integrated test bench.

**Rationale.** The 01-bme280 sketch was re-flashed unchanged. The sensor was detected at its I²C address, returned plausible indoor readings, and responded to a breath stimulus exactly as in DL-015 — no behavioral regression from being physically re-mounted alongside an actuator subsystem.

**Why this matters.** Confirms the integration approach is sound: previously-validated subsystems can coexist with newly-added components without losing their validation status. Each new component going forward can be added to the same breadboard with confidence that the existing components stay healthy.

**What this did not verify.** Behavior of the BME280 while the pump is actively cycling on the same breadboard — that's a true co-operation test and is deferred until enough subsystems are present to make it meaningful (likely after soil moisture, BH1750, and OLED join the bench).

---

<a id="dl-020"></a>
### DL-020 — Soil moisture sensor validated and calibrated

**Date:** 2026-05-29 · **Status:** Active

**Context.** Phase 1 component validation for the capacitive soil moisture sensor. Unlike a digital sensor, an analog sensor's raw reading has no inherent meaning until it is calibrated against known conditions in the specific physical setup (this sensor, this soil mix, this insertion depth). The bench test therefore combines validation (does it work?) with initial calibration (what does the data actually mean?).

**Decision.** Soil moisture sensor passes validation. Initial calibration values recorded below. The sensor is approved for integration. Final threshold values will be refined in Phase 2 when observing the sensor in the actual plant pot over real watering cycles.

**Rationale.** Sensor wired to GPIO34 (ADC1, input-only, WiFi-safe). The test sketch reads at 1 Hz with 20-sample averaging per read; this reduced jitter to ~15 ADC counts (~0.4% of full scale) across all three conditions — well below the operational range of interest. Readings monotonically decrease as moisture increases, consistent with capacitive-sensor physics. The dry-air, dry-soil, and wet-soil conditions are well-separated, confirming the sensor distinguishes between the relevant physical states.

**Calibration data** (potting mix from current bag, room temperature, probe at working depth, container without drainage):

| Condition | Raw ADC range | % of ADC scale |
|---|---|---|
| Dry air (probe in hand, no medium) | 2846 – 2862 | 69.5 – 69.9% |
| Dry soil (potting mix, untreated) | 2518 – 2528 | 61.4 – 61.9% |
| Wet soil (~30 min post-watering, distributed) | 1946 – 1960 | 47.3 – 48.1% |

**Implications for firmware.**
- The operational range for the watering decision is approximately **2523 (dry) → 1953 (wet)**, a span of ~570 counts.
- A midpoint threshold (~2238) would mean "water when soil is halfway between dry and wet" — likely too late for a real plant.
- A more realistic Phase 2 starting threshold is approximately **2350–2400**, corresponding to roughly 25–30% of the way from wet to dry — i.e. water when the soil has dried out about a third of the way. This will be refined empirically by observing the actual plant's daily moisture cycles in Phase 2.
- Hysteresis around the threshold (e.g. trigger at 2400, stop at 2200) is recommended to prevent toggling near the threshold. Noise of ~15 counts is small enough that a 200-count hysteresis band is more than sufficient.

**What this test did not verify.** Long-term sensor drift over weeks of use. Behavior with the actual plant's root system in place (roots affect soil structure and water distribution near the probe). Temperature dependence (capacitive sensors have some thermal coefficient). Behavior during the transient period immediately after watering, before water has distributed. All of these are addressed in Phase 2 observation.

**Alternatives considered.** None for the validation outcome itself. For the calibration methodology, using the actual plant pot was considered and rejected — calibrating in the plant pot would require driving the live plant to both moisture extremes (dry stress and saturation), which is unacceptable. A separate container with the same soil mix gives valid calibration that transfers to the plant pot because the dielectric properties depend on the soil composition, not the container.

---

<a id="dl-021"></a>
### DL-021 — BH1750 light sensor validated

**Date:** 2026-05-29 · **Status:** Active

**Context.** Phase 1 component validation for the BH1750 ambient light sensor. The BH1750 returns calibrated lux directly — no analog-style calibration is required for the sensor itself. The test was also an opportunity to verify that the I²C bus tolerates two devices (BME280 at 0x76, BH1750 at 0x23) sharing SDA and SCL without interference.

**Decision.** BH1750 passes validation. The sensor is approved for integration.

**Rationale.** Sensor wired to the existing I²C bus on the breadboard alongside the BME280. Both devices respond to their respective addresses; neither interferes with the other. The BH1750 returns plausible lux values that respond correctly to light changes — covering the sensor drops the reading toward zero, exposing it to a strong light source raises the reading proportionally.

**Bench observations** (room with fluorescent overhead lighting):
- Ambient with room lights on: ~6.7 lux
- Hand covering sensor: < 6 lux
- Room lights off (sensor sees only stray light): ~0 lux ("dark")
- Grow light brought near sensor: > 50 lux

The sensor responds correctly to relative changes — the validation criterion for Phase 1.

**On the classifier labels.** The sketch includes a coarse classifier (`dark / dim / room / bright / very bright`) with thresholds taken from generic indoor-lighting reference tables (typical incandescent or LED-lit residential rooms in the 100–500 lux range). The test environment's actual measured ambient is ~6.7 lux under fluorescent overhead lighting — substantially lower than generic indoor references because fluorescent fixtures often produce lower task-surface illuminance than incandescent fixtures of comparable perceived brightness, and because the sensor's orientation reads reflected rather than direct light. The classifier labels are therefore not well-calibrated to this specific environment, but this does not affect sensor validation: the labels are debug-print color only and the firmware does not act on them.

**Sensor's role under the revised grow-light architecture.** When the project was originally designed, the BH1750 was intended to drive smooth PWM dimming of a MOSFET-controlled 12V DC grow light via Daily Light Integral (DLI) ramping. DL-010 changed the grow light to an AC fixture controlled by a Shelly smart plug over MQTT, which is binary — the light is either on or off. This does not retire the BH1750 from the project. The sensor is still what informs the decision *when* to command the smart plug on or off: the firmware integrates lux over the day, compares the accumulated DLI against basil's target, and triggers the grow light when natural light is falling short. The control granularity is coarser (on/off rather than continuous dimming), but the decision logic is still data-driven by the BH1750 rather than running on a dumb time-of-day schedule.

**Implications for Phase 2 firmware.** The grow-light supplementation logic requires a real lux threshold below which supplementation triggers. That threshold cannot be set from generic reference tables given the observed mismatch with this environment. It must be set empirically by recording lux readings across a representative day-cycle (morning, midday, evening) and during the desired grow-light operating window, in the actual final sensor mounting position (height, orientation, distance from windows and overhead fixtures). This is a Phase 2 task and is explicitly deferred.

**What this test did not verify.** Long-term sensor stability. Behavior in direct sunlight (the sensor has multiple measurement modes for different ranges; the current sketch uses high-resolution continuous mode, suitable for indoor lighting but may saturate near 100,000 lux). Behavior during rapid light transitions (sensor has settling time). Multi-day drift. These are addressed by Phase 2 observation in the deployed environment.

**Alternatives considered.** TSL2591 was the rejected alternative at the project-definition stage for wider range and higher sensitivity; the BH1750 was kept as sufficient for this project's indoor-only use case. This validation confirms the BH1750 choice is appropriate for the operating environment.

---

<a id="dl-022"></a>
### DL-022 — Grow-light control strategy: deferred pending operating-environment characterization

**Date:** 2026-05-29 · **Status:** Active. Deferred — to be resolved before Phase 2 firmware integration.

**Context.** With the grow-light architecture revised in DL-010 (AC fixture controlled via Shelly smart plug, binary on/off) and the BH1750 validated in DL-021, the next question is how Phase 2 firmware should decide when to command the grow light on or off. Two strategies are on the table, and the choice depends on factors that cannot be evaluated until the system is observed in its actual operating environment.

**Decision.** Defer the choice between closed-loop DLI control and scheduled operation until the operating environment is better characterized. Both strategies remain viable. Phase 2 will start with whichever is appropriate based on observations made closer to deployment.

**The two candidate strategies.**

**Strategy A — Closed-loop DLI control.** Firmware integrates measured lux from the BH1750 over the day, compares the accumulated daily light integral against basil's target, and commands the grow light on when accumulated light is falling short. Reactive to actual conditions; theoretically the more autonomous choice.

**Strategy B — Scheduled operation with BH1750 as verification.** Firmware runs the grow light on a fixed daily schedule (e.g. 7am–9pm) sized to give basil its target light hours. The BH1750 is used to (1) verify the grow light actually turned on when commanded (closed-loop fault detection — if lux does not rise after a grow-light-on command, the smart plug or fixture has failed), and (2) log ambient light data for portfolio purposes. Simpler, more predictable, less sensitive to environmental contamination.

**Why this is not yet a clear choice.**

1. **The operating environment is shared and uncontrolled.** The room has dimmable overhead lighting used by other occupants. Their lighting behavior cannot be predicted. The BH1750 cannot distinguish their light from natural light or from the grow light — it reads total ambient lux.
2. **Lux is not photosynthetically meaningful.** A given lux value from fluorescent overhead lighting delivers substantially less photosynthetic energy to a plant than the same lux from a dedicated grow light, because plants respond to specific wavelengths (PAR) that lux measurement does not weight for. The BH1750 cannot measure PAR; a PAR sensor would cost ~10×–20× more and is out of scope.
3. **The user has partial control over the operating environment.** During summer the room is less shared and lighting patterns are more predictable, making Strategy A potentially viable. During the academic semester the room is more variable and Strategy B becomes more defensible.

**What would force the decision.**
- If observations show the room's ambient lux varies wildly during the period the system will be deployed, Strategy B is the honest choice — the BH1750 readings would be too contaminated to drive reliable DLI control.
- If observations show the room's ambient lighting is reliably predictable during the deployment window (because the user controls the room or the deployment is short enough to plan around), Strategy A becomes viable.
- The decision can also remain pragmatic: start with Strategy B (simpler, lower risk), and migrate to Strategy A later only if observations of how the plant actually grows under scheduled lighting suggest a more reactive approach would help.

**Implications for hardware.** None. Both strategies use the same hardware (BH1750 + Shelly smart plug + grow light). The choice is firmware-only and can be revisited without any wiring changes.

**Implications for the BH1750's role.** The sensor stays in the build regardless of which strategy is chosen. Under Strategy A it is the primary input; under Strategy B it is the verification and logging input. Either way, the validation in DL-021 stands.

**Alternatives considered.** Hybrid (schedule + BH1750 trims the schedule) — possible, but adds complexity without clearly outperforming either of the simpler strategies in the user's mixed-lighting environment. Same deferral logic applies.

---

<a id="dl-023"></a>
### DL-023 — Float switch validated and orientation mapped

**Date:** 2026-05-29 · **Status:** Active

**Context.** Phase 1 component validation for the reservoir float switch. Float switches are sold without standardized polarity — the same physical state can produce either an open or closed circuit depending on how the manufacturer wired the internal reed switch. The bench test determines which orientation corresponds to which state for *this specific switch*, so the firmware's "is the reservoir low?" check can be written with correct polarity.

**Decision.** Float switch passes validation. The orientation-to-state mapping is recorded below and will be referenced when Phase 2 firmware reads the reservoir level input.

**Rationale.** Switch wired between GPIO27 (configured `INPUT_PULLUP`) and GND. With pull-up enabled, an open switch reads HIGH (3.3V via the internal pull-up) and a closed switch reads LOW (shorted to GND). Transitions were clean — no flickering when held still, immediate response on inversion. The test was conducted dry (no water) because the float's position is controlled by gravity alone; flipping the device between right-side-up and inverted produces the same internal magnet movement as water rising and falling in a reservoir.

**Orientation mapping for this specific switch.**

| Physical state | Real-world meaning | Switch state | GPIO27 reads |
|---|---|---|---|
| Threaded end up, float at bottom (gravity-pulled down) | Empty reservoir | CLOSED | LOW |
| Inverted, float at top (gravity-pulled up, simulating water lift) | Full reservoir | OPEN | HIGH |

This is the "normally closed" wiring convention (closed circuit when the float is at the bottom of the tube). The opposite convention also exists in commercial float switches, so the mapping must be explicit in firmware comments.

**Firmware implications.**
- `digitalRead(FLOAT_PIN) == LOW` → reservoir is empty → enter Fault state, alert operator, do not run pump
- `digitalRead(FLOAT_PIN) == HIGH` → reservoir has water → normal operation
- The polarity is the *opposite* of the intuitive "open means low water" assumption, so the firmware should include a clear comment block above the float read with the mapping table to prevent future bugs.

**What this test did not verify.** Behavior under actual water (the test was dry — flipping the device replaced rising water). Long-term reliability of the reed switch under repeated cycling. Behavior at marginal water levels (the float oscillating near the threshold position). These are addressed in Phase 2 with the switch mounted in the actual reservoir.

**Alternatives considered.** None — this is a validation outcome, not a design choice.

---

<a id="dl-024"></a>
### DL-024 — OLED display validated, three-device I²C bus confirmed

**Date:** 2026-05-30 · **Status:** Active

**Context.** Phase 1 component validation for the SSD1306-based 0.96-inch, 128×64 yellow/blue OLED display. The OLED is the first *output* device in the project (everything previously validated was a sensor or actuator) and is the third I²C device to join the shared bus after BME280 and BH1750. This test therefore validates two things simultaneously: the display itself, and the I²C bus's ability to host three devices without interference.

**Decision.** OLED passes validation. The display is approved for integration. The shared I²C bus is confirmed healthy with three devices attached.

**Rationale.** OLED detected at default I²C address 0x3C on first try. The splash screen rendered correctly, transitioned cleanly to a mock dashboard. The dashboard renders the system title and phase label in the yellow zone (top 16 pixels — a fixed hardware property of this OLED variant, not a software setting) and mock sensor values plus a live uptime counter in the blue zone (bottom 48 pixels). The uptime counter increments once per second, confirming the display is being actively refreshed rather than locked on a static frame. No rendering artifacts, no flickering, no I²C bus errors.

**Bus integrity.** Three devices on the same SDA/SCL pair: BME280 (0x76 or 0x77), BH1750 (0x23), SSD1306 OLED (0x3C). All three remained reachable and functional during the OLED test. This is the first multi-device I²C validation in the project and de-risks the eventual integrated firmware, which will read all three devices in the same `loop()`.

**Mock dashboard layout** (preview of the Phase 2 status UI):

| Zone | Pixel range | Content |
|---|---|---|
| Yellow | y = 0..15 | Title "PLANT AUTONOMY"; subtitle "Phase 1 bench test" |
| Blue | y = 16..63 | Temp, humidity, light, soil, uptime + pump state (five lines) |

The values displayed are mocked, not live — that integration belongs to Phase 2. The layout choice reserves the yellow band for status-at-a-glance information (title now; later, an alert indicator or system state) and the blue band for the actual sensor data, which is the natural reading hierarchy for a yellow/blue split OLED.

**What this test did not verify.** Live data rendering (the values are hardcoded in the sketch — the integration with BME280/BH1750/soil sensor happens in Phase 2). Long-term burn-in characteristics of the OLED (OLED pixels age with use; static UI elements may show after months of operation, but this is a known property of the technology and is mitigated by occasional layout changes or low-power sleep). Behavior under voltage sag (the test was run with the buck-converted 5V supply through the extension board's 3.3V regulator, all known stable from prior validations).

**Alternatives considered.** None — this is a validation outcome, not a design choice.

---

<a id="dl-025"></a>
### DL-025 — User-feedback subsystem validated: LEDs, buttons, buzzer, and state machine

**Date:** 2026-05-31 · **Status:** Active

**Context.** The original PDF specified three status LEDs, three buttons, and a buzzer as a single subsystem for local user feedback and control. With the OLED already validated as the primary status display (DL-024) and the eventual MQTT dashboard available for remote control, the original specification was reconsidered against actual need before any wiring or code began. The goal: justify each component's presence in terms of a real fault mode or interaction scenario, rather than carrying components forward by default.

**Decision.** The user-feedback subsystem is reduced and validated in its revised form: **2 LEDs (green, red), 3 buttons (STOP, ACK, MANUAL), and 1 active buzzer**. All six components pass validation, both individually and together via a four-state state machine. The subsystem is approved for integration.

**Rationale for the revised scope.**

The driving principle is the "Waymo manual-override" scenario: in a real fault, the operator may not be present, and a remote helper (someone the operator phones from a distance) must be able to describe what they see and execute simple recovery actions on the operator's behalf. The subsystem is therefore scoped to that interaction, not to general convenience.

- **2 LEDs** — Green and red, deliberately distinct colors, easy to describe over the phone ("the red one is solid"). Two LEDs support five visually distinct states via blink patterns: green steady (IDLE), green slow blink (MEASURING), green fast blink (ACTION), red steady (FAULT), red fast blink (CRITICAL). The third LED was dropped — incremental information value did not justify the additional pin, resistor, and wiring complexity.
- **3 buttons** — Each maps to a specific fault-recovery action: STOP (latch the system into FAULT), ACK (acknowledge and clear FAULT or CRITICAL), MANUAL (override to trigger one watering cycle from IDLE). The MANUAL button is the bench-utility addition over the strict-minimum two-button design; it earns its place during Phase 2 testing and as a "make the plant get water now" affordance for the operator.
- **1 active buzzer** — The only audible channel. Chosen over the passive buzzer because the system needs a single fault alarm rather than melodies; the active type is louder at the same drive level. Driven through an NPN transistor (1 kΩ base resistor) to keep the GPIO from sourcing the buzzer's current directly; powered from the 5V rail rather than 3.3V to maximize loudness.

**Pin assignment changes.**

The original PDF allocated GPIO16 and GPIO17 for two of the LEDs. These pins are not broken out on the ESP32-WROVER variant — they are reserved internally for the WROVER's PSRAM. This was not caught at the project-definition stage and is corrected here. Working assignments:

| Component | Pin | Notes |
|---|---|---|
| Green LED | GPIO18 | output, via 270Ω to GND |
| Red LED | GPIO19 | output, via 270Ω to GND |
| Button A (STOP) | GPIO32 | INPUT_PULLUP |
| Button B (ACK) | GPIO33 | INPUT_PULLUP |
| Button C (MANUAL) | GPIO26 | INPUT_PULLUP — reclaimed from grow-light MOSFET (freed by DL-010) |
| Buzzer | GPIO4 | GPIO → 1kΩ → NPN base; emitter → GND; collector → buzzer(−); buzzer(+) → 5V |

Button C was initially wired to GPIO35, which is input-only on the ESP32 and *also* lacks internal pull-up/pull-down resistors — a property easy to miss because the broader "input-only" labeling does not call this out explicitly. With INPUT_PULLUP configured but no actual pull-up present, the pin would have floated when the button was released, producing random false triggers. Caught during bench planning before any code ran; the wire was moved to GPIO26 (available because DL-010 moved grow-light control off the MOSFET). This is an instance of the engineering value of writing down the GPIO inventory explicitly — the pin reclamation only worked because the project tracks which pins are actually in use, not just which were originally specified.

**State machine validated by the test sketches.**

| State | LEDs | Buzzer | Entry condition |
|---|---|---|---|
| IDLE | green steady | silent | boot; ACK from FAULT or CRITICAL |
| ACTION | green fast blink | silent | MANUAL from IDLE (3 s, then auto-returns to IDLE) |
| FAULT | red steady | silent | STOP from IDLE or ACTION |
| CRITICAL | red fast blink | **ON** | STOP from FAULT (escalation: unacknowledged fault) |

A fifth pattern (MEASURING — green slow blink) was demonstrated in the first test sketch but is not currently reachable through button input alone; it is reserved for Phase 2 when the firmware enters MEASURING during a sensor read cycle.

**Test methodology — three iterative sketches.**

Rather than build one combined sketch, the validation was done across three discrete sketches, each preserved as its own artifact in the repo. This gives a clear record of how each component was first validated:

- `07-feedback-io` — LEDs only; cycles every state pattern automatically to confirm visual distinguishability.
- `08-buttons` — Adds buttons; introduces the IDLE/ACTION/FAULT state machine driven by button presses; implicitly re-validates the LEDs in their state-indicator role.
- `09-buzzer` — Adds the buzzer and the CRITICAL state; introduces the STOP-from-FAULT escalation; implicitly re-validates the LEDs and buttons.

The state-machine logic is mocked: STOP unconditionally triggers FAULT, escalating to CRITICAL on a second press, with no actual fault condition behind it. The real fault triggers (low water, leak detected, watering verification failure, etc.) belong to Phase 2 firmware. What this test validates is that the *components and the state-rendering logic* are correct, so Phase 2 can connect real conditions to a known-good rendering layer.

**Safety property exercised.** The state machine enforces that the MANUAL button is inert when the system is in FAULT or CRITICAL. The fault state is *latched* — it does not clear by itself, and the operator (local or remote) must explicitly acknowledge it via ACK before any further actions are accepted. This is the standard pattern for safety-critical state machines (the same pattern used in industrial control, aerospace, and autonomous vehicles) and was verified observationally during the buzzer test: pressing MANUAL during FAULT produced no state change.

**What this test did not verify.**

- Real fault triggers (deferred to Phase 2: low-water signal from the float switch, leak detection, watering-verification failure, sensor-stuck detection, etc.).
- Long-term buzzer reliability under repeated CRITICAL events.
- Behavior when multiple buttons are pressed simultaneously (the current code services them in serial order each loop iteration — adequate for typical operator behavior, but not formally tested).
- Debouncing under noisy mechanical contacts (the 10 ms loop delay provides crude debouncing; if any button shows flicker on a press, software debouncing will be added in Phase 2).

**Alternatives considered.**

- Original 3+3+1 plan (rejected — components without specific justified roles).
- 1+1+1 ultra-minimal plan (rejected — insufficient state granularity for a remote helper to describe; "the light is on" doesn't distinguish IDLE from FAULT).
- Passive buzzer with PWM tones for variable alert sounds (rejected — over-spec for a single-alarm use case).
- Bi-color (red/green in one package) LED instead of two discrete LEDs (rejected — one pin and one resistor savings, but harder to wire reliably on a breadboard and slightly harder to describe over the phone).

---

<a id="dl-026"></a>
### DL-026 — Leak sensor validated and calibrated

**Date:** 2026-05-31 · **Status:** Active

**Context.** Phase 1 component validation and calibration for the DIYables comb-pattern water-detection sensor. This sensor was added to the project in DL-013 as a leak-detection input that would trigger the system's CRITICAL fault state if water appeared outside the reservoir. The bench test validates that the sensor works and establishes the operational range so the Phase 2 firmware can set a meaningful trigger threshold.

**Decision.** Leak sensor passes validation. The sensor is approved for integration. The Phase 2 firmware will treat the analog reading as a sensitive binary trip wire rather than a calibrated continuous measurement.

**Rationale.** Sensor wired to GPIO39 (ADC1, input-only, WiFi-safe). The test sketch reads at 2 Hz with 20-sample averaging per read; the dry baseline reads cleanly at 0 ADC counts (0%) with no measurable drift, providing a stable zero against which any wet signal is unambiguous.

**Calibration data.**

| Condition | Raw ADC | % of full scale |
|---|---|---|
| Dry pads (no contact with water) | 0 | 0% |
| Tissue lightly damp around sensor | ~283 | 6.9% |
| Tissue wet, squeezed against sensor | ~1376 | 33.2% |
| Sensor partially submerged in water | ~2067 | 50.5% |

**On the ~50% ceiling.** The sensor's output does not approach the full 0–4095 ADC range even when fully submerged. This is a physical property of conductive-pad water sensors, not a sensor defect: ordinary tap water is a poor conductor by electrical standards, so the resistance between the interleaved copper pads never falls low enough to drive the divider's output to its full-scale voltage. Higher-conductivity water (saltwater, mineral-rich runoff) would push the reading higher; deionized water would push it lower. For the project's intended deployment (detecting a leak of plain water onto a damp surface near the reservoir), a 0–50% practical range is the available signal, and the dry baseline of 0 is the meaningful reference point.

**Implications for the Phase 2 firmware.**

- The trigger threshold for entering CRITICAL on a leak event should be set at approximately **200 raw counts (~5% of full scale)**. This is well above the dry baseline of 0, well below the lightly-damp reading of ~283, and gives the sensor a sensitive trip-wire character — a small amount of water near the sensor (the kind that would indicate the *start* of a leak) is detected before it becomes a flood.
- Once triggered, the system enters CRITICAL state and remains latched until manually acknowledged via Button B (ACK). The fault does not auto-clear even if the reading falls back to zero, because a single brief wet reading still indicates that water reached a place it should not have — the operator needs to inspect and confirm the situation, not have the system silently dismiss the event.
- Hysteresis is not needed in the same way as the soil sensor: this is a one-shot event detector, not a continuous control input.
- The sensor reads in the opposite polarity from the soil moisture sensor: **higher = wetter** here, **lower = wetter** for soil moisture. The firmware comment block above each read must call this out to prevent inverted-logic bugs.

**What this test did not verify.**

- Long-term sensor stability under sustained wet exposure (the exposed copper pads will corrode over months of contact with water). The deployment plan is to mount the sensor on the *surface below* the reservoir and pump assembly, where it should remain dry under normal operation and only become wet during an actual fault. This minimizes corrosion exposure.
- Behavior with non-pure water (potting-mix runoff has dissolved minerals that increase conductivity; this could shift the practical range upward, which would be conservative — i.e. easier to trigger).
- Behavior at temperature extremes (water conductivity is temperature-dependent; for indoor operation this is negligible).
- Whether a slow drip is detected as quickly as a pour (the test used relatively rapid wetting; very slow drips might take longer to bridge the pads conductively, but should still trigger eventually).

**Alternatives considered.** None — this is a validation outcome, not a design choice. The sensor itself was chosen in DL-013 as the simplest, cheapest, most appropriate option for a binary leak detector.

---

<a id="dl-027"></a>
### DL-027 — Hub bootstrap: Raspberry Pi online, Mosquitto installed

**Date:** 2026-05-31 · **Status:** Active. Work-in-progress milestone — broker configuration and Shelly integration are next.

**Context.** The Raspberry Pi 4 is the project's hub, running the MQTT broker, data services, database, and dashboard. This entry records the first step of standing it up: getting the Pi online, updating the OS, and installing the Mosquitto package. Broker configuration and end-to-end MQTT testing are deferred to a subsequent entry.

**Decision.** The Pi is treated as a fully-fledged subsystem in the repository, with its setup and service installation documented as discrete steps under `hub/` — mirroring the per-component structure used for firmware test sketches. Each step gets its own subdirectory with a README that documents the procedure, the current state, and what remains. Configuration files and verification scripts will be added to those directories as the work progresses.

**Rationale.** A monolithic "hub setup guide" document was rejected in favor of incremental, per-step directories for three reasons. First, parity with the firmware structure: a reader scanning the repo sees the same organizational pattern in both `firmware/test-sketches/` and `hub/`, which is internally consistent and easier to navigate. Second, partial completion is honestly representable: each step's README can say "done X, not yet Y" without forcing the entire document into "draft" or "complete" states. Third, atomic commits: a single step's progress can be committed without rewriting an umbrella document, which keeps the git history meaningful.

**Current hub state.**
- Raspberry Pi 4 Model B flashed with Raspberry Pi OS Lite (64-bit) onto a SanDisk 64 GB microSD card. Older cards considered (1 GB, 16 GB) were rejected for size.
- SSH operational from the developer machine.
- `sudo apt update && sudo apt upgrade -y` completed.
- `mosquitto` and `mosquitto-clients` installed via apt. Broker not yet configured or tested.

**On the deployment WiFi.** Initial work was done using the developer's home WiFi for productivity reasons. The intended deployment environment is on the JSU campus network, where a first attempt suggested a *client-isolation* policy: devices on the network cannot reach each other directly, even on the same SSID. This would block both SSH from the developer machine to the Pi and MQTT between ESP32 nodes and the Pi. The diagnosis was made via ARP table inspection but is not yet conclusive — the initial WiFi credentials may also have failed to write during OS flashing (the configuration file was missing on the Pi after boot), so the campus WiFi was never actually attempted with valid credentials. **Action item:** retest the campus WiFi with verified credentials on the next campus visit; if client isolation is confirmed, escalate to campus IT for a viable network segment (separate AP, MAC registration, or wired Ethernet). If neither is available, fall back to a personal mobile hotspot dedicated to the deployed system.

**Implications.** The hub work can proceed on home WiFi without blocking, since the Pi just needs internet for package installs and the broker's loopback for local testing. The deployment-network question only becomes blocking when the ESP32 nodes and Shelly need to connect to the same broker, which is two to three commits away.

**What this entry does not record.** Broker configuration choices (listener interface, persistence settings, authentication). The decision to use Mosquitto over alternatives — that was made at project definition. The choice of Pi 4 over Pi Zero or other variants — also made earlier. Static IP assignment for the Pi — deferred until broker testing reveals whether DHCP-stable or static is needed.

**Alternatives considered.** Single monolithic hub-setup document (rejected, see Rationale). Skipping documentation until everything is working (rejected — would lose the procedural detail and the campus-WiFi diagnostic, which is exactly the kind of friction worth recording for replicability).

---

<a id="dl-028"></a>
### DL-028 — Campus deployment network deferred; project on home WiFi pending IT consultation

**Date:** 2026-05-31 · **Status:** Resolved 2026-06-02 — JSU_DEVICE confirmed viable across three devices (Pi, Mac, Shelly Plug US Gen4); proceeding with project on JSU_DEVICE as the deployment network

**Context.** The project's intended deployment environment is the JSU campus lab where Phase 2+ integration testing will happen. The hub bootstrap session (DL-027) attempted to bring the Pi up on a campus-accessible network so the Pi, the future ESP32 nodes, and the Shelly smart plug could all communicate over the same broker. The attempt failed and the investigation surfaced architectural constraints that would have blocked the project later if discovered during integration rather than now.

**Decision.** Continue all hub and integration work on home WiFi for the immediate term. Deferral of the campus-network choice until a consultation with campus IT confirms whether any available campus network path satisfies the four criteria listed below. If no campus path qualifies, fall back to a personal mobile hotspot dedicated to the deployed system or wired Ethernet from a vetted jack.

**Rationale — what was tried at the lab.**

- **Direct association with the campus IoT-style network (JSU_DEVICE).** WPA2-Personal authentication (verified — not enterprise EAP), so IoT-class devices like the Pi and Shelly can in principle join. Client isolation was suspected (devices on the same SSID cannot reach each other directly) but was never conclusively tested before the session ended.
- **TP-Link Archer A6 router uplinked to a campus wall jack.** The router obtained a WAN IP, but devices attached to its WiFi kept dropping. A factory reset did not stabilize the router. Discarded as a hardware issue rather than a network issue.
- **Netgear R6400v2 uplinked to the same wall jack.** The router itself was healthy and got upstream connectivity — direct IP pings to 8.8.8.8 succeeded — but DNS lookups resolved hostnames to the router's own LAN IP. That signature is characteristic of an upstream **captive portal** holding the network until the client authenticates through a web page. macOS repeatedly disconnected from the router's WiFi because it could not verify real internet access. A headless Pi has no browser, so even if the Mac side had cleared the portal, the Pi could not.
- **Direct Ethernet from the wall jack to the Pi.** Considered but not attempted — would have required either a connected monitor for headless diagnostics or pre-configured static networking, neither of which was set up that session.

**Diagnostic signatures worth recording.**
- **Captive portal** — DNS resolves hostnames to the router's own LAN IP regardless of the lookup; pings to bare IP addresses (e.g., 8.8.8.8) succeed; macOS shows the "no internet" yellow exclamation on the WiFi icon and may auto-disconnect.
- **Client isolation** — devices can reach the gateway and internet but cannot reach each other; ARP table on one device does not contain the other's MAC; `ping <other-device>` times out even though both devices are confirmed associated with the same SSID.

**Criteria the deployment network must satisfy.** The next campus-IT conversation should request a path that meets all four. Any path missing one of these is unworkable for this architecture, regardless of how friendly it is in other respects.

1. **Shared Layer 2 broadcast domain across participating devices** — Pi, both ESP32 nodes, and the Shelly must all be on a network where they can reach each other by IP. mDNS (`planthub.local`) is convenient but not required; raw IP works fine if `arp` and `ping` succeed device-to-device.
2. **No captive portal** — or, if one exists, a mechanism to bypass it for registered MAC addresses. A headless Pi and a headless Shelly cannot complete a browser-based portal.
3. **WPA2-Personal (PSK), not WPA2-Enterprise (EAP)** — the Shelly Plug US Gen4 does not support enterprise authentication. WPA3-Personal is acceptable on the Pi and ESP32 sides but the Shelly may not support it; verify before committing.
4. **Outbound internet** — for OTA updates, ntfy.sh alerts, optional remote dashboard access, and package installs. Strictly speaking the broker and dashboard work LAN-only, but losing outbound internet narrows the project's feature set significantly.

**What this entry does not commit to.** A specific campus network. A specific fallback (mobile hotspot vs. Ethernet vs. dedicated AP). A timeline for moving off home WiFi. The campus-IT conversation defines the next decision; this entry just establishes what that decision needs to satisfy.

**Implications for the immediate hub work.** None blocking. The Pi is online via home WiFi; the broker, Python services, and dashboard will all install and verify identically on any compatible network. When the deployment network is chosen, the Pi can be re-pointed via `sudo raspi-config` (WiFi) or `dhcpcd.conf` (Ethernet) with no other changes. The ESP32 firmware will hard-code the broker's IP and the WiFi SSID/password as compile-time constants — when the network changes, the firmware is recompiled and reflashed. This is acceptable for a single deployment but worth recording so it's not forgotten as a deployment-time chore.

**Lessons worth keeping.** Enterprise and institutional networks are hostile to IoT projects by default. Captive portals and client isolation are common, and both produce silent failures from the device side — there is no error message saying "this network does not allow device-to-device traffic." Verify both before committing an architecture to any given network. Also: a single mis-flashed credentials file (the initial Pi image had no WiFi credentials written despite the Imager UI claiming otherwise) can look like a network problem for hours. Always confirm the customization landed on the SD card before debugging anything downstream.

**Alternatives considered.** Continue troubleshooting on campus that night (rejected — diagnostic momentum was already lost and the home network is available, productive). Order a separate access point and bypass the campus network entirely (deferred — premature without IT input; might be the right answer but worth checking other paths first).

**Addendum (2026-06-01) — campus network testing.** Returned to campus and tested the deployment-network question empirically with the Pi.

*ARP visibility on JSU_DEVICE.* From the Mac connected to JSU_DEVICE, `arp -a` listed approximately 200 devices spanning the 10.6.0.0/16 subnet. A strictly client-isolated network would not produce that breadth — isolation typically restricts visibility to the gateway and a small number of devices the Mac has actively talked to. This is positive evidence that JSU_DEVICE is *not* aggressively isolated at the link layer, contrary to the original suspicion.

*Two Pi re-flash attempts.* The first re-flash with Raspberry Pi Imager customization for JSU_DEVICE failed: the Pi never appeared in the Mac's ARP table even after a full first-boot interval, and no Pi-prefix MAC was discoverable. This was indistinguishable from client isolation at first glance. The second re-flash succeeded — `ping planthub.local` from the Mac returned `64 bytes from 10.6.19.139` with sub-30 ms latency, the Pi was reachable via SSH on the campus network, and `mosquitto` was re-installed without incident.

*Concluded diagnosis.* The first re-flash's failure was almost certainly a password typo during Imager customization. The Imager UI shows the WiFi password as dots and does not validate it before writing, so a wrong character produces a Pi that boots, attempts WiFi association, fails authentication silently, and stays off the network with no error visible to the operator. The second re-flash, with the same target SSID but a freshly-typed password, joined immediately.

*Status of the four DL-028 criteria after today.*
1. Shared Layer 2 broadcast domain — **satisfied.** Mac and Pi can ARP each other directly on JSU_DEVICE (Mac sees 10.6.19.139 in its ARP table and can ping it).
2. No captive portal — **satisfied.** Pi authenticated and reached outbound internet (apt update + install ran cleanly during the re-install of Mosquitto on JSU_DEVICE) without any browser interaction.
3. WPA2-Personal — **satisfied** (verified during initial network selection).
4. Outbound internet — **satisfied** (apt-get reached Debian repos and downloaded packages successfully on JSU_DEVICE).

*Updated decision.* JSU_DEVICE is now considered viable for the project's deployment, pending one remaining test: device-to-device communication between the Pi and the eventual ESP32 nodes / Shelly smart plug (i.e. can other clients reach the Mosquitto broker on the Pi over JSU_DEVICE?). This requires Pi-side broker LAN configuration and is the immediate next step.

*Backup option retained.* JSU IT also provided an alternate WiFi credential set on 2026-06-01. The credentials remain on file as a fallback if the JSU_DEVICE viability conclusion turns out to be premature (e.g., if device-to-device traffic on JSU_DEVICE turns out to be filtered even though direct Mac-to-Pi ping works). The backup network has not yet been characterized.

*Lessons added to the project's bench-test discipline.*
- Imager customization fails silently on password typos. When the symptom is "Pi doesn't appear on the network and there's no obvious reason why," **re-flash with extra attention to the password before debugging the network**. The cost of re-flashing is ~5 minutes; the cost of misdiagnosing as a network issue can be hours.
- ARP table breadth is a useful low-effort signal for client-isolation diagnosis. A network where you can see ~200 other devices is structurally not strictly isolated, regardless of what symptoms a single failed device might produce.

---

<a id="dl-029"></a>
### DL-029 — Mosquitto broker verified via loopback pub/sub

**Date:** 2026-05-31 · **Status:** Active

**Context.** Mosquitto was installed on the Pi in DL-027, but installation alone does not prove the broker actually routes messages between clients. The loopback pub/sub test exercises the broker's core function — accept a connection from a publisher, accept a connection from a subscriber, deliver published messages to subscribed clients — on the most controlled possible setup, two clients on the same host as the broker.

**Decision.** Mosquitto broker verified operational on the Pi for loopback traffic. The broker is approved for use as the project's MQTT broker. LAN-accessible and authenticated configurations are deferred to subsequent entries.

**Rationale.** Test methodology and result documented in `hub/02-mosquitto-install/README.md`. Three test messages were published from one SSH session and received instantly by a subscriber running in a second SSH session, both on the Pi. The test confirms the broker is actively routing — installation succeeded, the systemd service is healthy, the default configuration is sufficient for local-loopback operation, and no firewall or service-level misconfiguration blocks pub/sub.

**Why two SSH sessions, not one.** The pub/sub model is fundamentally between separate clients — a publisher and a subscriber that may not be aware of each other's existence. Testing both roles from a single shell would not exercise the broker's actual routing behavior in any meaningful way; it would only prove the binary executes. Two independent client connections to the same broker is the minimum honest test.

**What this verifies in concrete terms.**
- Mosquitto's daemon accepts client connections on `localhost:1883` (the default).
- The default configuration (no custom `mosquitto.conf` yet) permits publish, subscribe, and message routing.
- The broker exhibits expected near-zero latency between publish and delivery (the messages appeared in the subscriber's terminal instantly upon publish from the other session).

**What this does not verify.**
- LAN client reachability. The broker has not been bound to the Pi's LAN interface or `0.0.0.0`, nor has any non-Pi device attempted a connection. The next step is to add an explicit listener for the LAN interface, then verify with a third client running on the Mac.
- Authentication. The default configuration is open — any client that can reach the broker can publish and subscribe to any topic. For LAN-only development this is acceptable; before any external exposure (Cloudflare Tunnel, Tailscale, port forward), the broker must require authentication.
- Persistence across reboots. Whether `mosquitto.service` is enabled in systemd has not been confirmed. A power cycle test is the appropriate next verification.
- Topic-level access control. The default permits any client to subscribe to `#` (all topics), which has security implications when more devices join. Not relevant for the loopback test but worth noting.

**Implications for the next steps.**
- Before the Shelly Plug US Gen4 can be told to use this broker, the broker must be reachable from the LAN. That requires configuring `mosquitto.conf` to bind a listener to the appropriate interface.
- Before the broker is exposed beyond the LAN, authentication must be enabled.
- A separate decision will record the topic structure the project uses (`plant/sensors/...`, `plant/state`, `plant/commands/...`, etc.) when the first ESP32 client publishes real data. The existing topic plan from the project's earlier design discussions is still a draft until it's exercised by real firmware.

**Alternatives considered.** None — this is a verification outcome, not a design choice.

---

<a id="dl-030"></a>
### DL-030 — Mosquitto broker configured for LAN access with authentication

**Date:** 2026-06-02 · **Status:** Active

**Context.** DL-029 validated the broker for loopback traffic only. For the project's actual architecture — Shelly smart plug, ESP32 nodes, and the eventual Streamlit dashboard all connecting to the broker over the LAN — the broker must accept connections on its LAN interface, not just on `localhost`. Authentication is required because the LAN in question is the shared campus network (JSU_DEVICE) with hundreds of other devices.

**Decision.** Mosquitto broker configured to listen on `0.0.0.0:1883` with username/password authentication required. The broker is approved as the Phase 3 telemetry hub.

**Rationale — listener interface.** `0.0.0.0` (listen on all interfaces) was chosen over binding to a specific IP. The Pi receives its IP via DHCP and the IP may change across reboots or lease renewals on a network this size. `0.0.0.0` keeps the broker reachable regardless of which IP the Pi currently holds. Other devices find the Pi either by mDNS (`planthub.local`, which is flaky on JSU_DEVICE) or by the current IP. A static DHCP reservation will be requested from campus IT in a future conversation to make `10.6.19.139` (the current address) permanent; until then, the IP is treated as discoverable rather than hardcoded.

**Rationale — authentication.** `allow_anonymous false` enforced from the start, before any client connects. The broker is on a shared campus network with ~200 other devices, MQTT port 1883 is widely scanned, and the cost of adding auth later (after firmware has been written, after the Shelly has been paired, after the dashboard has been built) is substantially higher than the cost of doing it now (one extra config line, one extra command-line flag in client invocations). The initial MQTT username chosen was `basilpi` (mirroring the Pi user account name for simplicity); subsequently renamed to `basilmqtt` during DL-032 to remove naming collision with the Linux user, since the two identities serve different purposes and conflating them in firmware code is a likely source of confusion.

**Verification — three tests in increasing scope.**
1. **Anonymous rejected.** `mosquitto_sub` without credentials → immediate `Connection Refused: not authorised`. Confirms auth is enforced rather than bypassed.
2. **Authenticated loopback.** `mosquitto_sub`/`mosquitto_pub` with credentials, both on the Pi → messages flow correctly. Confirms the credential check accepts valid logins.
3. **External client.** `mosquitto_sub` on the Mac to the Pi's IP with credentials, `mosquitto_pub` on the Pi → messages flow across the LAN. **This is the architecturally meaningful test** — it proves the broker is reachable, authenticated, and operational from a device that is not the Pi.

All three tests passed.

**On the password file ownership puzzle.** During initial setup, the broker repeatedly failed to start with `status=13` (EACCES). Investigation revealed that `mosquitto_passwd -c` creates the file owned by `root:root`, but the broker daemon runs as the `mosquitto` user and needs read access. The naive fix (`chown mosquitto:mosquitto`) makes the broker happy but produces a deprecation warning from `mosquitto_passwd` saying future versions will refuse to load files not owned by root. The correct configuration is **owner `root`, group `mosquitto`, permissions `640`** — root retains write access, the broker reads via group membership, and no one else has access at all. Recorded in `hub/03-broker-config/README.md` so this isn't re-diagnosed next time.

**On the duplicate-key error.** First config attempt re-declared `persistence_location`, which is already set in the default `mosquitto.conf`. Mosquitto refuses to load configurations with duplicate keys and the broker fails to start. Recorded similarly. **General principle:** override settings in `conf.d/` only when the override is actually different from the default. Defaults are not problems to be solved by re-stating.

**On the rotated password.** During the verification session, the MQTT password was inadvertently captured in plain text in chat logs while pasting command output. Out of professional habit, the password was rotated to a fresh value after testing completed. This is recorded not because the exposure was high-risk (the broker is LAN-only, the chat is private, no exploitable surface was opened), but because the pattern is the one to internalize for systems where the stakes are real: when a secret might have leaked, rotate it.

**Implications for the next steps.**
- The Shelly Plug US Gen4 can now be configured to connect to `10.6.19.139:1883` with the `basilpi` username and the current password. The Shelly's MQTT support is built in; this is a configuration step in its web UI, not new code.
- ESP32 firmware can begin to integrate MQTT publishing for sensor data. The broker is ready to receive.
- The MQTT password used by both Shelly and ESP32 firmware must be stored as a secret — gitignored secrets file in firmware, never committed. The pattern: a `secrets.h` (or similar) per sketch, present locally, never tracked.

**What this work does not yet verify.**
- Reboot persistence. The broker is `enabled` in systemd but a full power-cycle test has not been done. Quick verification scheduled before the Shelly integration.
- Multi-client load. Only one external client (Mac) has connected at a time. Confidence that the broker handles multiple simultaneous connections is based on Mosquitto's general behavior, not direct test.
- Topic-level access control. Currently any authenticated client can publish or subscribe to any topic. Acceptable for a single-tenant LAN broker but worth tightening later if multiple users ever share the broker.
- TLS encryption. The broker speaks plain MQTT. Acceptable for LAN-only; required before any external exposure.

**Alternatives considered.**
- Bind only to the Pi's current LAN IP instead of `0.0.0.0`. Rejected — DHCP makes the IP a moving target.
- Allow anonymous connections for development, add auth later. Rejected — the cost of retrofitting auth after clients exist is meaningfully higher than configuring it correctly from the start.
- Use mTLS (mutual TLS) instead of username/password. Rejected for Phase 3 — substantial complexity for marginal gain on a LAN-only broker. May revisit at deployment hardening.

---

<a id="dl-031"></a>
### DL-031 — Shelly Plug US Gen4 paired and joined JSU_DEVICE

**Date:** 2026-06-02 · **Status:** Active. MQTT configuration is the next step (separate DL entry).

**Context.** The Shelly Plug US Gen4 is the project's grow-light controller per DL-010. It is the project's first commercial, off-the-shelf networked client of the Mosquitto broker. The project's two ESP32 nodes are still on the bench and not yet networked, so the Shelly is the first opportunity to validate the broker's authenticated LAN access path (DL-030) with a real third-party MQTT client rather than command-line test tools.

**Decision.** Shelly Plug US Gen4 successfully paired, joined JSU_DEVICE WiFi, and named "basilplug." Current IP on JSU_DEVICE: `10.6.17.32`. The plug is approved as the project's grow-light controller and is ready for MQTT configuration.

**Pairing process and what was learned.**

The initial pairing attempt failed at the Shelly Smart Control app's "Add Device" step despite the app being able to discover the plug. Two factors were involved, both worth documenting:

- **Provisioning window expiry.** Shelly firmware 1.7.5+ (which this unit ships with) only accepts new pairing requests during a 15-minute window after power-on. The window had likely expired between unboxing the plug, downloading the mobile app, and signing in to a Shelly cloud account. Resolved by unplugging the plug and re-applying power to restart the window.
- **Prior HomeKit auto-pairing.** On first power-up before any app was installed, the plug auto-paired to the developer's iPhone via Apple Home (HomeKit). HomeKit broadcasts a setup code by default on first boot; an iPhone signed into iCloud and on the same local network discovers and adds it automatically. This is convenient for the consumer use case but conflicts with re-pairing via a different app: the Shelly Smart Control app treats a plug that already has a WiFi association as "already provisioned" and refuses to complete its own setup flow. The Apple Home control continued to work because HomeKit's local-network protocol is independent of WiFi credentials in the way the Shelly Smart Control app expects.
- **Resolution.** Factory reset via the standard "hold button for ~10 seconds" procedure. After reset, the LED flashed blue (provisioning mode), and re-pairing via the Shelly Smart Control app succeeded immediately. Manual typing of the WiFi password (no autofill, no paste) was used per Shelly's documented advice, though it is unclear whether that step was necessary or just well-advised hygiene.

**On the HomeKit-vs-MQTT relationship.** Configuring MQTT on the Shelly will not disable HomeKit; the two protocols coexist on the device. After the next DL entry covers MQTT setup, the plug will be controllable through three independent paths: the Shelly app (cloud or LAN), HomeKit (if re-added to Apple Home post-reset), and MQTT (our broker). For this project's automation logic, only the MQTT path matters; the others are conveniences available to a human operator.

**Network observations.**
- JSU_DEVICE supports 2.4 GHz WiFi. The Shelly's 2.4-GHz-only radio joined without issue, ruling out the band-mismatch concern raised in DL-028's criteria check.
- The plug received a DHCP-assigned IP (`10.6.17.32`) in a different subnet octet than the Pi (`10.6.19.139`), confirming that JSU_DEVICE assigns IPs from a large pool. Devices may not always be in the same /24, but the /16 subnet still puts them in the same broadcast domain, which is what matters for MQTT discovery.
- This is the third device (after the Mac and the Pi) to successfully use JSU_DEVICE for project-relevant traffic, fully resolving DL-028's deferral.

**What this entry does not yet record.**
- MQTT configuration on the Shelly (broker IP, credentials, topic prefix). That is the next DL entry, after the Shelly's web UI configuration is done and verified end-to-end.
- The grow-light's actual electrical load. The Shelly is currently plugged into a power strip with no load attached to it; the grow light will be plugged into the Shelly only after MQTT control is confirmed working.
- Static DHCP reservation. Both the Pi and the Shelly are currently on DHCP-assigned IPs; if either reboots, the IP may change. Worth requesting from campus IT eventually; not blocking today.

**Lessons worth keeping.**
- Consumer IoT devices often auto-pair to platform-specific ecosystems on first power-up. This is usually harmless but can interfere with re-pairing through a different application, with no clear error message indicating why. Factory reset is the standard recourse.
- Firmware-imposed provisioning windows are a real failure mode that looks identical to network or app issues. When pairing fails inexplicably, check the device's firmware behavior documentation for time-bound provisioning states before blaming the network.

**Alternatives considered.** None — the Shelly is already chosen per DL-010, and the pairing process is dictated by the device's firmware.

---

<a id="dl-032"></a>
### DL-032 — Shelly Plug US Gen4 validated as MQTT client; MQTT username renamed to `basilmqtt`

**Date:** 2026-06-02 · **Status:** Active

**Context.** Following the broker LAN configuration in DL-030 and the Shelly device pairing in DL-031, this entry covers the actual MQTT integration: configuring the Shelly to publish to and subscribe from the project's Mosquitto broker, validating bidirectional control over the LAN, and refining the username scheme based on practical experience. The Shelly is the project's first commercial MQTT client and proves the broker is usable by real devices, not just command-line test tools.

**Decision.** Shelly Plug US Gen4 is fully integrated with the project's MQTT broker. The plug publishes its status and event stream to topics under `plant/grow-light/` and responds to control commands sent to its command topics. The integration is approved for use; the next-step ESP32 firmware can model its broker-client behavior on this same pattern.

**Rationale — MQTT integration choices.**

- **Topic prefix `plant/grow-light/`.** Hierarchical and human-readable. The trailing slash convention lets us subscribe to `plant/grow-light/#` for everything the plug emits, or to specific sub-topics like `plant/grow-light/status/switch:0` when only the relay state matters. Picking the prefix at the device boundary (configured on the Shelly itself) means the broker stays prefix-agnostic; future devices can have their own prefixes (`plant/sensors/`, `plant/state/`, `plant/commands/`) without any broker-side changes.
- **Connection type: No TLS.** Plaintext MQTT on port 1883. Acceptable because the broker is LAN-only and JSU_DEVICE is not the public internet. If the broker is ever exposed externally (Cloudflare Tunnel, port forward, etc.), TLS becomes mandatory.
- **Enable 'MQTT Control', RPC over MQTT, RPC status notifications, generic status updates** — all enabled. These expose the Shelly's full RPC interface over MQTT, which lets the eventual Phase 2 firmware do more than just toggle the relay: it can read live electrical readings (voltage, current, power, frequency), query the device's system status, schedule on/off via the Shelly's own scheduler, and receive event notifications when anything changes. Over-broad for what the project strictly needs today, but the marginal cost of enabling them now is zero and the optionality is valuable.
- **Cloud connection left enabled.** The Shelly also maintains its own connection to Shelly Cloud, which is unrelated to our broker. Disabling it would require explicitly turning off cloud control in the Shelly settings; the marginal benefit (slightly less network chatter, slight privacy improvement) does not justify the loss of the Shelly mobile app's remote-control feature, which is a useful operator convenience. Cloud control coexists with MQTT control without interfering.

**Verification — bidirectional control.**

The integration was validated end-to-end with two tests, both run from the developer Mac (not the Pi), simulating how the eventual ESP32 firmware will interact with the plug:

- **Receive (Shelly → broker → external client).** Subscribed to `plant/grow-light/#` from the Mac. Within seconds of the Shelly's reboot, observed `plant/grow-light/online true`, `plant/grow-light/status/mqtt {"connected":true}`, `plant/grow-light/status/switch:0 {...}` with full electrical readings, and a continuous stream of `plant/grow-light/events/rpc` notifications. Confirms the plug publishes its complete state and event stream as configured.
- **Send (external client → broker → Shelly).** Published `'off'` to `plant/grow-light/command/switch:0` from the Mac. Observed the Shelly's relay physically toggle off (LED change, web UI state update), and a corresponding `status/switch:0` message reflecting `"output":false`. Repeated with `'on'`. Confirms the plug receives commands and acts on them.

Both tests were also repeated after physically moving the Shelly from the developer area to its intended location near the plant, confirming WiFi coverage and broker reachability at the deployment location.

**Username rename: `basilpi` → `basilmqtt`.**

The initial MQTT username chosen in DL-030 was `basilpi`, mirroring the Pi's Linux user account name. In practice, having the same name for two unrelated identities — the Linux user (used for SSH, sudo, file ownership) and the MQTT credential (used by MQTT clients) — proved cognitively heavy. The two are completely independent: they have different passwords, are used in different contexts, and have no functional relationship beyond convention. Conflating them in firmware code would also be a likely source of confusion: a developer reading "basilpi" in an ESP32 sketch might reasonably assume it refers to the Linux user, when in fact it refers to the MQTT credential.

Renamed via:

- `sudo mosquitto_passwd /etc/mosquitto/passwd basilmqtt` (add new user with new password)
- `sudo mosquitto_passwd -D /etc/mosquitto/passwd basilpi` (delete old user)
- Updated the Shelly's MQTT username field in its web UI from `basilpi` to `basilmqtt`

After both ends were updated and the Shelly rebooted, the integration resumed normally. Verified by subscribing to `plant/grow-light/online` from the Mac using the new credentials and observing `true` within ~30 seconds.

**On the rotations.** Four password rotations occurred during the integration session — each triggered by a different visible exposure (screenshot, terminal paste, command-line argument). The cumulative cost was minimal (~3 minutes per rotation), but the recurrence revealed a workflow problem: passing `-P 'password'` on the command line means the password ends up in shell history, terminal output, and anything pasted from that terminal. The disciplined response is not just rotation but eliminating the exposure surface.

**Adopted workflow — credentials in a sourced environment file.** Created `~/.mqtt/plant-broker.conf` on the developer Mac with file permissions `600`. The file contains two lines:

```text
export MQTT_USER=basilmqtt
export MQTT_PASS='current-password'
```

Sourced manually at the start of each session via `source ~/.mqtt/plant-broker.conf`, then referenced in commands as `-u "$MQTT_USER" -P "$MQTT_PASS"`. Pasted commands now show literal variable names (`$MQTT_USER`, `$MQTT_PASS`) rather than the actual values, eliminating the most common exposure vector.

The file is deliberately **not** added to `~/.zshrc` for automatic loading. Auto-loading would put the credentials in the environment of every program run on the machine, which is a broader exposure than the project requires; the trivial cost of typing `source ~/.mqtt/plant-broker.conf` once per session is worth the security boundary. For frequent use, a `mqtt-load` alias in `~/.zshrc` is acceptable since the alias itself does not contain credentials.

`~/.mqtt/plant-broker.conf` lives only on the developer Mac. It is not committed to the repo and is not on the Pi. On the Pi, the broker reads credentials from `/etc/mosquitto/passwd` (root-owned, mosquitto-group-readable, 640) — that file is also not committed and is regenerated locally per the procedure in `hub/03-broker-config/README.md`. The pattern for Phase 2 ESP32 firmware will be analogous: a `secrets.h` file present locally per sketch, listed in `.gitignore`, never committed.

**Implications for the next steps.**

- The ESP32 firmware to be developed in Phase 2 will use the same broker (`10.6.19.139:1883`), the same authentication mechanism (`basilmqtt` + current password), and a topic structure parallel to the Shelly's. Suggested ESP32-side prefixes: `plant/sensors/` for sensor publishes, `plant/state/` for the firmware's state-machine state, `plant/commands/` for inbound commands. Final topic structure will be locked in a future DL when the firmware is actually being written.
- A dashboard or data-logger on the Pi (Python service, Streamlit app) will subscribe to `plant/#` to receive everything from all devices. Subscribe-only consumers do not require a separate MQTT user; they can use `basilmqtt` until the project ever needs role-based topic ACLs.
- The Shelly's full RPC stream is now consumable by any Phase 2 component that wants live electrical readings — useful both for monitoring (energy use of the grow light over time) and for closed-loop verification (firmware can confirm the grow light actually drew power after issuing an ON command).

**What this entry does not yet record.**

- Plugging the actual grow light into the Shelly. The plug has been controlled empty so far. Once the grow light is physically connected, a quick verification that the relay actually switches the load (and that power readings reflect the load) will close out the deployment readiness for this subsystem.
- Topic ACLs. Currently any authenticated client can publish or subscribe to any topic. Acceptable for a single-tenant LAN broker; revisit if the broker ever serves multiple uses.
- Static DHCP reservation for either the Pi or the Shelly. Both currently use DHCP-assigned IPs; if either reboots and gets a new IP, the broker connection still works (because the Shelly initiates outbound to the Pi's IP), but the Mac's web access to the Shelly's UI would need the new IP. Worth requesting from campus IT eventually.

**Alternatives considered.**

- Use the Shelly's cloud relay instead of a direct MQTT integration. Rejected — the project's whole premise is local operation independent of any cloud service, and cloud relays add latency, dependency, and a layer of trust that is not justified for a local automation.
- Configure the Shelly via the mobile app rather than the web UI. The mobile app does expose MQTT settings but with less detail; the web UI was strictly better for this kind of integration work.
- Disable HomeKit on the Shelly to avoid dual-control surfaces. Rejected — HomeKit costs nothing to leave running and provides a useful operator-convenience path that doesn't interfere with MQTT in any observable way.

---

<a id="dl-033"></a>
### DL-033 — Pi power architecture: dedicated USB-C supply for permanent independent operation

**Date:** 2026-06-03 · **Status:** Active

**Context.** Throughout the hub bring-up (DL-027 through DL-032), the Raspberry Pi was powered from the developer's Mac via USB-C. This is convenient during initial setup but architecturally untenable: the Pi is the project's MQTT broker, the broker must be available 24/7 for the Shelly and the eventual ESP32 nodes to reach it, and the developer's Mac is portable and frequently away from the lab. Cutting Mac power means cutting the broker. The "Pi as broker" decision (DL-027) is meaningless without a power architecture that lets the Pi run independently.

**Decision.** The Pi is powered by a dedicated **CanaKit DCAR-RSP-3A5C** wall adapter: 5.1V at 3.5A output over a captive USB-C cable. The Pi now runs continuously, independent of the developer's Mac. This is the project's first deployment-ready hardware decision — the Pi is no longer a portable component.

**Rationale.**

- **The 5.1V output is intentional.** USB-C cables drop a measurable amount of voltage across their length under current load. A supply rated 5.0V at the wall may deliver only 4.7–4.8V at the Pi's input under sustained load, which trips the Pi's under-voltage warning and triggers conservative throttling. Pi-specific supplies output 5.1V at the wall to compensate, landing closer to 5.0V at the board. This is one of the most common silent failure modes for Pi installations — a generic 5V/3A supply often produces "works fine, sometimes throttles for no reason" behavior that is mistakenly attributed to software.
- **3.5A is comfortably above the Pi 4's 3A minimum** rating. Headroom matters because the Pi's instantaneous current draw spikes during boot, USB peripheral attachment, and high-CPU operations. A supply running near its limit ages faster and is more prone to under-voltage events during transients.
- **Captive cable** removes one variable (a flaky third-party USB-C cable could otherwise be the difference between stable and unstable). Means no separate cable purchase and no ambiguity about whether the cable is rated for the load.
- **CanaKit is the same designer/spec as Raspberry Pi's official PSU.** Both target the same 5.1V/3A Pi 4 spec. CanaKit's offering came with the Pi kit; no premium for switching to "official."

**Alternative supply rejected.** A second adapter was available — an Onn brand 2.1A wall charger (5V/2.1A). Rejected because the Pi 4 requires a 3A minimum supply. Using a 2.1A supply would have produced the silent under-voltage symptoms above. Recording the rejection here because "is this charger good enough?" is a recurring question worth a definitive answer: **no charger below 3A at 5V should ever be used with a Pi 4, regardless of the device claims about USB Power Delivery or fast-charging capability.**

**Deployment readiness implications.**

- **The Pi is now treated as a permanent fixture.** It stays plugged in, runs the broker continuously, survives developer travel and Mac unavailability. This was the unstated assumption behind every earlier hub-related DL but was not actually true until this point.
- **The broker is now able to maintain persistent connections** to the Shelly Plug US Gen4 and the eventual ESP32 nodes without periodic interruption. The Shelly's `online/offline` last-will signaling is now meaningful — going offline now indicates the *Shelly* lost connection, not the broker.
- **Overnight uptime test.** The Pi was left running overnight on the new power supply. Confirmed reachable the next morning (`ping -c 2 10.6.19.139` returned cleanly), broker still active. Single overnight is not a long-term stability test, but it establishes the baseline.

**What this entry does not yet record.**

- **Battery backup / UPS.** Campus power is not guaranteed stable. A brief outage will cause the Pi to reboot, briefly disconnecting the broker. Acceptable for a research/portfolio project; a real deployment would want a small UPS (or a Pi UPS HAT) to ride through outages and shut down cleanly during extended ones. Worth budgeting for, not blocking.
- **Power monitoring.** The system does not currently know whether *the Pi itself* is being powered correctly — it can only know it is currently running. Under-voltage events are logged by the kernel and visible via `vcgencmd get_throttled`, but no alerting is set up. If the supply ever degrades, the first symptom will be unexplained reboots; the diagnosis path would be checking the throttled counter and inspecting recent kernel logs.
- **Surge / lightning protection.** The Pi is plugged into a Belkin surge-protected power strip (visible in earlier deployment photos), which provides basic surge protection. Beyond that, the project is on whatever the building's electrical infrastructure provides.

**Alternatives considered.**

- **PoE HAT** (Power over Ethernet). Cleaner cable management — a single Cat5/6 cable delivers both network and power. Rejected for now because the project uses WiFi, not Ethernet; introducing PoE would require either an unmanaged switch with PoE injection or a PoE injector module, all for a problem that doesn't currently exist. Worth revisiting if the project ever moves to a wired-network configuration.
- **Continue Mac-powered.** Rejected because incompatible with autonomous operation, which is the entire point of the broker.
- **Battery / power bank.** Rejected for primary operation because Pi 4 power draw (2–5W typical, more under load) exceeds what's practical for indefinite operation on consumer power banks. Acceptable as an emergency fallback during brief moves.

---

<a id="dl-034"></a>
### DL-034 — ESP32-CAM bench validation deferred pending hardware replacement

**Date:** 2026-06-03 · **Status:** Active. Camera node deferred until known-good hardware is acquired and validated.

**Context.** The ESP32-CAM is the project's vision node per the original architecture — a separate camera-equipped MCU intended to capture periodic images of the plant for monitoring purposes. It is the only remaining unvalidated Phase 1 component after the user-feedback subsystem (DL-025), the leak sensor (DL-026), and the broker integrations (DL-029 through DL-032) all completed successfully. A bench-validation session was attempted to close out Phase 1.

**Decision.** ESP32-CAM bench validation is **explicitly deferred**, not abandoned, pending acquisition of known-good replacement hardware. The remaining Phase 1 inventory is closed at **11 of 12 components validated** with the camera node tracked separately.

**Rationale — what was tried.**

The session attempted standard ESP32-CAM bench bring-up with a USB-to-TTL programmer driving the bare ESP32-CAM module. Wiring followed the well-documented pattern:

- FTDI VCC (5V) → ESP32-CAM 5V
- FTDI GND → ESP32-CAM GND
- FTDI TX → ESP32-CAM U0R (RX)
- FTDI RX → ESP32-CAM U0T (TX)
- Jumper from ESP32-CAM IO0 to GND (flashing mode)

Two USB-to-TTL programmers were tested: a generic red breakout board (initial attempt) and an FT232-based blue programmer (second attempt). Both were correctly enumerated by macOS (`/dev/cu.usbserial-A5069RR4` and `/dev/cu.usbserial-B0046FJF` respectively). The platformio.ini was correctly configured for the AI-Thinker ESP32-CAM target, and the project compiled cleanly to a valid binary (`firmware.bin`, 11.2% flash used) on every attempt.

The upload step consistently failed with `Failed to connect to ESP32: No serial data received` after esptool exhausted its sync retry attempts. This indicates the chip was silent on its TX line — either not running, or running but not transmitting bootloader handshake responses.

**Diagnostics performed (all failed to identify a single cause).**

- **Wiring continuity** verified with multimeter — both 5V to 5V and IO0-to-GND connections measured continuous, no opens or shorts.
- **TX/RX swap test** — wires intentionally swapped, then restored. Same failure mode both ways, ruling out an obvious crossover error.
- **Upload speed reduction** — `upload_speed` dropped from 460800 to 115200 to rule out timing issues with the slower FTDI. No change.
- **PlatformIO reset-flag adjustments** considered (`--before=no_reset`, `--after=hard_reset`) per online troubleshooting guides, but not all permutations were exhausted.
- **Spare ESP32-CAM swapped in**, kept identical wiring. **Same failure on both boards** — the strongest signal that the issue is not the individual board but something in the common setup.
- **Multimeter measurement of the 3.3V rail** while the FT232 was powering the board read **3.9V**. The ESP32 chip is rated to 3.6V absolute maximum. If this reading was accurate and sustained, both chips may have been damaged during the diagnostic sessions today. (The reading was taken once and not repeated under controlled conditions, so it is one data point, not a confirmed diagnosis.)

**Honest assessment.** The combination of:
- Two boards failing identically with two different programmers
- An anomalous 3.9V reading on the 3.3V rail
- A pattern of "compile succeeds, flash fails at sync"

…does not point to a single mechanical fault we can fix. It points to either (a) both ESP32-CAM units were marginal or defective from the start (plausible — they came from the same lot, and one was unused stock), (b) today's 3.9V exposure damaged them during testing, (c) something subtler in the FTDI-to-board interaction we did not isolate, or (d) some combination. Without a known-good ESP32-CAM as a reference, we cannot distinguish among these explanations.

**Path forward.**

Order replacement hardware before resuming. Specifically considering:

1. **ESP32-CAM + ESP32-CAM-MB programmer combo** from a reputable Amazon seller (HiLetgo, AITRIP, DIYables). The MB programmer is a snap-on USB dock with built-in CH340 USB-to-serial, auto-reset, and auto-flash buttons. It eliminates the wiring and timing failure modes that consumed today's session.
2. **Adafruit ESP32-CAM** as a known-quality alternative — more expensive but zero ambiguity about authenticity.
3. **Seeed Studio XIAO ESP32-S3 Sense** as a platform alternative — built-in camera, native USB-C programming (no FTDI required at all), modern successor to the ESP32-CAM. Slightly different software libraries; some non-trivial pin and config differences from the AI-Thinker, but eliminates an entire class of failure modes by design.

Decision on which to order is deferred to a separate session focused on the purchase. Until the replacement arrives and validates cleanly, the camera node is treated as a known gap in the project's bench-validation inventory.

**What is committed despite the failure.**

- `firmware/test-sketches/11-esp32-cam/platformio.ini` — the working PlatformIO configuration for the AI-Thinker ESP32-CAM target. Valid configuration; the build succeeded. Kept in the repo so the next attempt has a starting point.
- `firmware/test-sketches/11-esp32-cam/src/main.cpp` — the minimal bench-test sketch (camera initialization + frame capture, no WiFi, no MQTT). Valid code; compiled cleanly. Did not run because the upload never succeeded, so the sketch is *unvalidated by execution* — important caveat.
- `docs/images/esp32-cam-ftdi-wiring.png` — photo of the wiring used during the session. Documents the setup that did not work, which is useful diagnostic context for the next attempt.

These artifacts are committed because they represent real, valid engineering work that should not be lost. The DL entry above makes clear they did not result in validation.

**What this entry does not commit to.**

- A specific replacement purchase.
- A timeline for resuming ESP32-CAM work.
- Whether the camera node will ultimately use AI-Thinker hardware or a different platform (XIAO ESP32-S3 Sense, etc.). That decision is part of the next session.

**Lessons worth keeping.**

- **A 3.6V-rated chip is unforgiving.** When voltage measurements are out of spec, stop immediately and re-check before applying more power. Repeated power-on attempts at out-of-spec voltage can cumulatively damage the chip. Continuing to debug "why won't this respond" without first confirming the supply is in spec wastes both time and components.
- **Two boards failing identically is a setup problem, not a board problem.** When the second board behaved identically to the first, it should have prompted an earlier pivot to questioning the common variables (FTDI current capacity, wiring, supply quality) rather than continued attempts on the same path.
- **Compile success is not flash success.** A valid binary that never gets uploaded is functionally equivalent to no work. Reading the actual terminal output (not just "PlatformIO ran") matters.
- **Bench-validation discipline scales to "could not validate."** This entry is materially the same shape as the successful DL entries — context, what was tried, what was learned, what remains — because the work is the same kind of work. Failure cases are part of engineering, not anti-engineering.

---

<a id="dl-035"></a>
### DL-035 — Phase 3 hub services kickoff: Python listener, SQLite, dashboard

**Date:** 2026-06-03 · **Status:** Active. Kickoff entry — implementation work begins this session.

**Context.** With the broker fully validated (DL-030) and a real MQTT client publishing telemetry (DL-032), the data flowing through the broker is currently *unobserved* — nothing on the Pi reads, persists, or visualizes the Shelly's status stream. The Phase 3 hub services close this gap: a Python listener subscribes to project topics and writes them to SQLite, and a Streamlit dashboard reads from SQLite to present live system state in a browser. Together they make the project observable.

**Decision.** Begin Phase 3 hub services this session with the Python listener + SQLite layer. Streamlit dashboard follows once the listener is collecting data. Each layer gets its own commit; nothing is done as one monolithic change.

**Rationale — architectural choices.**

- **Python on the Pi**, not on the developer Mac. The hub services need to run continuously alongside the broker, so they belong on the Pi. The developer Mac is portable and frequently away from the bench.
- **SQLite**, not a heavier database (PostgreSQL, InfluxDB, etc.). Single-file storage, zero administration, plenty fast for one-Hz-ish sensor telemetry from a handful of devices. A real production deployment with hundreds of nodes would warrant a time-series database; this project is one plant.
- **Streamlit**, not a custom Flask app or a heavy framework. Streamlit is purpose-built for read-only dashboards backed by Python data sources. It produces a working web app from short scripts, which is the right tool for this scale of project.
- **Subscribe-only consumer pattern.** The listener publishes nothing back to the broker. It only reads. This keeps the data flow direction unambiguous: devices → broker → listener → SQLite → dashboard. The firmware-side decision-making (state machine, fault handling) stays on the WROVER; the Pi is purely an observer until later integration work requires otherwise.

**Topic structure plan (subject to refinement during implementation).**

| Topic prefix | Purpose | Source |
|---|---|---|
| `plant/grow-light/#` | Shelly Plug US Gen4 telemetry | already publishing (DL-032) |
| `plant/sensors/#` | ESP32 WROVER sensor publishes | future Phase 2 firmware |
| `plant/state/#` | WROVER state-machine state | future Phase 2 firmware |
| `plant/commands/#` | inbound commands to WROVER | future Phase 2 firmware |
| `plant/camera/#` | image metadata / triggers | future ESP32-CAM firmware (deferred per DL-034) |

The listener subscribes to `plant/#` and routes incoming messages by topic prefix into appropriate SQLite tables.

**Implementation order.**

1. Python venv on the Pi for hub service dependencies (paho-mqtt, sqlite3 from stdlib, streamlit).
2. SQLite schema for the topic categories above.
3. Listener script that subscribes, parses, writes. Tested initially against the Shelly's existing publishing — that's a live data source we already have.
4. Listener as a systemd service so it survives reboots like the broker does.
5. Streamlit dashboard reading from SQLite. Initial scope: grow-light status, recent power readings, system uptime.
6. Streamlit as a second systemd service.

Each step gets a commit. Each step has its own README under `hub/04-listener/`, `hub/05-dashboard/`, etc.

**What this entry does not commit to.**

- The exact SQLite schema. That gets locked in when the schema is actually written, with whatever pragmatic adjustments emerge during implementation.
- Specific dashboard layout. That gets iterated based on what the data actually looks like.
- Authentication for the dashboard. The Streamlit app will initially be LAN-accessible without authentication, same security model as the broker. Adding auth is a future hardening step.
- Remote access. Cloudflare Tunnel or Tailscale (per the original project plan) is deferred until the LAN-side dashboard is working and the question "is this even useful enough to expose externally?" can be answered honestly.

**Alternatives considered.**

- **No persistent storage, dashboard reads live from MQTT.** Rejected — loses all history, can't show trends, can't survive page reload.
- **InfluxDB + Grafana** instead of SQLite + Streamlit. Rejected — substantially more setup and operational overhead for a project of this scale; useful at the "many nodes, long history, multi-user dashboarding" tier but premature here.
- **Skip the listener, just have Streamlit subscribe directly.** Rejected — the listener-as-persistence separation is exactly what makes the dashboard restart-safe and history-aware. Conflating the two would create reliability problems on the first browser refresh.

**Implementation status as of 2026-06-03.**

The listener and SQLite schema described above were implemented and validated end-to-end this session. Specifically:

- Python virtual environment created at `~/plant-hub/venv` on the Pi with `paho-mqtt` installed
- Six-table schema written to `~/plant-hub/schema.sql` and applied via `sqlite3 plant.db < schema.sql` — all tables created with the indexes and foreign keys as designed (foreign keys enforced per-connection via `PRAGMA foreign_keys = ON` in the listener)
- Listener written to `~/plant-hub/listener.py` (~270 lines), subscribed to `plant/#` and ran for ~10 minutes against the live Shelly publishing
- Real data captured into the database: continuous voltage/current/power/frequency/temperature/cumulative-energy readings in `sensor_readings`; grow-light state changes (init/on/off) in `actuator_events`; online/offline status in `system_status`; raw archive of every message in `mqtt_messages`
- Operator commands (mosquitto_pub to turn the grow light on/off) round-tripped correctly: command → broker → Shelly → relay closes → status published → listener observes → row written to `actuator_events` with `source="mqtt"`
- Graceful shutdown (SIGINT/SIGTERM) updates `runs.ended_ts` correctly, verified across multiple runs

**Friction worth recording.**

- A typo in the MQTT password during environment-variable setup caused ~20 minutes of false-trail debugging (initially attributed to paho-mqtt vs mosquitto_sub protocol differences, MQTT 5 vs 3.1.1, broker ACLs, etc.). Root cause was a single-character typo introduced via `read -s` silent input. Fix in this session: switched to an alphanumeric password that can be re-typed accurately under silent-input conditions. Long-term fix: credentials will live in gitignored config files, not be typed at the shell, once the listener becomes a systemd service.
- The Shelly's UI hides stored passwords on page reload — even with the show-password eye icon — for security. There is no way to re-verify a stored Shelly password by inspection; only by testing whether authentication succeeds. Worth remembering for future credential rotations.
- The `mosquitto_passwd` command emits a warning about the password file group ownership ("group is not root. Future versions will refuse to load this file"). The current ownership (`root:mosquitto 640`) is intentional per DL-030 — the broker process needs group-read access to validate credentials. The warning may eventually become an error in newer mosquitto versions, which will require a different ownership scheme; not a blocker today.

**Committed in this session.**

- `hub/04-listener/schema.sql` — the six-table schema, idempotent
- `hub/04-listener/listener.py` — the Python service
- `hub/04-listener/README.md` — setup procedure, run-identifier rules, example queries

**Still pending — next session(s).**

- Convert the listener into a systemd service so it survives reboots like the broker does
- Streamlit dashboard reading from the SQLite database
- Then a separate DL noting "Phase 3 hub services complete"

---

<a id="dl-036"></a>
### DL-036 — Listener promoted to systemd service with EnvironmentFile credentials

**Date:** 2026-06-04 · **Status:** Active. Listener now runs continuously as a system service.

**Context.** DL-035 implemented the Python listener as a manually-launched script. That was correct for development — you can run it under your shell, watch logs scroll, Ctrl+C when done. But it makes the listener exactly as available as a developer happens to be: closing the SSH session also closes the listener, every Pi reboot loses the listener, and there is no automatic recovery if the listener crashes. The hub's data-collection role is supposed to be 24/7, the same way the broker is 24/7. The listener needs to be promoted from "script the developer runs" to "service the operating system runs."

**Decision.** The listener now runs as a systemd service named `plant-listener.service`. It starts at boot after `mosquitto.service` and the network are ready, restarts itself on crash with a 10-second backoff, runs as the `basilpi` user (not root), and loads MQTT credentials from a root-only file at `/etc/plant-hub/credentials`. The service unit is committed to the repo at `hub/05-listener-service/plant-listener.service` alongside the procedure for installing it on a fresh Pi.

**Rationale — service unit design.**

- **`After=network-online.target mosquitto.service` + `Requires=mosquitto.service`.** The listener cannot do its job without the broker; making the dependency explicit means systemd will not start the listener until mosquitto is up, and will fail loudly if mosquitto disappears, rather than silently looping reconnect attempts. `network-online.target` covers the JSU_DEVICE WiFi being available; without this, the listener would race the WiFi association at boot.
- **`User=basilpi`, not root.** The listener does nothing that needs root privilege. It reads from the broker, writes to a SQLite file in `~basilpi/plant-hub/`, and reads its credentials from a root-readable env file. Running as a non-privileged user limits the blast radius if the Python code is ever compromised — a credential leak doesn't become a root-shell leak.
- **`Restart=on-failure` with `RestartSec=10`.** Crashes (paho-mqtt protocol error, OOM, transient SQLite lock) get an automatic restart after a 10-second backoff. The 10 seconds is deliberately long enough to (a) avoid hammering a broken broker with reconnect attempts during a real outage, and (b) leave time for an operator to notice and intervene before a hot-loop restart cycle hides the underlying problem. Not `Restart=always` — clean exits (e.g., the listener choosing to terminate) should stay terminated, not be force-revived.
- **`EnvironmentFile=/etc/plant-hub/credentials`.** Credentials live in a file owned by `root:root` with permissions `600`. systemd reads the file at service-start time, exports the variables into the listener's environment, and the listener picks them up via `os.environ` exactly as in the manual-run workflow. The credentials file is *not* committed to the repo (`/etc/` is outside the repo entirely); the repo only commits the unit file that *references* the path.
- **`StandardOutput=journal` + `StandardError=journal`.** Listener log output goes to the systemd journal, queryable via `journalctl -u plant-listener`. This replaces the previous workflow of "watch the listener's stdout in a terminal" with a structured, time-ordered, size-capped log store that the OS manages automatically. Existing Python `logging` calls work unchanged — they write to stdout/stderr, which systemd intercepts.
- **`SyslogIdentifier=plant-listener`.** Makes the journal entries readable: each line is prefixed with `plant-listener[PID]:` so the source is unambiguous when multiple services are logging concurrently.

**Rationale — credentials in `/etc/plant-hub/credentials`.**

The DL-035 development workflow had the operator set `MQTT_USER` and `MQTT_PASS` in the shell before running `python listener.py`. That was acceptable for ad-hoc bench work, but for a permanent service it has problems: the credentials would have to be set in the listener's parent process at startup, which means either a shell wrapper, hard-coded values in the service file, or an EnvironmentFile.

EnvironmentFile is the standard pattern and the one chosen here. A few specific properties of `/etc/plant-hub/credentials`:

- **Path under `/etc/` rather than `/home/basilpi/`.** `/etc/` is the conventional location for system-level configuration; using it signals that this file belongs to the system, not to any one user. It also keeps the credentials out of any user's home directory backup.
- **Directory mode `700`, file mode `600`, owned by `root:root`.** Only root can read the file. systemd reads it at service start because systemd itself runs as root. The listener process — running as `basilpi` — never reads the file directly; it inherits the environment variables systemd exports. So even compromising the listener wouldn't directly expose the file.
- **EnvironmentFile syntax, not shell.** Lines are `KEY=VALUE`, no `export`, no quotes around values, no comments after `#` (systemd is stricter than bash here). This is a small but real footgun — pasting in shell-style `export MQTT_USER="basilmqtt"` would break.
- **Includes `RUN_PHASE=phase3_hub_service`.** This is non-credential but useful: it changes the listener's auto-generated `run_id` prefix from `auto_...` to `phase3_hub_service_...`, so the runs table tells the truth about which kind of run it is. Putting it in the env file means we don't have to remember to set it manually.

**Verification — auto-restart actually works.**

`systemctl status` reporting `Active: active (running)` is a static check — it only confirms the service is currently up. The harder question is whether systemd will recover from an unexpected death. To test:

```text
sudo systemctl show plant-listener.service --property=MainPID
  MainPID=4031
sudo kill -9 4031
# Wait 15 seconds (RestartSec + buffer)
sudo systemctl show plant-listener.service --property=MainPID
  MainPID=4064
```

Different PID confirms systemd noticed the death, observed the 10-second restart delay, and started a new instance. The `runs` table also shows two rows after this exercise — the original (no `ended_ts`, because SIGKILL bypasses the listener's graceful shutdown handler), and the restart (no `ended_ts` either, because it's still running). The crashed run's missing `ended_ts` is preserved as a forensic signal, not papered over.

**Implications and what changes for the project.**

- **The listener is no longer tied to a developer session.** Closing all SSH connections, walking away from the lab, restarting the Pi — none of these stop data collection any more.
- **The runs table is now noisier.** Every Pi reboot starts a fresh run; every service restart (crash or planned) starts a fresh run. This is the right behavior — each run is a distinct continuous-collection window — but it means analyzing data "across all runs since deployment" needs to ignore the run boundaries unless they matter (which they usually don't for telemetry).
- **Credentials are no longer typed at the shell.** This unblocks the password-rotation policy noted at the end of last session: from here on, the MQTT password lives only in `/etc/plant-hub/credentials` and the Shelly's stored configuration. Operators rarely type it. When we rotate, the procedure is: update the broker, edit the env file, restart the service, update the Shelly. Four steps, no shell history exposure.

**What this entry does not commit to.**

- **No log retention policy.** systemd journal does its own size-capped rotation; if the Pi runs out of disk for any reason, the journal is the first thing to get trimmed. For a long-running deployment, we'd want explicit retention rules. Not blocking.
- **No alerting if the service fails.** If the listener enters a crash-loop or fails repeatedly, no one is notified. The dashboard (Streamlit, coming next) will eventually surface "no data in the last N minutes" as a visible signal.
- **No SQLite backup.** Database is on the Pi's SD card. If the card fails, all collected data is lost. Acceptable for a research/portfolio project; a real deployment would back up `plant.db` to external storage periodically.
- **The credentials file is single-user.** When the project grows to multiple service accounts (e.g., a read-only dashboard user vs. the publish-capable listener user), the env-file approach scales to one file per service; we don't need to redesign now.

**Alternatives considered.**

- **Run the listener under `screen` or `tmux`.** Rejected — these are session managers, not service managers. They survive logout but not reboot, do not restart on crash, and have no integration with system logging. They're for "I want to keep my interactive session alive across disconnects," not "this is a permanent system component."
- **Hardcode credentials in the .service file's `Environment=` directives.** Rejected — this means the credentials end up in `/etc/systemd/system/plant-listener.service`, which we commit to the repo. EnvironmentFile keeps the path in the unit file and the actual values out of the repo.
- **Run as root for "simpler" file access.** Rejected categorically — there is no part of the listener's job that needs root. Running services as root is the default mode of every IoT-device security disaster.
- **Use a different service supervisor (supervisord, runit, custom shell loop).** Rejected — systemd is what the Pi already uses for every other service, has the dependency model we need (`After=`, `Requires=`), and is the operating-system-standard pattern. Adding a second supervisor would mean two ways to start/stop things, two log destinations, and two operator mental models.

**Files committed in this session.**

- `hub/05-listener-service/plant-listener.service` — the systemd unit file
- `hub/05-listener-service/README.md` — installation procedure, operational reference, rotation procedure

**Files explicitly not committed.**

- `/etc/plant-hub/credentials` — contains the MQTT password. Lives only on the Pi.
- `/etc/systemd/system/plant-listener.service` — the in-place copy used by systemd. The repo holds the canonical source at `hub/05-listener-service/plant-listener.service`; the Pi's copy is installed from there.

---

<a id="dl-037"></a>
### DL-037 — Streamlit dashboard with UTC-storage / local-time-display convention

**Date:** 2026-06-04 · **Status:** Active. Dashboard is LAN-accessible read-only; further enhancements (systemd service, Tailscale remote access) tracked separately.

**Context.** With the listener writing structured telemetry to SQLite (DL-035) and running autonomously as a systemd service (DL-036), the project has a continuous stream of data but no way to look at it without writing SQL queries by hand. The dashboard closes that gap: a browser-accessible view that any operator (developer or otherwise) can read at a glance, on a phone or laptop, on the same LAN as the broker. The dashboard is the visible product — until it exists, the hub services are difficult to demonstrate.

**Decision.** A read-only Streamlit dashboard runs on the Pi, binds to `0.0.0.0:8501`, and is reachable from any device on JSU_DEVICE WiFi at `http://10.6.19.139:8501`. The dashboard refreshes every 10 seconds via `streamlit-autorefresh`. It reads the SQLite database in read-only mode (`file:...?mode=ro&uri=true`) so it cannot interfere with the listener's writes. The visual design follows a light cream botanical theme with green as the primary color and semantic status colors (green / amber / red / gray) for state communication.

**Rationale — Streamlit instead of a custom web framework.**

The mockup that informed the visual design (ChatGPT-generated reference) is genuinely well-composed and could be replicated more precisely with a Flask or FastAPI backend serving custom HTML/CSS. That was considered and rejected for this phase. Streamlit gets to roughly 80% of the mockup's polish in roughly 5% of the implementation time. The remaining 20% of polish (pixel-perfect sidebar, decorative botanical elements, custom-styled tables) is not load-bearing for the project's portfolio or demonstration value. The honest trade-off: Streamlit imposes layout constraints that a custom frontend would not, but at this phase of the project the time savings are worth more than the design control. Phase 4 or later could revisit if needed.

**Rationale — read-only v1.**

The dashboard could include controls (toggle grow light, send pump command, acknowledge fault). Those are deliberately excluded from v1. Reasons:

- **Safety surface area.** A dashboard with controls is a security and access-control problem. Anyone who can reach the URL can turn things on and off. LAN-only access partially mitigates this, but adding controls would push us to "we need auth before this is acceptable," which is a meaningful additional scope.
- **The system is still being validated.** A read-only dashboard is a passive observer of the system; it cannot make the system worse. Controls would make the dashboard part of the system's safety-critical path, which raises the bar for testing.
- **Existing control paths work.** Operator control already works via `mosquitto_pub` from the command line, the Shelly mobile app, and HomeKit. The dashboard doesn't need to be the third control surface today.

Controls are tracked as a future enhancement and will be added after the system is more mature and an authentication story exists.

**Rationale — light cream theme rather than dark.**

For a project-presentation context (portfolio review, demo to a non-technical audience), light themes read more professional and more like a polished consumer product. They photograph better in screenshots. They project credibility — "this is a real thing someone built" — more strongly than dark themes, which read as developer tools. Dark mode is a reasonable later enhancement once the dashboard is live, but it's not the first impression we want today.

Green primary follows from the project's identity. The color choice is not arbitrary — it's thematically appropriate, conveys "living systems," and reads well against the cream background. Semantic status colors layered on top (amber for warning, red for fault, gray for unknown) follow universal operator-dashboard conventions and were not optional — without them, every state reads the same, which makes the dashboard useless during incidents.

**Rationale — store UTC in the database, display America/Chicago in the UI.**

This is the single most important non-aesthetic decision in this entry and worth recording carefully because it applies to every timestamp the project will ever generate, including future ESP32 publishes, sensor data, and event logs.

**Why UTC in storage.**

- **Daylight saving creates ambiguous local timestamps.** When CDT falls back to CST in November, the hour from 1:00 AM to 2:00 AM local repeats. A row written at "2026-11-01 01:30:00" local time could refer to either pass through that hour; there is no way to tell from the local timestamp alone. UTC has no daylight saving, so every UTC instant maps to exactly one moment in time. The database can answer "what happened between time A and time B" with no ambiguity.
- **Devices in different timezones can all write to the same database without coordination.** The Shelly publishes UTC. The Pi runs UTC internally. The eventual ESP32 nodes will publish UTC. If they each wrote local time, comparing their timestamps would require knowing what timezone each device was in at the time of writing — which the database would not record.
- **Moving the deployment doesn't corrupt historical data.** If the Pi ever moves to a different timezone (lab relocation, travel, daylight saving transition), UTC timestamps in past rows remain correct. Local-time storage would make every historical row mean something different.
- **It is the universal industry convention.** Every cloud telemetry platform (AWS CloudWatch, Datadog, InfluxDB, Grafana) follows this pattern. The systemd journal stores UTC. Mosquitto logs are UTC. Storing UTC keeps the project consistent with the rest of the ecosystem and makes future integrations straightforward.

**Why America/Chicago in display.**

- The Pi is physically deployed in Jackson, Mississippi, which is Central Time. Operators looking at the dashboard expect to see times in the timezone where the events actually happened locally.
- Hardcoding to `America/Chicago` (not `CST` or `CDT` as fixed offsets) lets Python's `zoneinfo` library handle daylight saving transitions automatically. The dashboard displays CDT in summer and CST in winter without any code changes or operator action.
- Hardcoding the timezone rather than auto-detecting from the viewer's browser is the right choice for this project's scope. Auto-detection adds JavaScript complexity and makes the dashboard "mean different things to different viewers," which is confusing when troubleshooting. "All times shown are Central, where the system lives" is mentally simpler.

**Implementation: a single helper function in the dashboard.**

```text
LOCAL_TZ = ZoneInfo("America/Chicago")

def format_local(ts_str: str) -> str:
    ts = pd.to_datetime(ts_str, utc=True)
    return ts.tz_convert(LOCAL_TZ).strftime("%Y-%m-%d %H:%M:%S %Z")
```

Every timestamp the dashboard shows passes through this function or its pandas equivalent (`pd.to_datetime(..., utc=True).dt.tz_convert(LOCAL_TZ)` for chart axes). The database is untouched; storage stays UTC; display becomes local.

**The Pi's system clock.** The Pi itself runs UTC at the OS level (verifiable with `timedatectl`). This was not changed for this entry. Keeping the Pi on UTC means everything — kernel log timestamps, systemd journal entries, mosquitto logs, the listener's `datetime.now(timezone.utc)` — agrees on the same time reference. Operators occasionally SSH into the Pi and need to read these logs; "all logs are UTC" is the disciplined convention. Local-time display is purely a dashboard rendering concern.

**Mobile-friendly layout.**

The dashboard was designed with a mobile-first sensibility: single-column-friendly card layouts, large tap targets, font sizes that remain legible on iPhone Safari without zoom. Streamlit's wide layout reflows reasonably on narrow viewports; custom CSS in the dashboard tightens the mobile case further (smaller card padding on screens below 768px). Validated visually on an iPhone via Safari.

**LAN-only for now; Tailscale planned.**

The dashboard binds to `0.0.0.0`, meaning it accepts connections on any network interface. On JSU_DEVICE this means any device on the same WiFi network can reach `http://10.6.19.139:8501`. This is acceptable because the same network already has unauthenticated access to mosquitto on port 1883, and the dashboard exposes strictly less attack surface than direct broker access (read-only SQLite queries vs. the broker's full pub/sub capability).

Remote access from outside JSU_DEVICE (e.g., from home, from travel) is **not** addressed by this entry and is deferred to a future session. The planned approach is **Tailscale** — a zero-trust VPN that gives each device a private address reachable only by other devices on the same tailnet, with no public-internet exposure of the Pi. Cloudflare Tunnel was also considered but rejected for this project's scope; we don't need a public URL, just personal access.

**What this entry does not yet commit to.**

- Auto-start of the dashboard at boot. Currently the dashboard is manually launched. The next step is a `plant-dashboard.service` systemd unit following the same pattern as `plant-listener.service` (DL-036).
- Tailscale setup. Deferred to a separate session.
- Authentication. Not present; not needed for LAN read-only.
- Dark mode. Mentioned as a future polish item; not in scope here.
- Sidebar navigation. The mockup shows a sidebar with multiple pages (Overview, Environment, Events, Alerts, Settings); the v1 dashboard is single-page. Sidebar comes later if and when more pages exist.

**Alternatives considered.**

- **Custom Flask/FastAPI backend with hand-written HTML/CSS.** Rejected for this phase — substantially more implementation work, separate deployment story, and the gains (pixel-perfect design control) are not load-bearing for the project's value.
- **Grafana fronting an InfluxDB time-series database.** Rejected — both Grafana and InfluxDB are excellent at this category of work, but introducing them now would require migrating data out of SQLite, learning two new systems, and managing their lifecycles. SQLite + Streamlit gives 80% of the experience at 10% of the operational complexity for our scale.
- **Node-RED or Home Assistant.** Both could be made to work for this use case. Both are heavier than this project needs, and both impose their own opinionated data models that the project would have to adapt to.
- **Auto-detect viewer's local timezone via JavaScript.** Rejected per the rationale above — adds complexity, makes the dashboard mean different things to different viewers, and isn't worth it for a single-deployment-location project.

**Files committed.**

- `hub/06-dashboard/dashboard.py` — the Streamlit app (~340 lines)
- `hub/06-dashboard/.streamlit/config.toml` — Streamlit theme configuration
- `hub/06-dashboard/README.md` — setup, operational reference, visual examples
- `docs/images/dashboard-desktop-{1,2,3}.png` — desktop browser screenshots
- `docs/images/dashboard-mobile-{1,2}.png` — iPhone Safari screenshots

**Lesson worth keeping.**

The "store UTC, display local" pattern is a foundational design decision that scales across every part of the system. Recording it now, in this entry, means it stays consistent across future ESP32 publishes, sensor data, log files, and any future dashboard panels. Inconsistency in timestamp handling is one of the most common silent bugs in telemetry systems; making the convention explicit and documented keeps the project on the right side of it.

---

<a id="dl-038"></a>
### DL-038 — Tailscale for remote dashboard access; tailnet IP as canonical URL

**Date:** 2026-06-04 · **Status:** Active. Remote access working end-to-end; Phase 3's stated remote-access goal achieved.

**Context.** With the broker, listener, dashboard, and their respective systemd services all running on the Pi (DL-027 through DL-037), the LAN-side of Phase 3 was complete. But one of the project's original goals was that the system remain observable while the operator is away from the lab — travel, home, anywhere with internet. LAN-only access doesn't satisfy that. This entry adds remote access via Tailscale and records the network architecture and operational implications.

**Decision.** Tailscale is installed on the Pi, the developer Mac, and the developer iPhone. All three are on the same tailnet, authenticated via Google SSO. The Pi has a stable tailnet IP (`100.79.225.18`) that resolves to the Pi from any device in the tailnet, regardless of which physical network either end is on. **The tailnet IP is now the canonical URL for the dashboard** (`http://100.79.225.18:8501`); the LAN IP remains valid as a fallback but is no longer the recommended access point.

**Rationale — why Tailscale specifically.**

Several remote-access patterns were considered. The shortlist:

- **Tailscale** — chosen. WireGuard-based mesh VPN. Each device gets a private address in the `100.64.0.0/10` range, reachable only by other devices on the same tailnet. The Pi never has a port open to the public internet. The iOS app is well-built and matters because the operator's primary mobile device is iPhone. Free tier supports 100 devices, far beyond what we'll ever need.
- **Cloudflare Tunnel** — rejected for this project's scope. Would have given the dashboard a public-facing URL (e.g. `dashboard.example.com`), which we don't need; we don't want to share the dashboard with anyone, just access it ourselves. Public URL also raises the bar for authentication — anyone who guesses the URL would otherwise reach it. Tailscale's "only my devices can reach this" model is closer to what we actually want.
- **ZeroTier** — similar idea to Tailscale, slightly older. Comparable security model, slightly worse iOS UX. Tailscale won the comparison on polish.
- **Self-hosted WireGuard** — full control, but substantially more setup (running a WireGuard server somewhere with a public IP, managing keys manually). Not justified for this project's scale; Tailscale gives us the same WireGuard transport with managed key distribution and a friendly UI.
- **SSH tunnel** — works only from one device at a time, doesn't work easily on mobile, not really a remote-access solution at scale.
- **Ngrok / similar** — public URLs with random subdomains, free tier has time limits and unpredictable URLs. Suitable for sharing a dev preview, not for permanent operational access.

The deciding factors for Tailscale were:
1. **It punches through NAT and firewalls automatically.** This matters specifically because the Pi is on JSU_DEVICE — a university network the operator does not control. No port forwarding is possible. Tailscale's automatic NAT traversal solves this without any router/firewall changes.
2. **The Pi is never exposed to the public internet.** Tailscale's connections are end-to-end encrypted and reachable only by authenticated devices on the same tailnet. There is no `dashboard.somewhere.com` URL for an attacker to even find.
3. **It survives the Pi's IP changing.** The tailnet IP `100.79.225.18` is permanent (until the device is removed from the tailnet). If JSU's DHCP ever reassigns the Pi's LAN IP, the tailnet IP is unaffected.

**Setup procedure (for the record).**

On the Pi:

```text
curl -fsSL https://tailscale.com/install.sh | sh
sudo tailscale up
# URL printed to terminal -> open in browser -> Authorize -> Pi appears in admin console
```

On the Mac and iPhone:

- Install Tailscale from the Mac App Store / iOS App Store
- Sign in with the same SSO account that owns the tailnet
- Tailscale automatically joins the existing tailnet

No router configuration, no port forwarding, no Pi-side firewall changes. The Pi's tailnet identity is persistent across reboots because the `tailscaled` daemon runs as a system service that auto-starts.

**Verification — the actual remote-access milestone.**

The deciding test was performed by:
1. Connecting to the dashboard at `http://100.79.225.18:8501` from the Mac on JSU_DEVICE WiFi — worked.
2. Connecting from the iPhone on JSU_DEVICE WiFi — worked.
3. Turning WiFi off on the iPhone, switching to cellular data — Tailscale stayed connected — refreshing the same URL — **dashboard loaded over cellular.** This is the actual proof: the operator is no longer tied to JSU_DEVICE to see the system.

**On the LAN IP behavior with Tailscale installed.**

After Tailscale was installed on the Mac, the LAN IP (`http://10.6.19.139:8501`) became unreliable from some browsers on the Mac. Chrome consistently failed with `ERR_ADDRESS_UNREACHABLE`; Safari was intermittent. The Pi itself remained reachable on the LAN — the listener and dashboard processes are unchanged, Streamlit binds to `0.0.0.0` and listens on every interface — but the Mac's routing decisions for `10.6.19.139` are affected by Tailscale's installed network routes in ways that are browser-dependent and not consistently reproducible.

This is a known interaction pattern with Tailscale on macOS: the OS's routing table is altered, browser network sandboxes cache routing decisions, and the combination produces "the LAN IP sometimes doesn't work from this specific machine" until the cache is cleared or the system network state is refreshed.

The honest assessment: this is a client-side quirk on the Mac, not a server-side problem. The Pi continues to serve `10.6.19.139:8501` correctly to any other LAN client (the iPhone, a fresh-install browser, a different Mac without Tailscale running). It is also not a project-functionality problem: the tailnet IP works from every device on the tailnet, from every network, every time.

**Decision: the tailnet IP becomes the canonical URL.** The LAN IP is no longer the recommended access point; it remains valid as a fallback and is still used by the Shelly to reach the broker, which is unaffected. This change matters in two places:

- The `hub/06-dashboard/README.md` should recommend `http://100.79.225.18:8501` first, with the LAN IP as a footnote.
- The `docs/explainers/phase3-hub.md` should mention that the building's "front door" is now accessible from anywhere, not just from inside JSU_DEVICE.

**Security posture, honestly.**

Adding Tailscale to a project is not a free pass on security; it shifts the threat model rather than removing it.

- **The dashboard still has no authentication.** Anyone on the tailnet can reach it. Currently the tailnet has three members: the Pi, the Mac, and the iPhone, all owned by the same person. If a future fourth device joined (e.g. for sharing with a collaborator), they would have read access to all current plant telemetry. Acceptable for the current single-operator state; revisit before adding more tailnet members.
- **The broker still has no TLS.** MQTT traffic between the Shelly and the Pi is plaintext on JSU_DEVICE WiFi. Tailscale only encrypts the dashboard access, not the broker's LAN-side traffic. Adding TLS to mosquitto is tracked as a future hardening step.
- **The Pi's other services are not exposed via Tailscale by default.** Only services bound to `0.0.0.0` (or to the tailnet interface explicitly) are reachable from the tailnet. SSH is bound to all interfaces and is therefore tailnet-reachable, which is convenient for remote administration but means an attacker who compromised the operator's Google SSO account could SSH into the Pi. This is currently the worst-case scenario; mitigations would be SSH key-only login (already enabled) and Tailscale ACLs (deferred).

**What this entry does not yet commit to.**

- **Tailscale ACLs.** Tailscale supports per-device, per-port access rules. Currently every device on the tailnet can reach every service on every other device. For the current three-device tailnet this is acceptable. Adding ACLs makes sense when more devices join or when collaborators are added.
- **MagicDNS / friendly hostnames.** Tailscale offers `planthub.tail<random>.ts.net`-style hostnames so you don't have to remember the IP. Useful, but not enabled in this entry; the IP is fine for now.
- **Cloudflare Tunnel or other public exposure.** Rejected for this project; not needed.
- **Authentication on the dashboard.** Deferred per the security-posture note above.

**Alternatives considered, rejected.**

- **VPN into JSU's network.** University VPN access is per-user and might not allow the operator to reach LAN devices on JSU_DEVICE. Tailscale sidesteps this entirely by not relying on JSU's network infrastructure.
- **Port forwarding through a home router** (if the project were at home instead of JSU). Would have required exposing the Pi to the public internet, which categorically increases attack surface. Tailscale's "no public exposure" property is the architectural improvement.

**Phase 3 close-out.**

This entry completes the original Phase 3 plan:
- Broker + Shelly integration (DL-027, DL-029, DL-030, DL-031, DL-032)
- Pi power and autonomous operation (DL-033)
- Listener and SQLite schema (DL-035)
- Listener as a system service (DL-036)
- Dashboard with UTC-storage / local-time-display (DL-037)
- Remote access via Tailscale (this entry)

Phase 3's deliverable — *"a Pi-based hub that runs autonomously, collects telemetry from a real device, persists it queryably, presents it visually, and is observable from anywhere"* — is genuinely done.

---

<a id="dl-039"></a>
### DL-039 — Shelly unexpected reboot diagnosed from telemetry; "Restore last state" mitigation applied

**Date:** 2026-06-05 · **Status:** Active. Mitigation deployed; root cause undetermined but observable if it recurs.

**Context.** While reviewing the dashboard the day after the Phase 3 close-out, an unexplained gap was observed in the power-draw chart: the grow light had been ON, drew ~15W for several hours, then dropped to 0W for over twelve hours overnight, and came back ON the following morning. The Recent Activity log showed a corresponding row at 10:12 CDT with `source: init`. This was unexpected — the grow light should have stayed on. The system's own telemetry was the diagnostic tool.

**What the telemetry showed.**

Querying `system_status` for the surrounding window:

```text
sqlite3 plant.db "SELECT ts, status, metric, value FROM system_status
                  WHERE device='grow-light'
                  AND ts BETWEEN '2026-06-05T14:50:00Z' AND '2026-06-05T15:30:00Z'
                  ORDER BY ts;"
```

Returned (UTC times):

| ts | status | metric | value |
|---|---|---|---|
| 15:11:07 | offline | — | — |
| 15:11:07 | online | — | — |
| 15:11:07 | online | broker_connected | 1.0 |
| 15:11:07 | online | cloud_connected | 1.0 |
| 15:11:08 | online | uptime | 6.0 |
| 15:11:08 | online | ram_free | 174832.0 |

The story these rows tell: at 15:11:07 UTC (10:11 CDT) the Shelly's MQTT last-will fired (`offline`), followed almost immediately by a fresh `online` status. One second later, the Shelly was publishing system status with `uptime: 6.0`. A six-second uptime conclusively means the Shelly had just completed a power-on cycle — this was a full reboot, not a network blip. When a Shelly reconnects from a network-only outage, its uptime continues from where it was; only a hard restart resets the counter.

Roughly one minute later, the Shelly published its `status/switch:0` message with `output: false, source: init`, which the listener captured into `actuator_events`. The `init` source explicitly indicates the relay state at boot, distinct from operator-driven (`mqtt`, `HTTP_in`, `SHC`) state changes.

**Why the relay was off after the reboot.**

By default, Shelly relays initialize to the OFF position when their firmware boots, regardless of the relay's state before the reboot. This is a configurable setting in the Shelly's per-output configuration — typically labeled "Default mode" or "Power-on default" — and is controllable from the device's own web UI. The default value is "Always off," which is the safe choice from the Shelly's perspective (a power-cycled device shouldn't surprise the operator by drawing load) but the wrong choice for a project that expects the grow light to stay on across unexpected reboots.

**Mitigation applied.**

Changed the Shelly's output default-state setting from "Always off" to **"Restore last state"** (equivalent labels across firmware versions: "Match last state," "Last known state"). With this change, the Shelly's relay state is preserved across reboots — if the grow light was on before a reboot, it comes back on automatically after the reboot.

This is the right setting for any output that participates in an autonomous control loop. The "Always off" default makes sense for a smart plug controlling a single user-facing appliance (a lamp, a fan) where the user wants explicit control; it does not make sense for an output that is part of a longer-running automation that should be self-healing.

**Root cause — honest assessment.**

The telemetry tells us **that** the Shelly rebooted but not **why**. Plausible causes ranked by likelihood:

1. **Brief power glitch at the wall outlet.** JSU's mains is not guaranteed clean. A sub-second voltage sag could trigger the Shelly's internal under-voltage protection and force a reboot. Not investigable after the fact. The Shelly is plugged into a surge-protected power strip but surge protection ≠ UPS.
2. **Internal firmware watchdog.** Shelly devices include a watchdog timer that reboots the device if the firmware becomes unresponsive (memory leak, deadlock, internal fault). This is a self-recovery mechanism, not a malfunction. The 6-second clean boot is consistent with a watchdog reboot.
3. **WiFi-related watchdog.** If WiFi association is lost for an extended period, some Shelly firmware versions will reboot to attempt re-connection. The MQTT last-will fired at the same second as the new online status, which doesn't fit a long WiFi outage but doesn't rule out a brief one followed by a reboot.
4. **Firmware auto-update.** Eliminated. Auto-update is confirmed disabled in the Shelly's settings, and the firmware version was unchanged before and after the event.
5. **Manual reboot via the app.** Eliminated. No deliberate reboot occurred.

The root cause is undetermined and may remain so unless the event recurs frequently enough to correlate with another observable factor (time of day, weather, building-wide power events).

**What this entry establishes for future operations.**

- **Telemetry-driven diagnosis works.** The listener captured exactly what was needed to understand the event — last-will timing, uptime reset, init-source state change — without anyone needing to be present when the event happened. This is the actual payoff of the Phase 3 hub: events that would otherwise have been mysteries are observable in retrospect.
- **The `init` source value in `actuator_events` is meaningful.** Until this entry, "init" was just one of several possible values for the `source` column. Now it's clear: `init` means "the Shelly published its state because it just booted, not because anyone or anything asked it to change state." Future analysis should treat `init` rows as reboot markers, not as user actions.
- **A small subset of `system_status` rows are reboot indicators.** Specifically: an `online` status with a small `uptime` value (less than ~30 seconds) following a recent `offline` row is a strong reboot signature. This is queryable.
- **Mitigation does not equal explanation.** "Restore last state" makes the system robust to future reboots but does not prevent them. If the reboots become frequent, that's a separate problem requiring further investigation.

**A reusable query pattern for "did the Shelly reboot recently?"**

```text
sqlite3 plant.db "SELECT ts, value AS uptime_seconds
                  FROM system_status
                  WHERE device='grow-light' AND metric='uptime' AND value < 60
                  ORDER BY ts DESC LIMIT 10;"
```

Returns the most recent ten short-uptime samples, each of which is likely a reboot. Useful for trend analysis if reboots become frequent.

**What this entry does not yet commit to.**

- **No UPS or battery backup for the Shelly.** A small in-line UPS at the wall outlet would absorb brief power glitches and likely eliminate the most likely root cause. Not justified for a research/portfolio project; revisit if reboots become frequent.
- **No automatic alerting on reboot detection.** Currently a reboot is detectable only by manual query. A future enhancement could publish a fault_event when an `init`-source actuator event arrives, or when an uptime row resets, so the dashboard's faults panel surfaces it.
- **No corresponding firmware-side action.** When Phase 2 firmware is being designed, its state machine should not assume the grow light's relay state is preserved across its own restarts; it should query the current state from the broker on startup. The "Restore last state" setting is helpful but not a substitute for firmware that handles the case correctly.

**Alternatives considered, rejected.**

- **Leave the default "Always off" and let the firmware re-enable the relay.** Rejected — the firmware doesn't exist yet, and even when it does, relying on a separate process to recover is more failure-prone than letting the Shelly handle it natively.
- **Add a periodic "ping" from the Pi that re-asserts the desired relay state.** Rejected — adds complexity, doesn't address other reboot scenarios, and creates a polling loop on top of an already-reactive system.
- **Investigate firmware-side instrumentation in the Shelly to determine reboot cause.** Rejected for this entry — the Shelly is closed-source; we can read what it publishes but cannot get a kernel-level reboot reason. Worth revisiting only if reboots become a chronic issue.

**Rejected: inferring actuator events from power readings.**

A proposal was raised to synthesize rows into `actuator_events` from power-reading transitions (low→high = inferred ON, high→low = inferred OFF, tagged `source = inferred_power`). Rejected because it conflates *commanded state* with *measured electrical behavior* — two semantically different things the current schema correctly separates. It would also double-count real toggles and hide physical anomalies under what looks like a normal state transition.

The real underlying need — detecting when reality and intent disagree — belongs in `fault_events`, not `actuator_events`, and requires a notion of *intent* that doesn't exist yet. That comes in Phase 2 when the WROVER state machine asserts desired state. Revisit then.

**Files touched.** None this session — the change was a setting on the Shelly's own configuration interface, not project code. The decision log entry is the durable record.

---

<a id="dl-040"></a>
### DL-040 — Phase 2 WROVER integrated firmware: architectural principles

**Date:** 2026-06-05 · **Status:** Active. Phase 2 begins; this entry establishes the principles the firmware will be built around, before any code is committed.

**Context.** Phase 1 validated 11 of 12 hardware components individually on the WROVER bench (DL-015 through DL-026). Phase 3 brought the Pi-side infrastructure online: broker, listener, dashboard, all autonomous (DL-027 through DL-038). The hub already expects messages from devices; the Shelly is the only one publishing today. Phase 2 produces the firmware that turns the WROVER from a collection of validated bench components into an actual plant-care system that reads sensors, makes decisions, controls actuators, publishes telemetry, and operates autonomously for weeks at a time.

The project is in an unusual position — Phase 3 was built before Phase 2. This means the firmware is being designed into a *running* system, not into a vacuum. The broker is up, the listener is waiting, the dashboard's "Plant environment" panels are sitting empty waiting to populate. That changes the design space: the firmware doesn't have to invent everything from scratch, but it does have to honor the conventions Phase 3 established.

**Decision.** Phase 2 firmware is built around the principles below. Specific implementation choices (state count, exact module boundaries, sensor read intervals, watering algorithm) will emerge as code is written; this entry captures the *principles* that should remain stable across the implementation. The firmware lives at `firmware/integrated/` as a single PlatformIO project.

**Principle 1 — Non-blocking design.**

The firmware must never call `delay()` in the main loop. All timing is millis()-based: each periodic task tracks its own "next due" timestamp and runs when due. The loop body checks each scheduled task, runs whatever's ready, and returns immediately.

This matters because the firmware has many concurrent responsibilities — read sensors at one rate, blink status LEDs at another, check buttons constantly, publish MQTT periodically, update the OLED, respond to watering decisions, handle faults. A blocking `delay()` in any of these would stall everything else. A single `delay(1000)` while waiting for a sensor would mean the STOP button has up to a one-second response time, which is unacceptable for a safety control. The cost of getting this wrong is high and the cost of retrofitting it later is even higher, so we lock it in from the start.

**Principle 2 — Module decomposition.**

The firmware is not a single `main.cpp`. It is structured as a set of modules with clear responsibilities:

- **Sensors** — one logical interface per sensor (BME280, BH1750, soil moisture, float switch, leak sensor). Each returns a struct with the reading and a validity flag.
- **Actuators** — pump, status LEDs, buzzer, OLED. Each exposes simple operations (start/stop, on/off, set display).
- **State machine** — the control logic. What the firmware should be doing right now lives here, separate from how to do it.
- **Telemetry** — MQTT connect, publish, subscribe. Sole owner of the broker connection.
- **Config** — pin assignments, thresholds, timing constants, calibration values. Tuning happens in one file.
- **Secrets** — WiFi credentials, MQTT credentials. Gitignored. An example template (`secrets.h.example`) is committed.

The exact file layout will be informed by what makes sense in PlatformIO — likely `src/main.cpp` plus headers/implementations in `src/` or `lib/` depending on whether we want PlatformIO's library finder to manage them. The principle is that the firmware is decomposed; the exact decomposition is implementation detail.

**Principle 3 — Sensors return readings with validity flags.**

Every sensor read returns a structured result: the value, the unit, and a boolean indicating whether the reading is trustworthy. A failed I²C read, an out-of-range ADC value, an obviously-broken sensor (e.g. soil moisture reading 0 ADC counts forever) — all set the validity flag to false. The reading itself may still be present, but downstream code is required to check validity before acting on it.

This is what makes graceful degradation possible. Without per-sensor validity, the firmware would have to guess whether a value is real, or worse, treat clearly broken readings as truth and water the plant based on a soil sensor that just disconnected. With validity flags, the state machine has the information it needs to refuse to act on bad data.

**Principle 4 — State machine for control flow.**

The firmware's behavior at any instant is captured by a finite state machine. "What should we be doing right now" is encoded as a state; transitions between states have explicit conditions; the loop runs the current state's logic each tick.

The specific states will emerge as the implementation comes together. Likely candidates include Initializing, Monitoring (normal sensor-reading operation), Watering (pump active), Settling (post-water grace period before next read is trusted), Fault (unrecoverable problem requiring operator ACK), and possibly others. DL-040 does not commit to a specific state count; it commits to the FSM *pattern* as the right structure for the control logic.

The implementation should make the FSM debuggable: state transitions should be loggable to serial, the current state should be visible on the OLED and publishable as telemetry. A future operator looking at the dashboard should be able to see "the firmware is in Monitoring" or "the firmware faulted because the leak sensor tripped."

**Principle 5 — Graceful degradation: what is fatal and what is not.**

This is the most operationally important principle in the document, because it determines whether the plant survives unattended failures.

**Sensor invalidity is fatal to watering**, but not to the firmware overall. If the soil moisture sensor returns invalid for sustained reads, watering is halted. The firmware continues to run, continues to read other sensors, continues to publish telemetry, continues to respond to operator commands — but it does not water until the sensor is valid again or the fault is explicitly cleared. The reasoning: watering is the only path that can physically harm the plant (overwatering, root rot, flooding the bench). Without trustworthy moisture data, watering becomes guessing, which is unsafe.

**WiFi and MQTT loss are non-fatal.** If the broker disappears or WiFi drops, the firmware keeps running. The control loop continues. Sensors are still read. Watering decisions are still made. The plant continues to be cared for. Telemetry that would have been published is either dropped or queued (decision deferred to implementation); when connectivity returns, telemetry resumes from that point. The firmware is the authority on what the plant needs; the broker is just where the firmware announces what it's doing.

**Hard faults — leak detected, reservoir empty for too long, sensor stuck-broken for hours — halt all actuation and require operator ACK.** These are conditions where continuing automatic operation would likely cause harm. The firmware lights the red LED, posts a fault to the broker, and waits for the operator to press the physical ACK button before resuming.

**Principle 6 — Centralized configuration.**

Pin assignments, sensor read intervals, soil moisture thresholds for watering decisions, pump dose volumes, calibration constants, timing values — all live in one `config.h`. Tuning the system means editing one file, not hunting through code for magic numbers. This also means the file becomes a readable summary of the system's tunable behavior, useful for review and for future-you who has forgotten what the values mean.

**Principle 7 — Credentials never committed.**

A gitignored `secrets.h` holds WiFi SSID, WiFi password, MQTT username, MQTT password. A committed `secrets.h.example` shows the structure with placeholder values. Same pattern as the Pi-side `/etc/plant-hub/credentials` (DL-036): real values stay on the developer machine and the deployed device, the repo never sees them.

**On the broker-side conventions the firmware must honor.**

Phase 3 established a loose topic namespace plan: `plant/sensors/...`, `plant/state/...`, `plant/commands/...`. The exact JSON shapes and topic structure under these prefixes are *not* fixed by Phase 3 — the listener parses by topic prefix and writes to the raw archive regardless of message shape, and the projection logic for ESP32 publishes hasn't been written yet. So the firmware is free to propose specific shapes; the listener will be updated to project them into the structured tables.

This means message-shape design becomes part of the firmware work, not a constraint from Phase 3. When the firmware is ready to publish, we design the topic structure and JSON shapes together and update both ends.

**On the directory choice — `firmware/integrated/` rather than numbered subdirectories.**

Phase 1's `firmware/test-sketches/01-bme280/` through `firmware/test-sketches/10-leak-sensor/` were independent test sketches, each its own PlatformIO project, each validating one component in isolation. Phase 3's `hub/01-pi-setup/` through `hub/07-dashboard-service/` were distinct logical components (broker, listener, dashboard) that happen to be sequenced.

Phase 2 is different in kind. It is *one* firmware that evolves over many commits. Numbering subdirectories would create the illusion of separate firmwares; a single PlatformIO project at `firmware/integrated/` matches the actual nature of the work and follows standard PlatformIO conventions. The directory contains `platformio.ini`, `src/`, and possibly `include/` and `lib/` as the firmware grows.

**On commit sequencing.**

The firmware is not built according to a pre-planned three-commit (or N-commit) sequence. Each commit is whatever natural unit of working code emerges next: project skeleton; WiFi connect; one sensor integrated; state machine scaffold; telemetry publishing; pump path; etc. Some will be small; some will be substantial. The discipline is "real progress per commit," not "stick to a plan."

The one ordering constraint worth recording: **the watering path is the last major piece to land.** The pump is the only actuator that can physically harm the plant (overwatering, flooding). Watering depends on calibration work (dose volume → milliliters) that hasn't been done yet. It also depends on the state machine and the moisture-sensor logic being trustworthy. Adding watering before these foundations are solid would be putting the dangerous capability into the system before its safety guards are real.

**Verification approach.**

The firmware is verifiable at several layers:

- **Serial output.** Every state transition, every sensor read, every decision logged to serial. The first sanity check at every commit.
- **OLED display.** Shows current state + most recent readings, allowing visual verification without a serial console.
- **MQTT telemetry.** Once published, the dashboard's currently-empty "Plant environment" panels start populating. Visual confirmation in the browser.
- **Database queries.** The same kind of telemetry-driven diagnostic that revealed the Shelly reboot in DL-039 will work for the WROVER firmware. If something goes wrong, the database will show what happened.

**What this entry does not commit to.**

- The number of states in the FSM.
- The exact module/file layout under `firmware/integrated/src/`.
- The specific JSON message shapes for telemetry.
- The watering decision algorithm (threshold-based? hysteresis? proportional dose?).
- The dose calibration values (will be empirically measured in a future commit).
- Specific sensor read intervals.
- The handling of MQTT telemetry during disconnects (drop vs. queue, queue size if queued).
- The OLED layout and what gets displayed in each state.
- Whether sensors run in their own task on the ESP32's second core or all in the main loop.

These are all implementation decisions that will be made as code is written, recorded in subsequent DL entries when they are non-obvious or constitute reversible-but-costly choices.

**Alternatives considered.**

- **One giant `main.cpp` with no modules.** Faster to start writing, much harder to maintain or reason about beyond a few hundred lines. Rejected — even a one-person project benefits from decomposition, and this project is portfolio-visible.
- **`delay()`-based timing for "simplicity."** Common in Arduino-style sketches. Rejected — incompatible with the concurrent responsibilities the firmware has. The "simplicity" of `delay()` evaporates the moment you need two things to happen at different intervals.
- **Skip the FSM, use ad-hoc conditionals.** Workable for very simple control flow; rapidly becomes unreadable as states multiply. The FSM pattern was already validated as the right structure during Phase 1 work (DL-025, the user-feedback subsystem's four-state IDLE/ACTION/FAULT/CRITICAL FSM). Continuing that pattern is the natural choice.
- **WiFi/MQTT as fatal to operation.** Would simplify the firmware (no need to handle disconnect gracefully) but would mean the plant dies any time the broker has a hiccup. Categorically wrong given the project's "weeks unattended" goal.
- **Watering as the first major piece to land.** Sometimes recommended in "get the dangerous thing working first" school of thought. Rejected here because the dangerous thing is what you need to be most sure of, which means it should land *after* the supporting safety infrastructure (FSM, valid-sensor checks, fault handling) is in place.

**Files committed in this DL.** None — this is an architecture entry. Code follows in subsequent commits.

---

<a id="dl-041"></a>
### DL-041 — WROVER telemetry convention

**Date:** 2026-06-06 · **Status:** Active. Proven end-to-end with the first sensor (BME280).

**Context.** DL-035 built the hub but left the WROVER's topic structure and JSON shapes undefined, to be designed in Phase 2. With WiFi, MQTT, and BME280 now publishing, that convention is settled and recorded here so every later sensor follows it.

**Decision.**
- Topics: `plant/sensors/<device>` for readings, `plant/status/wrover` for presence. `plant/commands/...` and `plant/state/...` reserved for later.
- Presence: retained `{"online":true,"uptime_s","rssi","heartbeat"}` on the heartbeat cadence, plus an MQTT Last Will (retained `{"online":false}`) so the broker marks the node offline if it drops uncleanly.
- Payload: one JSON blob per sensor device (e.g. `plant/sensors/bme280` = `{"temperature_c","humidity_pct","pressure_hpa"}`), not retained.
- Projection: the listener fans each blob into one EAV `sensor_readings` row per metric. The `device` column holds the sensor-component identity (`bme280`, later `bh1750`/soil/leak/float), paralleling the existing `grow-light` device.
- Cadence: read at 2 s for control freshness, publish at 30 s to keep the archive lean; invalid readings are withheld, not published.

**Rationale.** A per-sensor blob is one atomic publish whose metrics share a timestamp and fan cleanly into the EAV table DL-035 built. Retained presence + Last Will gives a truthful online/offline signal for an unattended system. `device`=component reuses the existing schema pattern and keeps dashboard queries uniform. Read/publish decoupling avoids ~43k rows/day/metric for slow-moving air values.

**Alternatives considered.** Per-metric topics (more messages, no atomicity — rejected). Minimal ping without Last Will (no truthful offline signal — rejected). `device`=node with a 4-part topic (more scalable but unneeded for one node; noted as the extension path — briefly mis-wired as `wrover` and corrected to `bme280`). Publishing at read cadence (DB bloat — rejected).

**Files.** Firmware: `config.h`, `net_mqtt.{h,cpp}`, `main.cpp`. Hub: `listener.py`, `dashboard.py`.

---

<a id="dl-042"></a>
### DL-042 — Soil moisture: percentage mapping, sensor count, and placement

**Date:** 2026-06-06 · **Status:** Active. Sensor integrated and publishing; watering thresholds tuned later on the live plant.

**Context.** Bringing the capacitive soil probe (GPIO34) online required deciding how to report it, how many to use, and where to place it. Calibration data is the three DL-020 conditions: air ~2854, dry soil ~2523, wet soil ~1953 ADC (lower = wetter), each fluctuating ~10–16 counts.

**Decision.**
- Report both `soil_raw` (ground truth for the watering logic) and `moisture_pct` (human-readable). Percent maps dry anchor (2523) = 0% to wet anchor (1953) = 100%, clamped; in-air clamps to 0%.
- Average 16 samples per read to tame ADC jitter. Plausibility band 800–3200; outside it the probe is treated as disconnected.
- Use one sensor for the 5–6" pot, placed on the far side from the water inflow.
- Watering is closed-loop on moisture (drive soil to a setpoint, then stop), not a fixed dose.

**Rationale.** The 3 conditions already give a full dry→wet scale; "healthy" is a chosen setpoint on that line (the DL-020 trigger/stop pair), not a fourth measurement. The wet anchor was non-waterlogged, so it is conservative: readings wetter than it simply clamp to 100% rather than driving the soil to saturation — the safe direction. One sensor suffices in a small uniform pot; a second doubles failure surface for little gain. Placing it away from the inflow means it reads "wet enough" only once moisture has migrated across the root zone, not when a slug of water hits the inlet — avoiding early pump cutoff. Closed-loop control means a growing plant is handled automatically: it dries out faster and is watered more often, but each watering returns the soil to the same setpoint, so growth changes frequency, not dose.

**Alternatives considered.** Raw-only reporting (less readable — rejected; we publish both). Provisional percent flagged as untrustworthy (unnecessary — the dry/wet anchors are solid measurements). A second probe (deferred to larger/multi-plant setups). Open-loop timed dosing (rejected: fixed volume drifts wrong as the plant grows).

**What this entry does not commit to.** Exact trigger/stop values and pulse-then-wait pump behaviour — finalized on the live plant with the state machine (water wicks to the probe with a lag, so pumping must pulse-and-settle, not run to threshold in one shot). A future vision layer may nudge the setpoint, never drive the pump directly.

**Files.** Firmware: `soil.{h,cpp}`, `config.h`, `net_mqtt.{h,cpp}`, `main.cpp`. Hub: `listener.py`, `dashboard.py`.

---

<a id="dl-043"></a>
### DL-043 — Float switch and leak sensor: report-only, faults deferred to FSM

**Date:** 2026-06-06 · **Status:** Active. Both integrated, publishing, and physically verified.

**Context.** The reservoir float (GPIO27, digital) and leak sensor (GPIO39, analog) complete the sensing layer. Unlike the continuous sensors, both are effectively stateful/safety signals, which raised where they belong in the DL-035 schema.

**Decision.**
- Both are treated as plain sensors: they publish on `plant/sensors/float` and `plant/sensors/leak` and project into `sensor_readings` (booleans stored as 0/1), same path as every other sensor.
- The float reports `reservoir_empty`; polarity is set by `FLOAT_EMPTY_WHEN_CLOSED` in config.h (INPUT_PULLUP: HIGH=open, LOW=closed), made flippable because the empty↔closed mapping depends on the physical switch. Verified by lift/drop test.
- The leak module averages 16 samples and applies `LEAK_THRESHOLD` (200, DL-026); higher raw = wetter. It publishes both `leak_raw` and a `leak_detected` flag so the threshold stays authoritative in firmware config.
- Fault interpretation (leak detected → critical; reservoir empty → block the pump) is NOT done here. The sensor layer only reports; the state machine consumes these and is what will write `fault_events`.

**Rationale.** Keeping the sensor layer pure (report, don't decide) matches the separation held throughout Phase 2 and keeps each module independently testable. `fault_events` is reserved and rightly written by the decision-making layer, not by a sensor. Publishing `leak_detected` from firmware keeps the single source of truth for the threshold in config.h rather than duplicating it on the dashboard. The flippable float polarity avoids hard-coding an assumption that depends on physical mounting.

**Alternatives considered.** Routing leak to `fault_events` / float to `system_status` directly from the listener (rejected: that puts decision logic in the ingest layer before the FSM exists, and a raw "wet pad" reading is not yet a declared fault). Publishing only `leak_raw` and thresholding on the dashboard (rejected: splits the threshold's source of truth).

**Files.** Firmware: `float_switch.{h,cpp}`, `leak.{h,cpp}`, `config.h`, `net_mqtt.{h,cpp}`, `main.cpp`. Hub: `listener.py`, `dashboard.py`.

---

<a id="dl-044"></a>
### DL-044 — Third status LED for traffic-light state indication

**Date:** 2026-06-06 · **Status:** Active.

**Context.** The state machine (DL-045) drives the on-board LEDs as a glanceable state indicator. Two LEDs (validated in Phase 1 sketches 07-feedback-io / 08-buttons) force encoding 6+ states via blink patterns, which is hard to read and conflates recoverable-blocked states with latched faults.

**Decision.** Add a third LED so the board shows three colors, wired and physically arranged as a vertical traffic light: red GPIO23 (top), yellow GPIO19 (middle), green GPIO18 (bottom). Map the three semantic tiers to color: green = normal / watering, yellow = blocked-but-recoverable (reservoir empty, daily limit), red = fault-latched (leak, emergency stop). This mirrors the dashboard's color tiers.

**Rationale.** Colour-per-tier reads at a glance without decoding blink patterns, and matches the dashboard so board and screen tell the same story. GPIO23 is the newly-wired pin (red moved there, yellow took red's old GPIO19) because GPIO23 has no strapping role; GPIO5 was rejected as a strapping pin that pulses at reset (LED flicker on boot). GPIO16/17 are unavailable on the WROVER (internal PSRAM). Physical top-to-bottom red/yellow/green arrangement makes the traffic-light metaphor literal.

**Alternatives considered.** Two LEDs + blink patterns (rejected: ambiguous, especially blocked-vs-fault on one red LED). OLED alone for state (it is additive detail; a glanceable colour signal is still wanted). GPIO5 (rejected: strapping-pin boot flicker).

**Files.** `config.h` (pins). The FSM (DL-045) drives it.

---

<a id="dl-045"></a>
### DL-045 — Traffic-light status LEDs bench-validated

**Date:** 2026-06-06 · **Status:** Active.

**Context.** DL-044 added a third LED and rewired the set (green GPIO18, yellow GPIO19, red GPIO23) into a vertical traffic light. The rewire — a new GPIO23 and a red↔yellow move — was unproven, and the state machine (DL-046) will depend on these pins driving the right LEDs.

**Decision.** Validated with a dedicated bench sketch (`firmware/test-sketches/12-traffic-light-leds`) before integration: each LED lit alone in physical top-to-bottom order (red, yellow, green) with serial labels, then all three blinked together. Confirmed each GPIO drives its intended LED in the correct position, no shorts/swaps, and GPIO23 (the new wire) works.

**Rationale.** Consistent with the project's phase-gated discipline — bench-validate each subsystem in isolation before integrating, so a wiring fault is caught on its own rather than masked as a logic bug inside the FSM. The sketch is committed (not throwaway) alongside the other Phase 1 component tests.

**Result.** Pass — all three LEDs correct in position and individually addressable.

**Files.** `firmware/test-sketches/12-traffic-light-leds/{platformio.ini, src/main.cpp}`.

---

<a id="dl-046"></a>
### DL-046 — Watering state machine

**Date:** 2026-06-06 · **Status:** Active. Built, flashed, and bench-validated with the pump stubbed.

**Context.** With all six sensors reporting (DL-042/043) and the LEDs validated (DL-045), the system needed the decision layer that turns telemetry into autonomous watering, safely and unattended.

**Decision.** A seven-state machine in `fsm.{h,cpp}`, ticked every loop with the latest soil/float/leak readings:
- States: monitoring, watering, manual, reservoir_empty, daily_limit, leak_fault, stopped.
- Safety-first evaluation order every tick, overriding all states: a debounced leak (must persist `LEAK_DEBOUNCE_MS` = 3s) → leak_fault; STOP button → stopped. Both latch and clear only on ACK (leak_fault also requires the leak to have cleared).
- Recoverable gating below safety: reservoir empty → blocked (auto-clears on refill); daily pump-time cap reached → blocked (auto-clears on window reset).
- Core loop: soil dries past `SOIL_THRESHOLD_TRIGGER` → watering; pump runs in pulses (`WATER_PULSE_MS` on, `WATER_SETTLE_MS` settle) so water wicks to the probe between reads (no overshoot) until soil re-wets to `SOIL_THRESHOLD_STOP` → monitoring. MANUAL button forces a watering cycle by hand, subject to the same gates.
- LEDs (DL-044/045) mirror state: green steady = monitoring, green blink = watering/manual, yellow steady = reservoir_empty, yellow blink = daily_limit, red steady = leak_fault, red blink = stopped.
- Buttons debounced (`BUTTON_DEBOUNCE_MS`, active-LOW edge): STOP GPIO32, ACK GPIO33, MANUAL GPIO26.
- State published retained to `plant/state/wrover` on change and every 30s: `{state, pump, daily_pump_ms}`.

**Pump deliberately stubbed.** `pump.{h,cpp}` logs `[PUMP] ON/OFF (stub)` and tracks state for daily accounting but does NOT drive GPIO25. The pump is the only actuator that can harm the plant, so it stays stubbed until the logic is validated and the pump is calibrated. "Pump lands last."

**Daily cap as run-time, not volume.** Implemented as cumulative pump-on milliseconds over a rolling 24h window (`MAX_DAILY_PUMP_MS`, placeholder 60s), not a calendar-midnight mL budget. Rationale: the failsafe (a single fault can't drain the reservoir into the pot) works immediately without pump calibration or NTP. Relabel to mL after calibration; true-midnight reset via NTP is a deferred refinement.

**Validation (pump stubbed, no water moved).** Confirmed on the bench: monitoring (green steady); auto-watering on a dry soil reading with the pulse cycle visible in serial; manual via button; reservoir_empty on float-down and auto-recovery on refill; leak_fault after sustained wet pad (and a sub-3s splash correctly ignored); stopped via STOP; ACK clearing both faults to monitoring. daily_limit not exercised live (structurally identical to the other gated states; needs a temporary low cap to trigger).

**Alternatives considered.** Build with the real pump from the start (rejected: unsafe before calibration and logic validation). NTP + mL budget now (rejected: more moving parts before the logic was proven). Safety as ordinary states rather than a first-evaluated override (rejected: a leak mid-watering must cut the pump immediately, not wait for a transition).

**Files.** `fsm.{h,cpp}`, `pump.{h,cpp}`, `config.h`, `net_mqtt.{h,cpp}`, `main.cpp`.

---

<a id="dl-047"></a>
### DL-047 — Buzzer alarm and OLED status display

**Date:** 2026-06-06 · **Status:** Active. Both bench-validated.

**Context.** The FSM (DL-046) drove the LEDs and stubbed pump; the buzzer (GPIO4) and OLED (SSD1306, I2C 0x3C) were the remaining feedback devices from Phase 1 (sketches 09-buzzer, 06-oled) still to integrate.

**Decision.**
- Buzzer (`buzzer.{h,cpp}`): active buzzer driven HIGH = sound (no PWM). Sounds an intermittent beep (`BUZZER_ON_MS`/`BUZZER_OFF_MS`) only in `leak_fault`; silent in every other state. The FSM calls `buzzer_update(state == leak_fault, now)` each tick; non-blocking.
- OLED (`oled.{h,cpp}`): a status screen refreshed every `OLED_REFRESH_MS` (500 ms) showing the current state plus pump, soil (% and raw), reservoir, leak, air (temp/humidity), and the daily pump-time budget. Composed in main.cpp from the FSM state getter and the sensor caches. Non-fatal: if the display is absent, `oled_render` is a no-op.
- FSM exposes `fsm_state_name()` and `fsm_daily_pump_ms()` so the display layer reads state without owning it.

**Rationale.** The buzzer is reserved for the one autonomous emergency that demands immediate attention while unattended (a leak / water pooling); STOP is user-initiated and reservoir-empty / daily-limit are non-urgent (yellow LED suffices), so sounding for them would only train the alarm to be ignored. Keeping OLED composition in main (which holds all caches) preserves separation: the FSM decides state, the display only renders it. Non-fatal init matches the project's non-blocking ethos — a missing screen never stalls control.

**Validation.** Buzzer beeps on a sustained wet leak pad and silences on ACK. OLED shows the live status screen and tracks state transitions and readings.

**Files.** `buzzer.{h,cpp}`, `oled.{h,cpp}`, `fsm.{h,cpp}`, `main.cpp`, `config.h`, `platformio.ini`.

---

<a id="dl-048"></a>
### DL-048 — Pump flow calibration and daily water cap

**Date:** 2026-06-06 · **Status:** Active.

**Context.** The daily cap (DL-046) was implemented as a pump-run-time budget with a placeholder value, pending the pump's measured flow rate. Enabling the real pump also needs a known mL/s.

**Method.** Standalone bench sketch (`firmware/test-sketches/13-pump-calibration`) drove the real pump (GPIO25) for fixed timed dispenses into a measuring cup. Tube primed to a steady stream first (peristaltic pump holds prime; occasional air bubbles from the inline barb connectors were judged negligible at this volume). Cumulative readings taken at eye level on the oz scale to avoid parallax (oz lines were the clearest graduations):

| Elapsed | Reading | Volume | Rate |
|--------|---------|--------|------|
| 90 s   | 3.0 oz  | 88.7 mL  | 0.99 mL/s |
| 180 s  | 6.0 oz  | 177.4 mL | 0.99 mL/s |
| 270 s  | 9.5 oz  | 280.9 mL | 1.04 mL/s |

**Result.** Flow rate ≈ **1.0 mL/s**. The first two points are identical; the third is ~5% higher, within reading tolerance on a coarse cup (the half-oz increment is at the resolution limit) — treated as measurement noise, not a real change in rate. Overall slope (280.9 mL / 270 s) = 1.04 mL/s; tight two-point figure = 0.99 mL/s. Recorded as 1.0 mL/s.

**Decision.** Daily water cap set to **200 mL/day** → `MAX_DAILY_PUMP_MS = 200000` (200 s at 1.0 mL/s). Conservative for the first autonomous run given a small basil plant in ~4" soil depth; chosen below a "drain the reservoir into the pot" catastrophe and above one normal watering. Revisit upward (250–300 mL) if the soil dries too quickly in testing. The cap remains stored as run-time; mL is the human-facing interpretation via the calibrated rate.

**Alternatives considered.** 250 mL (leaning candidate, deferred as slightly less conservative for a first run); 300/500 mL (more headroom, rejected as too loose for an unattended debut). True mL accounting + NTP midnight reset still deferred (DL-046).

**Files.** `firmware/test-sketches/13-pump-calibration/{platformio.ini, src/main.cpp}`, `config.h` (cap value).

---

<a id="dl-049"></a>
### DL-049 — Real pump enabled and watering dosing tuned

**Date:** 2026-06-06 · **Status:** Active. Full autonomous watering loop validated on the bench.

**Context.** With the FSM and all safety guards validated (DL-046/047) and the pump calibrated at ~1.0 mL/s (DL-048), the final step was to flip the pump from stub to real GPIO25 output and set the watering dosing.

**Decision.**
- `pump.cpp` now drives the MOSFET gate: `pump_begin()` sets GPIO25 OUTPUT and LOW; `pump_on/off` `digitalWrite` HIGH/LOW. The stub (log-only, GPIO untouched) is retired now that the logic is proven and the pump is calibrated — the deliberate "pump lands last" sequencing.
- Dosing: `WATER_PULSE_MS` = 5000 (~5 mL per pulse at 1.0 mL/s), `WATER_SETTLE_MS` = 10000 (10 s). One pulse-cycle ≈ 15 s delivering ~5 mL; the pump runs only a third of each cycle.

**Rationale.** Small pulses with a generous settle let water spread and wick to the off-center soil probe before re-reading, avoiding overshoot (the closed-loop lag the user flagged). 5 mL (vs an initial 2 mL) reaches a real watering volume in fewer cycles while staying small enough that the 200 mL daily cap (DL-048) backstops any over-pumping. Settle raised from 5 s to 10 s for the same spreading-lag reason. Underwatering a cycle is recoverable; overwatering a small basil in ~4" soil is not — so the conservative dosing is intentional for the first autonomous run.

**Validation (real water under FSM control).** First live run with the outlet into a cup: soil probe lifted to air (dry) → FSM `monitoring -> watering`, pump physically pulsed ON ~5 s / OFF ~10 s; probe returned to moist → pump stopped cleanly and FSM returned to monitoring. LEDs tracked state throughout. Pump stops reliably on OFF; STOP button available as kill switch.

**Follow-ups (tune on the real plant).** Confirm `WATER_SETTLE_MS` is long enough for water to reach the probe in real soil (lengthen if it over-pumps); verify probe-to-outlet spacing; position outlet over the root zone, not at the stem. Raise daily cap toward 250–300 mL only if the soil dries too fast.

**Files.** `pump.cpp`, `config.h`.

---

<a id="dl-050"></a>
### DL-050 — Float switch mounting: rigid rod through the lid

**Date:** 2026-06-06 · **Status:** Active. Validated in the final mounted configuration.

**Context.** The reservoir float needs to read a repeatable water level. First attempt rested the float hanging from its wire (`docs/images/float-mount-1.jpg`), which sat slanted and swung freely — a float switch only reads a consistent threshold when its stem is held in a fixed orientation, so a free-hanging/tilting float gives unreliable, slosh-sensitive readings and an unpredictable empty point.

**Decision.** Route the float's wires through a rigid carbon-fiber rod and bond the rod to the float's threaded collar as one stiff unit (`float-mount-2.jpg`, `float-mount-3.jpg`), then anchor the rod through a hole drilled in the reservoir lid, fixing the assembly vertical and centered (`float-mount-4.jpg`). Bonded with waterproof cyanoacrylate gel (aquarium-rated). The mounting depth sets the empty threshold; depth chosen to trip EMPTY with usable reserve still in the jar (avoids the pump drawing air before it trips).

**Rationale.** A rigid lid-anchored rod holds the float vertical with repeatable travel, and the only reservoir penetration is in the lid — above the waterline, so no leak risk (unlike a side-wall bulkhead, which adds a sealed hole below the waterline). Center mounting keeps the float off the wall and clear of the pump inlet. Glue applied only to the stem/collar and rod-to-lid joints, never the sliding float, so the float remains free (a glued/stuck float would mask an empty reservoir — the dangerous failure direction noted below).

**Validation.** In the final mount with a full reservoir the switch reads `reservoir ok`; drawing the level down past the float flips it to `EMPTY`. Both directions confirmed on the dashboard. `FLOAT_EMPTY_WHEN_CLOSED` polarity correct as-is.

**Known limitation (for the observability backlog).** A disconnected float reads "not empty" (→ ok) due to INPUT_PULLUP, which would let the FSM run the pump dry — the highest-priority item for the planned device-presence/fault-detection work. (The soil probe has the opposite, safer failure: disconnection reads dry, only causing a benign watering attempt.)

**Alternatives considered.** Free-hang from wire (rejected: slanted, slosh-sensitive, unreliable level). Side-wall bulkhead mount (rejected: sealed hole below the waterline = leak risk). Pendulum from lid (rejected: swings, triggers on tilt/slosh).

**Files / images.** `docs/images/float-mount-1.jpg` (failed first mount), `-2.jpg` (float held), `-3.jpg` (rod+float assembly), `-4.jpg` (final, in reservoir).

---

<a id="dl-051"></a>
### DL-051 — Pot tube routing: drilled inlet for gentle delivery

**Date:** 2026-06-06 · **Status:** Active.

**Context.** The outlet tube initially hung over the pot rim, dropping water from height. Water falling from height channels and splashes rather than soaking evenly, and an unsecured tube shifts, changing the outlet-to-probe geometry that watering behavior depends on.

**Decision.** Drilled a hole in the pot side to route the outlet tube so it delivers water at/near the soil surface, calmly, into the root zone — positioned offset from the soil probe (not at the stem, not adjacent to the probe), above the soil/drainage line so it is a tube inlet, not a drainage leak.

**Rationale.** Surface-level delivery soaks the root zone evenly instead of channeling straight down and out the drainage holes. A fixed routing keeps the outlet-to-probe spacing stable, so the closed-loop watering behavior stays consistent run to run. Offset-from-probe placement (per the soil-probe siting decision) lets the probe read general spread, not the inflow point.

**Follow-up.** Confirm on the real plant that water reaches the probe within the settle window (`WATER_SETTLE_MS`, 10 s) and that spacing neither stops watering too early nor overwaters; tune from observed behavior.

**Files.** Physical change to the pot; no firmware change.

---

<a id="dl-052"></a>
### DL-052 — Dashboard watering-system state banner

**Date:** 2026-06-06 · **Status:** Active. Validated end-to-end.

**Context.** The FSM publishes its state to `plant/state/wrover` (retained), but nothing consumed it — the dashboard showed sensors and grow-light status, not what the watering system was actually doing.

**Decision.** Route `plant/state/<device>` in the listener into `system_status` (with `metric` set to `fsm_state` / `pump`, so it does not collide with the `metric IS NULL` online-status query), and render a prominent banner at the top of the dashboard showing the current FSM state, color-coded to match the LED tiers (green = monitoring/watering, amber = reservoir_empty/daily_limit, red = leak_fault/stopped), plus pump status, daily watering seconds used, and last-updated time.

**Rationale.** Reuses existing plumbing (`system_status` table, the status palette already in the dashboard) rather than adding a new table. Storing state with a metric keeps it cleanly separable from device-online rows. The banner makes the system's behavior legible at a glance and remotely (via Tailscale), and mirrors the physical board so screen and bench tell the same story.

**Validation.** Pressing STOP on the board flipped the banner to red "Emergency stop" within a refresh, confirming the full path FSM → MQTT → listener → SQLite → dashboard.

**Files.** `hub/04-listener/listener.py`, `hub/06-dashboard/dashboard.py`.

---

<a id="dl-053"></a>
### DL-053 — Watering-effectiveness watchdog

**Date:** 2026-06-06 · **Status:** Active. Bench-validated; real-plant watering confirmed (soil rose to ~66%).

**Context.** A disconnected or failed float reads "not empty" (INPUT_PULLUP → HIGH), so the reservoir interlock can't catch a dead float — the system could pump a dry reservoir. Hub-side presence detection can't catch this either: the WROVER stays online and keeps reporting `reservoir ok`. The gap (flagged in DL-050) needs a firmware cross-check.

**Decision.** Add a watering-effectiveness watchdog to the FSM. While watering, each completed pulse+settle cycle checks whether the soil actually responded: if `soil_raw` dropped by at least `WATER_RESPONSE_MARGIN` (30 counts) the streak resets; otherwise a no-progress counter increments. After `WATER_WATCHDOG_PULSES` (8) consecutive no-progress cycles the FSM concludes it is acting without effect and trips to a new latched state, `watering_fault` (pump off, clears only on ACK). LED: steady red + yellow together (a unique combo). The dashboard banner shows it in red.

**Rationale.** A closed-loop system should detect when it acts but the world doesn't respond — this catches a dead/disconnected float, an empty reservoir despite a stuck float, a dead pump, a clog, or a kinked tube, all with one feedback check rather than per-sensor diagnostics. Dedicated latched fault (not a silent auto-clearing block) because an ineffective-watering condition means the plant isn't getting water and warrants a human looking at it. Thresholds chosen conservatively (8 cycles ≈ 2 min, ~40 mL, well under the 200 mL cap; small response margin) so a slow-but-working real-soil response does not false-trip; to be tuned with real-plant behavior.

**Validation.** Bench: probe held in air (watering triggered, no soil response) tripped to `watering_fault` after ~8 cycles — red+yellow LEDs, pump off, banner red; ACK cleared it. Progress (wetting the probe mid-watering) correctly reset the streak and prevented the fault. Real plant: a normal watering cycle raised soil moisture to ~66%, confirming water reaches the off-center probe within the settle window (10 s settle adequate for this soil/spacing).

**Known limitation.** Catches dead-float only indirectly (via ineffective watering), not at the moment of disconnection; device-silence (WROVER/Shelly offline) is a separate hub-side concern (next slice).

**Alternatives considered.** Auto-clearing block like daily_limit (rejected: ineffective watering needs human attention, not silent retry). Hub-side presence detection for the float (rejected: can't see a dead float while the WROVER is online). Hardware float-disconnect detection (deferred: more wiring; the feedback watchdog covers more failure modes).

**Files.** `firmware/integrated/src/fsm.cpp`, `config.h`, `hub/06-dashboard/dashboard.py`.

---

<a id="dl-054"></a>
### DL-054 — Grow-light daily photoperiod via Shelly scheduler

**Date:** 2026-06-06 · **Status:** Active.

**Context.** The grow light needed to come on by itself daily so the plant gets consistent light without manual intervention. This is the "schedule-based" path from the deferred lighting decision (vs. closed-loop control on the BH1750), which suits basil — it needs a steady photoperiod, not reactive dimming.

**Decision.** Run a fixed daily photoperiod, 07:00 ON → 19:00 OFF (12h light / 12h dark), on the **Shelly's own built-in scheduler**, configured via its RPC API (`Schedule.Create`) from a committed script (`hub/08-grow-light/set-schedule.sh`). The BH1750 stays report-only, verifying actual light.

**Rationale.** Device-side scheduling is autonomous and robust — the light keeps its schedule even if the Pi, WROVER, or network is down, with no runtime dependency. Setting it via RPC (not hand-tapping the app) keeps the schedule reproducible and documented in the repo. A full photoperiod (on AND off) gives the required dark period; 12h is a sensible, slightly conservative window given supplementary ambient lab light. Lighting stays architecturally separate from the watering FSM (independent concerns).

**Gotcha recorded.** Shelly schedules fire in the device's local time, so the Shelly's timezone/location must be set (America/Chicago) or "07:00" fires at the wrong hour.

**Alternatives considered.** Pi-side cron sending commands (rejected: adds a Pi/network dependency for a fixed schedule, no benefit). Closed-loop control off BH1750 lux (deferred: needs lux→meaning calibration; unnecessary for a fixed photoperiod). On-time only without an off-time (rejected: no dark period; stresses the plant).

**Device note.** Unit confirmed via `Shelly.GetDeviceInfo` as a Shelly Plug US **Gen4** (model S4PL-00116US, app PlugUSG4), not the "Plus Plug US" named in earlier entries — the BOM should be corrected. RPC auth is disabled (`auth_en:false`), so the schedule calls need no credentials. IP `10.6.17.32` (DHCP; verify before re-running the script).

**Files.** `hub/08-grow-light/set-schedule.sh`.

---

<a id="dl-055"></a>
### DL-055 — Device-silence detection (WROVER presence)

**Date:** 2026-06-06 · **Status:** Active. Validated by power-cutting the WROVER.

**Context.** The WROVER published a retained presence message on `plant/status/wrover` (`{"online":true,...}`) and registered a Last-Will (`{"online":false}`) the broker emits on ungraceful disconnect — but the listener never routed that topic, so a WROVER power loss left no recorded trace and the dashboard kept showing its last state as if live (the same blind spot first noticed with the Shelly power-cut).

**Decision.** Route `plant/status/<device>` in the listener into `system_status` (online/offline, metric NULL, so `device_online_status()` reads it). Make the dashboard offline-aware: when the WROVER is offline, the state banner turns gray ("WROVER offline", showing the last known state and its timestamp rather than a stale live-looking state), and the header status pill shows "WROVER offline" in red. Also folds in the `watering_fault` banner mapping that was missed in DL-053.

**Rationale.** The MQTT Last-Will is the native, push-based mechanism for presence — the broker reports the device gone on power loss/crash with no polling, which is exactly the failure mode that matters. Surfacing it on the banner+pill stops the dashboard from implying the system is live when its brain is down. The Shelly's equivalent presence (`plant/grow-light/online`) was already ingested, so this brings the WROVER to parity.

**Validation.** Unplugged the WROVER: within the broker keepalive window the will fired, the listener logged `offline`, and on refresh the banner went gray "WROVER offline" with the last state/time, and the pill went red. Repowering republished `online:true` and the dashboard recovered to live state.

**Known gap / next.** While the WROVER is offline, the environment cards still show the last-published readings without a staleness indicator (they look live). Next tidy-up: dim/flag WROVER-sourced cards as stale when offline. A timeout-watchdog backstop (for a hung-but-still-connected device, which the will misses) is a deferred enhancement.

**Files.** `hub/04-listener/listener.py`, `hub/06-dashboard/dashboard.py`.

---

<a id="dl-056"></a>
### DL-056 — Stale environment cards when the WROVER is offline

**Date:** 2026-06-08 · **Status:** Active. Validated by power-cutting the WROVER.

**Context.** DL-055 made the dashboard banner and header pill offline-aware, but the seven WROVER-sourced environment cards (temp, humidity, pressure, light, soil, reservoir, leak) still showed their last-published readings in normal live styling. A glance could read "offline… but soil 40%, all green," when those values are frozen and possibly hours old — the same staleness blind spot the banner had, left on the cards.

**Decision.** When `device_online_status("wrover") == "offline"`, render those seven cards greyed (status `unknown`) and append "· stale (as of <local time>)" to each card's meta, where the time is the most recent reading timestamp among the WROVER sensors. Cards return to live styling automatically when the WROVER republishes.

**Rationale.** Completes the honesty story started in DL-052/DL-055: every WROVER-derived surface (banner, pill, cards) now visibly stops implying live data when the device is down. Showing the "as of" time lets the operator judge how stale the frozen readings are, and doubles as an indicator of roughly when the device went quiet. Reuses the existing presence signal and `render_card`'s `unknown` styling, so no new mechanism or schema.

**Validation.** Unplugged the WROVER: banner went gray (DL-055) and the seven environment cards greyed with "stale (as of HH:MM)" showing the last-reading time; repowering restored live cards on the next refresh.

**Files.** `hub/06-dashboard/dashboard.py`.

---

<a id="dl-057"></a>
### DL-057 — Soil-moisture trend with watering-episode overlay

**Date:** 2026-06-08 · **Status:** Active. Validated on live data.

**Context.** The dashboard showed soil only as a current card; there was no history and no visible record of when the system watered. The goal was a view that tells the autonomy story — sense dryness, act, verify the soil rises.

**Decision.** Added a "Soil moisture & watering" section (24h / 7-day tabs): a soil-moisture trend line with watering episodes shaded as blue bands, plus a table of episodes (time, duration, approximate mL, Auto/Manual). Dashboard-only — episodes are derived from the existing `fsm_state` history (a span in `watering`/`manual`), with volume estimated from the `daily_pump_ms` delta across each span (~1.0 mL/s, DL-048). No firmware or schema change.

**Rationale.** Reuses data already captured rather than adding new telemetry. Episodes (not individual 5 s pulses) are the meaningful unit and are reliably reconstructable from publish-on-change state transitions. The volume is labeled approximate because it samples cumulative pump time at state transitions, not a flow meter.

**Validation.** Live dashboard shows the soil trend with watering bands and a matching episode table over the 7-day window.

**Files.** `hub/06-dashboard/dashboard.py`.

---

<a id="dl-058"></a>
### DL-058 — mL daily cap and NTP calendar-midnight reset

**Date:** 2026-06-09 · **Status:** Active. NTP sync confirmed on hardware.

**Context.** DL-046/048 left two placeholders: the daily water cap was expressed as pump run-time (`MAX_DAILY_PUMP_MS`) rather than mL, and "daily" was a rolling 24h window measured from boot, so the reset drifted relative to the calendar.

**Decision.** Express the cap in mL: `PUMP_ML_PER_SEC = 1.0` (DL-048) and `MAX_DAILY_PUMP_ML = 200.0`; the cap check converts accumulated pump-on time to mL. Reset the daily counter at **local calendar midnight** using NTP time (`configTzTime` with POSIX TZ `CST6CDT,M3.2.0,M11.1.0` for America/Chicago, DST-aware), detected by a change in `tm_yday`. The rolling-24h window is retained as a **fallback** used only until NTP has synced, so the cap still works if WiFi/NTP is unavailable (consistent with the non-fatal-connectivity principle, DL-040). Accumulation stays internal in milliseconds (integer, precise); the published value and DB schema are unchanged, so the dashboard's history and episode derivation are unaffected. Dashboard banner relabeled from seconds to mL.

**Rationale.** mL is the meaningful unit and the calibration-aware constant keeps the cap correct if the flow rate is ever re-measured. Calendar-midnight reset matches how a daily budget is naturally understood. Keeping the wire format in ms avoids a mixed-unit history in the database for no numeric gain at 1.0 mL/s.

**Validation.** On hardware, NTP synced a few seconds after WiFi connect: `[FSM] NTP time synced: 2026-06-09 13:29 local` — correct Central time (CDT), confirming the TZ/DST string. The mL cap check is numerically identical to the prior ms check at 1.0 mL/s. Midnight reset relies on the verified time and the `tm_yday` comparison; not exercised live (awaits midnight).

**Files.** `firmware/integrated/src/config.h`, `fsm.cpp`, `net_wifi.cpp`, `main.cpp`, `hub/06-dashboard/dashboard.py`.

---

<a id="dl-059"></a>
### DL-059 — Device-presence timeout watchdog (backstop)

**Date:** 2026-06-10 · **Status:** Active. Logic unit-tested; integration smoke-tested in production.

**Context.** DL-055 added presence detection via the MQTT Last-Will, which the broker emits when a device loses power or its TCP link. It does not cover a device that hangs while *still connected* — the socket stays up, the will never fires, but the device stops publishing. That gap was explicitly deferred in DL-055.

**Decision.** Add a hub-side timeout watchdog in the listener. It tracks the last-seen time per watched device (`wrover`), and if one goes silent past `WATCHDOG_TIMEOUT_S` (90 s) it writes an `offline` row to `system_status` (metric NULL), and an `online` row when the device resumes publishing. The check runs every `WATCHDOG_INTERVAL_S` (30 s) on a time-driven tick — not message-driven, since total silence would otherwise stop the trigger from ever running. The listener moved from `client.loop_forever()` to `client.loop_start()` (background network thread, retaining auto-reconnect) plus a watchdog loop on the main thread; the watchdog uses its own SQLite connection, and both connections set `busy_timeout` to tolerate the second writer.

**Rationale.** Reuses the existing presence representation (`system_status`, metric NULL), so the dashboard's offline-awareness (DL-055/056) reacts to watchdog-driven offline exactly as it does to the Last-Will — one unified presence story. Scoped to the WROVER (a known, regular publish cadence) to avoid false positives on the more sporadic grow-light, which keeps its Last-Will coverage. The timeout (90 s) sits well beyond the WROVER's normal publish gaps to avoid false offlines.

**Validation.** Watchdog logic unit-tested: marks offline only after real silence, idempotent (no duplicate rows), recovers on resume, never flags an unseen or fresh device. In production the restructured listener reconnected and ingested normally with no false watchdog warnings and a live dashboard. The genuine hung-but-connected case is hard to induce on demand; the timeout path can be exercised by cutting the WROVER's network while leaving it powered (silence trips the watchdog before the keepalive-driven will).

**Known limitation.** Watches only the WROVER; the grow-light relies on its Last-Will. Timeout is fixed, not adaptive.

**Files.** `hub/04-listener/listener.py`.

---

<a id="dl-060"></a>
### DL-060 — Reboot detection (Piece A of reboot alerting)

**Date:** 2026-06-10 · **Status:** Active. Validated on hardware (two consecutive resets detected).

**Context.** An unexpected WROVER reboot (brownout, flaky supply, pump-induced sag, firmware crash) is a symptom worth knowing about. It was inferable from an offline→online presence blip (DL-055), but never recorded as a first-class event. This is Piece A (detect + record + show); push notification is Piece B (next).

**Decision.** The WROVER already publishes `uptime_s` in its retained status message (on connect and each heartbeat). The listener tracks the last `uptime_s` per device; when it jumps backwards, the device rebooted, and a `metric='reboot'` row is written to `system_status` with `value` = the uptime reached before the reboot. The dashboard shows a caption under the state banner ("↻ Last WROVER reboot: <time> (was up <duration> before)"), escalating to an amber "check power stability" warning at ≥ 2 reboots in 24h (flapping indicator).

**Rationale.** Reuses an existing field (`uptime_s`) — no firmware change. Recorded in `system_status` rather than `fault_events` to avoid ACK semantics (a past reboot is informational, not an active fault); `metric='reboot'` keeps it clear of the presence rows (which filter `metric IS NULL`). `_last_uptime` is touched only from `on_message` (single background thread), so it needs no lock, unlike the watchdog state. First-seen and the retained status on listener restart establish a baseline without false-flagging.

**Validation.** Logic unit-tested (no false flag on first sight or monotonic growth; flags only on a backwards jump). On hardware, two consecutive EN/RST resets were both detected and logged (`uptime 1s < previous 68666s`, then `< previous 51s`), and the dashboard caption rendered correctly.

**Known limitation.** A reboot that occurs entirely while the listener is down is not detected (the listener re-baselines from the retained status); the presence path still records the offline/online transition. Watches the WROVER (only device publishing `uptime_s`).

**Files.** `hub/04-listener/listener.py`, `hub/06-dashboard/dashboard.py`.

---

<a id="dl-061"></a>
### DL-061 — Push notification alerting layer (ntfy)

**Date:** 2026-06-10 · **Status:** Active. All alert branches validated on hardware.

**Context.** The dashboard surfaces problems only when someone is looking at it. The original brief (a self-sufficient plant you can leave alone) wants out-of-band notification — knowing something is wrong without watching. This is Piece B of the alerting arc (Piece A, reboot detection, was DL-060); it also generalises beyond reboots to cover faults and presence.

**Decision.** A new `alerter.py` module imported by the listener, evaluated every ~30 s from the existing watchdog loop. It polls the state already recorded in `system_status` (FSM state, presence, reboot markers) and pushes via [ntfy](https://ntfy.sh) — Pi → ntfy.sh → APNs → phone, so delivery works anywhere the phone has internet, independent of the lab network. Alerting is deliberately selective: notify only on genuine, actionable problems — leak fault and watering fault (high priority), reservoir empty and prolonged offline (default), and flapping reboots ≥ 2/24h (high) — each with a recovery ("Resolved") ping, plus a low-priority daily heartbeat summary at 09:00 local. Normal operation, single reboots, and the daily-limit cap stay dashboard-only. Alerts are edge-triggered and debounced via in-memory state (one alert per transition, never repeated while a condition persists); the offline alert has a 5-min grace so reboots and blips don't page.

**Rationale.** Lives inside the listener (no second service) because the listener already records every transition and has a timer loop. Polling rather than per-message hooks is simpler, naturally debounced, and robust to the firmware re-publishing the same state every 30 s; since the alert-worthy faults latch, a 30 s poll never misses them and ~30 s latency is fine. `notify()` fires on a short-timeout daemon thread so a slow network call never stalls ingestion, and is a silent no-op when `NTFY_TOPIC` is unset, so the listener behaves identically whether or not alerting is configured. The topic is a shared secret, so it lives in the systemd `EnvironmentFile` (`/etc/plant-hub/credentials`), never in the public repo. Standard library only (`urllib`, `zoneinfo`) — no new dependency.

**Validation.** End-to-end on hardware: on restart with two reboots in 24h, the flapping-reboot alert and the daily heartbeat both reached the phone in ~30 s (`ntfy sent:` logged for each). A probe-in-air manual watering latched `watering_fault` and pushed the high-priority alert; clearing it delivered the "Resolved" recovery ping. Outbound HTTPS to ntfy.sh confirmed working through the campus network.

**Known limitations.** Public ntfy.sh is best-effort (fine for a personal project, not life-safety); the topic name is effectively a bearer secret (mitigated by length, self-hostable later). In-memory alert state resets on listener restart, so an ongoing fault may re-alert once on restart. Watches the WROVER; the grow-light relies on its own Last-Will.

**Files.** `hub/04-listener/alerter.py` (new), `hub/04-listener/listener.py`, `hub/05-listener-service/README.md`.

---

<a id="dl-062"></a>
### DL-062 — ESP32-CAM bring-up: root-causing the "dead boards," validating sketch 11

**Date:** 2026-06-11 · **Status:** Active. Validated on hardware (sketch 11 captures frames continuously).

**Context.** Two ESP32-CAM modules had previously failed to flash, which motivated moving the planned vision node to the Seeed XIAO ESP32-S3 Sense (DL-034). Two new ESP32-CAMs plus the XIAO are now in hand. Before committing to the XIAO arc, the original "dead board" failures were re-investigated to settle whether the fault was the boards, the rig, or the toolchain — a question that would otherwise resurface every time a CAM is picked up.

**Finding (root cause).** The failures were never the boards. Three independent issues were stacked. (1) **Toolchain:** Arduino ESP32 core 3.3.9 ships a `platform.txt` that references `tools/flasher.py`, but the file is missing from the released package, so *every* IDE upload failed identically regardless of hardware — a known upstream packaging bug, fixed by fetching `flasher.py` from master, or rolling the core back, or flashing from PlatformIO. (2) **Wiring:** the FTDI ground had been landed on the signal-side `GND/R` pin next to `U0R` rather than the power-return `GND` beside `5V`, leaving the bootloader under-referenced ("Failed to connect: no serial data received"). (3) **Power under load:** the stock CameraWebServer example boot-looped (silent reset at `entry 0x400805b4`, before `setup()` printed anything), consistent with a WiFi current spike browning out a marginal supply — the no-WiFi sketch 11 runs rock-solid on the identical rig.

**Decision.** The two original CAMs (and the two new ones) are confirmed functional and retained as **spares**. **DL-034 stands unchanged — the XIAO ESP32-S3 Sense remains the vision node.** This episode reinforces that choice rather than reopening it: the CAM's manual GPIO0/RST bootloader entry, external-FTDI power dependence, and brownout sensitivity are exactly the friction the XIAO's native USB-C (auto-reset, single-cable power) removes, and the XIAO's S3 + 8 MB PSRAM is where the deferred ML path would live.

**Validation.** Sketch 11 (`firmware/test-sketches/11-esp32-cam/`) was flashed via PlatformIO and ran continuously: PSRAM detected, `esp_camera_init` succeeded, and VGA (640x480) JPEG frames captured steadily at ~14 KB each with a monotonically incrementing counter and no resets. This moves sketch 11 from "committed but unexecuted" to **execution-validated**. Live WiFi streaming was not exercised on the campus network — client isolation (DL-028) blocks browser-to-device — so an actual image is best viewed over a hotspot/home WiFi or a serial-capture path.

**Known limitation.** Validation covers camera init and frame capture only — not WiFi streaming, image quality (focus/exposure/colour), or the eventual image transport. The committed `platformio.ini` hardcodes an FTDI port (`/dev/cu.usbserial-A5069RR4`) that differs per adapter; the live run used a `--upload-port` override rather than editing the committed file.

**Files.** `firmware/test-sketches/11-esp32-cam/` (validated; no code change).

---

<a id="dl-063"></a>
### DL-063 — Grow-light photoperiod verification (lux vs schedule)

**Date:** 2026-06-12 · **Status:** Active — deployed; threshold in a deliberate data-collection phase (see below).

**Context.** The grow-light photoperiod runs *on the Shelly plug itself* (DL-054) so it survives a hub or network outage — but that resilience means nothing on the hub was confirming the light actually came on. A bulb failure, a dropped Shelly, a lost schedule, or an unplugged cord would go silent until the plant suffered. This is the same commanded-vs-measured gap the watering watchdog (DL-053) and presence/reboot detection (DL-059/060) close, now applied to lighting: don't trust that the command worked — verify the physical effect with an independent sensor. (That independent-verification principle is the same discipline safety-critical autonomy, e.g. self-driving, relies on.)

**Decision.** A hub-side check in `alerter.py` (polled from the existing evaluate loop) compares measured BH1750 lux against the expected photoperiod state. During the ON window lux must stay above a threshold; during the OFF window, below it. A mismatch sustained past a grace window raises a default-priority alert ("Grow light may be off" / "still on"), with a recovery ping when it clears. Threshold **30 lux**, sustain **15 min**, all env-overridable.

**Rationale.** Lives hub-side, not in firmware, because lighting and watering are independent concerns (the firmware doesn't control the grow light — the Shelly does) and the lux already flows to the hub. Absolute-threshold (not transition) detection chosen because the measured separation is clean. The 15-min sustain absorbs the 07:00/19:00 switch moments and transient shadows; the check stands down when lux is stale (e.g. controller offline) so a missing reading can't masquerade as a dark light. Note: the light is a blurple (red/blue) panel, and lux is photopic, so it under-reads the spectrum — ~45 lux is ample for the plant and is used here only as an on/off detection signal, not as a plant-light (DLI) proxy.

**Threshold derivation & data-collection phase.** Thresholds were set from measured data, not generic references (DL-021 noted this environment's ambient is far below textbook indoor values). In the current geometry: room-lights-max ≈ 18 lux vs grow light ≈ 44–47 lux at the sensor → a 30 lux line splits the gap with ~12 lux margin each side. Because the rig's geometry drifts as people adjust the light, pot, and breadboard, the threshold is being **validated over several days of real-handling data** — watching that the min-ON and max-OFF populations stay cleanly separated. The value is env-overridable (`GROW_LUX_THRESHOLD` in the credentials file), so retuning needs no code change. **A follow-up DL will record the validated or retuned threshold and link back to this entry.**

**Validation.** Logic unit-tested (lit/dark, grace, recovery, stale stand-down). Deployed and active; the scheduled OFF transition was observed alerting silent (matches expectation).

**Files.** `hub/04-listener/alerter.py`.

---

<a id="dl-064"></a>
### DL-064 — External (manual) watering detection

**Date:** 2026-06-12 · **Status:** Active — built, validated, deployed.

**Context.** If someone waters the plant by hand (or any moisture rise occurs that the system didn't cause), the closed loop already *responds* correctly — soil reads wet, so no watering triggers — but nothing ever *reported* it. That's an observability gap: the system handled the event silently. Where the grow-light check (DL-063) verifies a commanded action produced its effect, this is the complement — detecting when the world changed *without* the system causing it. (Both are the kind of model-vs-reality divergence check that safety-critical autonomy, e.g. self-driving, depends on.)

**Decision.** A hub-side check in `alerter.py` flags a soil-moisture rise of ≥ `SOIL_RISE_THRESHOLD` (15%) over a `SOIL_LOOKBACK_MIN` (30 min) window, **provided the pump did not run** within the lookback plus a settling buffer. Default priority, single debounced alert ("Unexplained watering — did someone water the plant?"). All parameters env-overridable.

**Rationale.**
- *Suppression keyed to the direct pump signal.* The check asks whether the pump was actually commanded on (`metric='pump'`, `status='on'`), not whether the FSM was in a watering mode. The actuator's real dispensation is the truest "did the system add water," and this is correct in the edge case where the controller enters `watering` but aborts before the pump ever runs (e.g. reservoir empty) — that must *not* suppress a genuine external rise. This is the project's commanded-vs-measured principle applied to the suppressor.
- *Settling buffer (30 min beyond the lookback).* Soil keeps climbing for many minutes after the pump stops as water diffuses around the probe. Without a buffer, the system's own post-watering settling would look identical to an external pour and self-trigger. The buffer ensures detection only resumes once even the *baseline* endpoint is past the settling climb.
- *Conservative threshold.* The quiet-hour soil swing (probe noise floor) measured ~2–5%; 15% is ~3× that, so only real pours trigger and routine jitter never does. Endpoints are short averages, not single readings, to reject probe glitches.
- *Correct time windowing.* Timestamps are stored as UTC ISO (`...Z`); the queries normalise them (`julianday(replace(replace(ts,'T',' '),'Z',''))`) so the short windows compare reliably rather than via a brittle string compare.
- *Hub-side, not firmware.* The firmware already responds correctly to external watering; only detect-and-notify was missing, which is an observability concern.

**Threshold derivation.** Δ and the noise floor were read off the logged soil history via read-only hub queries (hourly min/avg/max/swing), separating the ~2–5% quiet-period noise from a real rise — the same instrument-then-tune method used for the lux threshold (DL-063).

**Validation.** Logic unit-tested: pump-run suppression, the 15% bar (a +12% rise stays silent), debounce, re-arm once the baseline catches up, and graceful no-data handling. Deployed and active; the next automatic watering serves as a live negative test (pump-run suppression should keep it silent).

**Files.** `hub/04-listener/alerter.py`.

---

<a id="dl-065"></a>
### DL-065 — Correct timestamp windowing in hub time-range queries

**Date:** 2026-06-12 · **Status:** Active — fixed, validated, deployed.

**Context.** Several hub queries filter "the last N hours/minutes" with `ts >= datetime('now','-N …')`. But the database stores UTC ISO timestamps with a `T` separator and `Z` suffix (`2026-06-12T13:45:00Z`, from `utc_now_iso`), while SQLite's `datetime('now', …)` returns a space-separated string with no `Z`. The comparison is therefore a *string* compare, and at the separator position `T` (0x54) sorts after a space (0x20) — so any row sharing the threshold's calendar date is included regardless of its actual time. A "last 24h" filter could pull in rows up to ~24h older than intended.

**Impact.** `_reboots_24h` (alerter) and `reboots_recent` (dashboard) could over-count reboots, making the flapping-reboot alert slightly eager and inflating the dashboard's reboot count near a date boundary. The chart/history queries (`soil_history`, `power_history`, `watering_episodes`) could show more than the requested window.

**Decision.** Normalise the stored timestamp before comparison and compare numerically: `julianday(replace(replace(ts,'T',' '),'Z','')) >= julianday('now', …)`. Applied to `_reboots_24h` and the four affected dashboard queries. (DL-064's soil queries already used this correct form; this brings the older queries in line.)

**Discovery.** Found while writing DL-064's soil-window queries — the correct pattern there made the naive pattern in the pre-existing reboot/chart queries obvious.

**Validation.** Demonstrated on an in-memory SQLite DB: a reboot 30h ago that shares the calendar date with the −24h threshold was counted by the old query (total 2) and correctly excluded by the fixed query (total 1). Both files compile; deployed and restarted.

**Files.** `hub/04-listener/alerter.py`, `hub/06-dashboard/dashboard.py`.

---

<a id="dl-066"></a>
### DL-066 — Stop listener crash-loop and dashboard DB-locks under concurrency

**Date:** 2026-06-14 · **Status:** Active — fixed, deployed, verified.

**Context.** After ~3 days unattended, the dashboard intermittently failed with `database is locked` and the listener was crash-restarting (PID churn, clustered `sqlite3.OperationalError` tracebacks, multiple restart-heartbeats in one day). Root cause: the hub database ran in rollback journal mode (`journal_mode=delete`); the listener's two writer connections plus the dashboard reader (which had no `busy_timeout`) contended for the database lock, and as the DB grew (159 MB / 301k rows) collisions became frequent. Critically, a `database is locked` error in `alerter.evaluate()` was uncaught by the watchdog loop (which caught only `KeyboardInterrupt`/`SystemExit`), so it crashed the whole process; systemd restarted it and it re-crashed — flapping the entire watchdog/alerting layer. Consequence: a real grow-light fault (the Shelly rebooting to ON past its 19:00 off) went unalerted because the alerter wasn't running.

**Decision.** Three changes:
1. **WAL journal mode** on the listener's main connection — readers and one writer coexist without blocking (the standard fix for multi-process SQLite). Persists at the database-file level.
2. **Dashboard connection** gains `busy_timeout=5000` and `query_only=ON` — a read-write handle is required for WAL shared memory, but writes stay forbidden.
3. **Hardened watchdog/alerter loop** — each iteration is wrapped in `try/except` so a transient DB error logs and continues instead of crashing the process. The alerting layer must survive transient faults; it is the safety net.

**Rationale.** WAL is the canonical fix for SQLite read/write concurrency across processes. The loop-hardening is defense-in-depth — losing the entire alerting layer to one transient exception is far worse than the exception itself, and this incident proved it (the grow-light alert that should have caught the stuck light never fired because the alerter had crashed).

**Validation.** After deploy: `PRAGMA journal_mode` returns `wal`, the `-wal` file is present, the listener runs on a single stable PID with no further lock tracebacks, and the dashboard loads cleanly.

**Related — Shelly.** The grow-light plug was rebooting (~35 min uptime) and booting to ON, overriding the schedule's 19:00 off (the schedule itself was intact). Set the switch `initial_state` to `restore_last` so a reboot keeps the scheduled state, and switched it off. Why the plug keeps rebooting (power/WiFi) is still to be investigated.

**Known follow-up.** The database is 159 MB / 301k rows and growing fast (per-minute Shelly power telemetry + 30s WROVER sensors). WAL fixes concurrency but not query cost or file growth; indexes on `sensor_readings(sensor, ts)` and `system_status(device, metric, ts)` plus a retention/downsampling policy are the next slice.

**Files.** `hub/04-listener/listener.py`, `hub/06-dashboard/dashboard.py`.

---

<a id="dl-067"></a>
### DL-067 — Make windowed queries sargable so they use the existing time indexes

**Date:** 2026-06-14 · **Status:** Active — fixed, validated, deployed.

**Context.** DL-066 stopped the listener crash-loop (WAL + loop hardening), but the *driver* of the lock contention was slow queries holding the database too long. Investigation showed the time indexes already exist (`idx_sensor_readings_sensor_ts` on `(sensor, ts)`, `idx_system_status_device_ts`, etc.) — they were simply not being used. DL-065's correctness fix wrapped the column in a function: `julianday(replace(replace(ts,'T',' '),'Z','')) >= julianday('now', ?)`. Wrapping `ts` in a function makes the predicate **non-sargable**, so SQLite used only the `(sensor=?)` prefix of the index and then scanned every row of that sensor, applying the function to each. For high-frequency sensors (moisture, lux) with months of data that is tens of thousands of rows per call; as the table grew to 301k rows these scans lengthened the lock windows that DL-066 was mopping up after.

**Decision.** Normalise the **threshold**, not the column: `ts >= strftime('%Y-%m-%dT%H:%M:%SZ', 'now', ?)`. Both sides are now identical fixed-width UTC-ISO strings, so the comparison is still correct (lexicographic order = chronological order for this format), and because it is a plain `ts >= <constant>`, the `(sensor, ts)` / `(device, ts)` indexes range-scan it. Applied to all seven windowed queries: three in `alerter.py` (`_reboots_24h`, `_pump_ran_recently`, `_soil_avg` ×2) and four in `dashboard.py` (power history, recent reboots, soil history, watering episodes).

**Rationale.** This keeps DL-065's correctness (the bug it fixed — same-calendar-date rows wrongly included — does not return) while restoring index usage. It supersedes DL-065's *implementation*: same result, far cheaper. General principle: compare timestamps sargably — build the cutoff in the stored format and compare the raw column, never wrap the column in a function inside a WHERE clause.

**Validation.** Live `EXPLAIN QUERY PLAN` on the Pi: the old form planned `USING INDEX idx_sensor_readings_sensor_ts (sensor=?)` (scan within sensor); the new form plans `(sensor=? AND ts>?)` (index range seek). In-memory test confirmed the new queries return identical results to the old form (recent-window avg and lagged-baseline avg both unchanged) while using the index range.

**Files.** `hub/04-listener/alerter.py`, `hub/06-dashboard/dashboard.py`.

---

<a id="dl-068"></a>
### DL-068 — Nightly database retention to bound table growth

**Date:** 2026-06-14 · **Status:** Active — built, validated, deployed.

**Context.** `plant.db` grows without bound: per-minute Shelly power telemetry, ~30s WROVER sensor pushes, and every MQTT message archived. It reached 159 MB / ~826k rows in 10 days (`mqtt_messages` 334k, `sensor_readings` 306k, `system_status` 186k; `actuator_events` 48, `fault_events` 0). DL-067 bounded query *speed* (indexed range scans are independent of table size), but not file or SD-card growth. The dashboard's longest view is 7 days, so no rollup/downsampling is needed — a rolling-window delete suffices.

**Decision.** A nightly `systemd` timer (`plant-retention.timer` → `plant-retention.service` → `retention.py`) prunes the high-frequency tables to per-table windows, since they differ sharply in value:
- `mqtt_messages` — 7 days (raw archive, never queried historically; the biggest table).
- `system_status` — 14 days (alerter/dashboard only look back 24h).
- `sensor_readings` — 30 days (dashboard needs 7; the dominant table — lower to 14 to ~halve steady-state size).
- `runs`, `actuator_events`, `fault_events` — kept forever (sparse, valuable).

All windows are env-overridable (`RETENTION_*_DAYS`) so they tune without a redeploy.

**Rationale / safety.** Deletes are **sargable** (`ts < <cutoff>`, cutoff built in the stored ISO format) so they ride the per-table `ts` index (EXPLAIN: covering-index range scan). They are **batched** with a commit per batch and a brief yield between batches, so a large first-run backlog can't hold a long write lock or balloon the WAL against the live listener — applying the concurrency lessons from DL-066/067. **WAL + busy_timeout** means it waits for the listener instead of erroring and never blocks its reads. The job ends with `wal_checkpoint(TRUNCATE)`. The DB file plateaus via free-page reuse (no VACUUM in steady state); VACUUM is left as an optional manual off-hours step.

**Validation.** In-memory test: prunes only out-of-window rows, in batches, keeping recent rows, with the delete riding `COVERING INDEX idx_sensor_readings_ts (ts<?)`. Live first run removed ~N `mqtt_messages` rows; `sensor_readings`/`system_status` were no-ops (still inside their windows); listener unaffected.

**Files.** `hub/10-maintenance/retention.py`, `hub/10-maintenance/plant-retention.{service,timer}`, `hub/10-maintenance/README.md`.

---

<a id="dl-069"></a>
### DL-069 — Grow-light lux threshold validated against logged data

**Date:** 2026-06-15 · **Status:** Active — confirmed, no change.

**Context.** DL-063 set the grow-light verification threshold at 30 lux with a 15-minute sustain, flagged as data-collection-phase pending validation. ~10 days of logged lux are now available.

**Decision.** Keep 30 lux. The by-local-hour profile shows clean separation: scheduled-OFF hours (deep night) sit at ~0 lux with an ambient ceiling near 15 (excluding the 19:00 transition and the DL-066 stuck-on incident, both legitimate light-on); scheduled-ON hours average ~40–47 lux when the light is on. 30 is ~2× above the off-ceiling and ~30% below the on-level, and the 15-minute sustain filters transients. The threshold stays env-overridable (`GROW_LUX_THRESHOLD`).

**Notes.** Low per-hour minimums during ON hours are real light-off episodes (Shelly rebooting/stuck during the incident window), i.e. the alert correctly catching the plug's unreliability, not threshold error. Mid-morning max spikes (up to ~15k lux) are direct-sun/reflection artifacts and are harmless — they never cause a false "light off" alarm, and the OFF-window ceiling (06:00 max 2.5) confirms sun never triggers a false "light on" alarm.

**Files.** None (configuration/validation only).

---

<a id="dl-070"></a>
### DL-070 — Instrument Shelly uptime/RSSI to separate reboots from WiFi dropouts

**Date:** 2026-06-15 · **Status:** Active — built, validated, deployed.

**Context.** DL-066 found the grow-light Shelly rebooting to ON. Investigation: firmware is current (`1.7.99`; only a `2.0.0-beta1` is offered — not for a depended-on node); uptime at check time was ~2.2 h (not a tight reboot loop); `reset_reason: 4` indicates a fault-triggered restart, not a clean mains power-on; RAM is healthy (`ram_min_free` never hit zero); RSSI is a weak **−72 dBm**; and the telemetry gaps are irregular across all hours. That profile points to mostly **WiFi dropouts** on the flaky campus network (DL-028) with occasional fault reboots — but a telemetry gap alone cannot distinguish a reboot from a dropout: a reboot resets the device's uptime counter, a dropout does not.

**Decision.** Add a hub-side poller (`plant-shelly-monitor.timer` → `plant-shelly-monitor.service` → `shelly_monitor.py`) that samples the Shelly's own `uptime` + `rssi` once a minute via one `Shelly.GetStatus` RPC call, logging both to `sensor_readings` (device=`shelly`) and writing a `metric='reboot'` marker to `system_status` when uptime goes backwards (mirroring the WROVER reboot detection, DL-060). This makes reboot-vs-dropout unambiguous after the fact and yields an RSSI time-series to correlate with the dropout timing — direct evidence for the campus-network case (DL-028). Shelly reboots are recorded but deliberately **not** wired into the alerter: the plug's WiFi is known-flaky, and the grow-light verification alert (DL-063) already pages on real functional impact.

**Rationale.** This is independent measurement of device truth rather than inference from telemetry gaps — the same commanded-vs-measured / independent-verification discipline applied elsewhere. The poller is a stateless timer (previous uptime read from the DB, so detection survives restarts), consistent with the retention timer (DL-068); it uses WAL + busy_timeout like every hub writer, and its rows fall under the 30-day `sensor_readings` retention window so they stay bounded.

**Validation.** In-memory test: climbing uptime logs no reboot; an uptime reset logs exactly one reboot marker carrying the pre-reboot uptime; the uptime+rssi series rows are written. Live first sample logged `uptime=Ns rssi=NdBm` and two `shelly` rows.

**Files.** `hub/11-shelly-monitor/shelly_monitor.py`, `hub/11-shelly-monitor/plant-shelly-monitor.{service,timer}`, `hub/11-shelly-monitor/README.md`.

---

<a id="dl-071"></a>
### DL-071 — Power-chart gridlines + land the DL-067 sargable code

**Date:** 2026-06-15 · **Status:** Active — deployed.

**Context.** Two things. (1) The power-draw chart's x-axis had no vertical gridlines (`showgrid=False`) and sparse ~3-hour ticks, making a point hard to place in time. (2) Investigating that, git history showed DL-067's *code* was never committed — commit `5d717e6` contained only the decision-log entry, so `alerter.py` and `dashboard.py` on `origin/main` still used the non-sargable `julianday(...)` form. (The sargable files were deployed to the Pi at DL-067 time but never staged; a later working-tree copy then overwrote them.)

**Decision.** (1) Power chart: vertical gridlines + fixed ticks — 2-hour spacing on the 24-hour view, 10-minute on the 1-hour view, `HH:MM` labels (`plot_power` now takes the window length to choose the interval). Soil chart left unchanged (its 24h/7d tabs have different tick needs). (2) Land DL-067 for real: all seven windowed queries (three in `alerter.py`, four in `dashboard.py`) now use `ts >= strftime('%Y-%m-%dT%H:%M:%SZ','now', ?)`, restoring index range-scans.

**Validation.** Both files compile; zero `julianday(...)` predicates remain. Sargable plan reconfirmed live (`(sensor=? AND ts>?)`). Dashboard renders the new gridlines.

**Note.** This is the commit that actually lands DL-067's code; the DL-067 entry was accurate in intent and is unchanged.

**Files.** `hub/06-dashboard/dashboard.py`, `hub/04-listener/alerter.py`.

---

<a id="dl-072"></a>
### DL-072 — Housekeeping: unify Shelly device label, dedup import, gitignore handoff

**Date:** 2026-06-15 · **Status:** Active — deployed.

**Context.** A consistency audit after DL-071 found three cosmetic issues, none functional. (1) The DL-070 Shelly monitor logged `device='shelly'` for uptime/rssi/reboot, while the rest of the project labels that same physical plug `device='grow-light'` (power, voltage, online status, actuator) — a split data model. Verified harmless first: the "Shelly online" card reads `system_status … metric IS NULL` (so `reboot` markers are excluded), `latest_sensor` is sensor-scoped (so uptime/rssi rows don't leak into power/voltage), and the flapping alert is `device='wrover'`-scoped. (2) A duplicate `from pathlib import Path` in `dashboard.py`. (3) `HANDOFF-cam-arc.md` sitting untracked in the repo root.

**Decision.** Unify the monitor onto `device='grow-light'` (code + README) and migrate the existing `shelly` rows in `sensor_readings`/`system_status`; remove the duplicate import; gitignore `HANDOFF-*.md`. The service/file/log names stay `shelly-monitor` — they describe the hardware; only the device *data label* follows the project convention. Supersedes the `device='shelly'` choice in DL-070.

**Validation.** Both files compile; zero `device='shelly'` references remain in code or docs; the live migration left 0 `shelly` rows.

**Files.** `hub/11-shelly-monitor/shelly_monitor.py`, `hub/11-shelly-monitor/README.md`, `hub/06-dashboard/dashboard.py`, `.gitignore`.

---

<a id="dl-073"></a>
### DL-073 — Lower soil watering trigger to ~30%

**Date:** 2026-06-15 · **Status:** Active — firmware change flashed 2026-06-19; watering-cycle validation pending next dry-down.

**Context.** The soil looked dry between waterings, and reaching the old trigger took close to a week. Ten days of logged moisture confirm it: a clean, near-linear dry-down of ~4 %/day, so from a ~47% post-water level to the old 21.6% trigger is ~6 days. Basil prefers consistent moisture. (Caveat noted: the capacitive probe reads at root depth, so surface drying ≠ root-zone dry; watering earlier suits basil, with overwatering/root-rot the risk to watch if drainage is slow.)

**Decision.** Lower `SOIL_THRESHOLD_TRIGGER` from 2400 to **2352** in `config.h`; leave `SOIL_THRESHOLD_STOP` at 2200. Percent via the linear calibration (`DRY=2523` → 0%, `WET=1953` → 100%): `raw = 2523 − 0.30×570 = 2352` ≈ **30%** (old 2400 ≈ 21.6%; stop 2200 ≈ 57%). Effect: waters at ~30% instead of ~21.6% — roughly every ~4 days in smaller, more frequent top-ups rather than ~weekly. The hysteresis band is 152 raw counts (~27 points), an order of magnitude above the ~10–16-count sensor noise (DL-020), so no toggling. The daily mL cap (DL-058) still backstops volume.

**Validation.** Threshold lowering is data-driven from the 10-day dry-down curve (~4 %/day). Flashed to the WROVER on 2026-06-19 and verified running clean — heartbeats, all sensors valid, FSM in `monitoring`. At flash time the soil read raw **2034 (~85%)** — wetter than the auto-stop (2200, ~57%) and well below the new trigger, consistent with a recent manual top-up — so watering correctly did **not** fire. The 2352 (~30%) trigger therefore awaits the next natural dry-down; the full cycle (`monitoring` → `watering`, pump fires, soil rises toward ~57%, clean stop, no immediate re-trigger) is to be confirmed then.

**Files.** `firmware/integrated/src/config.h`.

---

<a id="dl-074"></a>
### DL-074 — Hub-side photoperiod enforcement (self-heal the Shelly)

**Date:** 2026-06-16 · **Status:** Active — deployed.

**Context.** The grow light didn't come on at 07:00 (had to be turned on by hand). Diagnosis via the DL-070 monitor: the onboard schedule was intact and enabled and the Shelly's clock was NTP-synced — but the plug is rebooting every ~90 min (9 reboots in 10 h) on weak WiFi (RSSI avg −74.5, worst −82 dBm, DL-028). A reboot around the 07:00 trigger made the onboard scheduler miss it, and `restore_last` kept the (overnight-OFF) state. The DL-063 alert *did* fire (08:15 "may be off" → +30 s "back to normal" — the reboot flap), proving detection works but that there was nothing to correct it automatically. The onboard schedule (DL-054) inherits all of the plug's instability.

**Decision.** Add hub-side enforcement: `photoperiod.py`, run every 2 min by `plant-photoperiod.timer`, asserts the desired state from the Pi's NTP-correct clock over HTTP RPC — symmetric (ON inside 07:00–19:00, hard-OFF outside), idempotent (reads `Switch.GetStatus`, calls `Switch.Set` only when wrong), and quiet (logs only corrections/errors, so the journal records how often the plug was found wrong). The Shelly's onboard schedule is kept as a fast-path fallback; the hub is the enforcer.

**Rationale.** The Pi is always-on, NTP-correct, and reaches the plug over HTTP even while the plug's MQTT/WiFi flaps. Re-asserting every 2 min means any reboot-induced wrong state self-heals within one tick (worst-case ~2 min of darkness, trivial for DLI), so a 07:00 reboot can no longer cost the day — closing the loop from detection (DL-063) to correction. Supersedes DL-054 as the authoritative control path.

**Validation.** `desired_on` unit-tested (ON 07:00–18:59, OFF otherwise). Live: forced the plug ON at night, ran the service, it corrected to OFF and logged the correction.

**Note.** This *masks* but does not cure the reboot/WiFi problem (−82 dBm); plug placement / extender / outlet still to be addressed. Monitoring the plug fit and link.

**Files.** `hub/08-grow-light/photoperiod.py`, `hub/08-grow-light/plant-photoperiod.{service,timer}`, `hub/08-grow-light/README.md`.

---

<a id="dl-075"></a>
### DL-075 — Shelly reboot cause: investigated, not curable in software; masked by the hub

**Date:** 2026-06-17 · **Status:** Active — accepted/monitoring; no code change.

**Context.** The grow-light Shelly Plug reboots chronically (~8–12×/day; 8 between midnight and 10:24 on 06-17), which is what caused the 06-16 missed 07:00 trigger. With the DL-070 monitor's uptime/RSSI/reboot data now accumulated, the cause was investigated rather than guessed.

**Findings (all from logged data + live RPC).**
- **Not WiFi.** RSSI before each reboot was a tight −73 to −75.7 (day avg −75.3), i.e. ordinary middling signal — reboots do **not** cluster at the −82 lows. Reboots are uncorrelated with signal strength, so the weak link is not the trigger. (Signal is still mediocre and worth improving for telemetry reliability, but it is not the cause.)
- **Not memory.** `ram_min_free` ~148 KB, never near zero — no leak.
- **Not an obvious brownout (snapshot).** Live electrical reading nominal: 119.1 V, 60.0 Hz, 15.7 W, 45 °C. A momentary sag wouldn't show in a one-shot reading, but nothing points to it.
- **Consistent fault reset.** `reset_reason: 4` (fault-class, not a clean power-on) on every restart.
- Reseating the plug in the extension box did not reduce the reboot rate.

**Conclusion.** Healthy RAM + nominal power + uncorrelated WiFi + a regular fault-reset every ~1–2 h points to a **flaky unit or firmware bug on this specific hardware** — a replacement/RMA candidate, not something fixable by reseating, relocating, or an extender. Not worth further chasing now.

**Decision.** Accept the reboots and let the **hub photoperiod enforcer (DL-074) mask them** — which it does: the plant had a flawless light day on 06-17 (continuous ON from 07:00, eyewitness-confirmed) through ~10 reboots, with zero hub corrections needed because `restore_last` happened to match the window on each reboot. Keep monitoring via DL-070; revisit (RMA / swap unit) if the rate worsens or if a reboot ever lands the light in a wrong state the hub can't cover.

**Note — telemetry reading lesson.** Power-telemetry gaps during reboots (e.g. the 06-17 "07:11 first-on" and 08:57–09:08 gap) are **MQTT dropouts, not the light turning off** — the light was continuously on by direct observation. With this plug, absence of power data ≠ absence of light.

**Files.** None (investigation/decision only).

---

<a id="dl-076"></a>
### DL-076 — Camera vision arc, slice 1: the Pi image receiver

**Date:** 2026-06-19 · **Status:** Active. Validated on the Pi (curl round-trip, file + DB row written).

**Context.** Opening the camera vision arc. The XIAO ESP32-S3 Sense (vision node per DL-034, bring-up rig cleared in DL-062) will capture periodic JPEGs of the basil and upload them to the hub for a greenness/leaf-area trend. Per the project definition, scope is deliberately bounded: capture → transmit → Pi stores → simple colour metric → dashboard trend. ML classification, disease ID, predictive models, and multi-plant are explicitly post-v1. This entry covers **only the Pi-side receiver**; XIAO wiring and firmware are a later slice with their own DL.

**Decision — transport (option B).** Image bytes travel over **HTTP**; the lightweight capture event and node presence travel over **MQTT** (listener side, added later). Reasoning: images are too large for clean MQTT payloads (the handoff's own warning), so the broker stays reserved for small telemetry/presence while HTTP carries the blob with native large-body support and simple retry. This keeps the XIAO a first-class networked node alongside the WROVER, rather than tethering it (USB-serial was the considered fallback if campus WiFi blocked HTTP — it did not).

**Decision — greenness by Excess-Green, not OpenCV.** Greenness = fraction of pixels whose Excess-Green index (`ExG = 2g − r − b`, normalized RGB) exceeds `GREEN_EXG_THRESHOLD` (0.10), computed with numpy + Pillow. ExG is a standard vegetation index, brightness-robust, and the metric tracks foliage area in frame rather than health stress. OpenCV was deliberately **not** adopted: greenness is trivial colour maths and cv2 is a heavy Pi dependency unjustified until real CV/ML work needs it. This is a conscious deviation from the earlier "OpenCV planned" note.

**Implementation.** `hub/09-camera/image_receiver.py` — a small always-on threaded `http.server`: `POST /image` (raw JPEG) stores the file as `cam-YYYYMMDD-HHMMSS-ffffff.jpg` (microsecond suffix so rapid captures never collide), computes greenness, and inserts `(ts, path, greenness)` into a new `camera_readings` table opened **WAL + busy_timeout** (the DL-066 multi-reader rule) so the dashboard can read while the receiver writes; `GET /health` for liveness. Config via the `/etc/plant-hub/credentials` EnvironmentFile (port, image dir, DB path, threshold). Deployed Pi-local at `/home/basilpi/plant-hub/image_receiver.py`; numpy and Pillow already present in the hub venv.

**Validation.** Deployed and run manually on the Pi; POSTed the colour-swatch fixture from the Mac over the **main LAN address** (`10.6.19.139:8080`, no Tailscale needed — campus isolation did not block Mac↔Pi HTTP). Receiver decoded 1448×1086, stored the file, returned JSON, and wrote the row: `greenness = 0.4817`, matching the chart's roughly-half-green composition — confirming the ExG maths end-to-end. The fixture is retained as `hub/09-camera/test-fixtures/greenness-reference-chart.jpg`, a regression reference for the metric.

**Not yet (later slices).** The systemd service is written (`plant-image-receiver.service`) but not yet enabled. The MQTT capture-event / presence wiring (joining the DL-059 watchdog set), the dashboard latest-image panel and greenness trend, the rolling image-retention window, and all XIAO wiring/firmware are out of scope here and tracked separately.

**Files.** `hub/09-camera/image_receiver.py`, `hub/09-camera/plant-image-receiver.service`, `hub/09-camera/README.md`, `hub/09-camera/test-fixtures/greenness-reference-chart.jpg`.

---

<a id="dl-077"></a>
### DL-077 — XIAO ESP32-S3 Sense vision-node bring-up

**Date:** 2026-06-19 · **Status:** Active — validated on hardware.

**Context.** Opening the firmware side of the camera vision arc. Before any capture/transport logic, the XIAO ESP32-S3 Sense (the vision node, DL-034; rig lessons from DL-062) was brought up in isolation — board + camera only, no WiFi/SD/HTTP — so a failure here points at hardware, not networking. Mirrors the slice discipline that cleared the ESP32-CAM.

**Validation.** Bring-up sketch `firmware/test-sketches/14-xiao-cam/` flashed over a single USB-C data cable and ran clean: **PSRAM 8 MB found**, **camera init OK**, **sensor PID 0x3660 (OV3660)**, and steady SVGA (800×600) JPEG frames at ~16 KB with a monotonic counter and no resets. The XIAO auto-resets into the bootloader over native USB — no FTDI, no GPIO0 jumper, no boot-button dance — exactly the friction the ESP32-CAM imposed and the reason DL-034 chose this board.

**Sensor note.** This unit ships the **OV3660** (3 MP), not the OV2640 named in older docs — newer Seeed Sense boards transitioned to it. The `esp_camera` driver auto-detects the sensor, so firmware is unaffected; greenness (foliage-area colour maths) is resolution-agnostic.

**Cable lesson.** The board would not enumerate on the first USB-C cable — a **power-only cable** carries power but no data, so no serial port appears (`ls /dev/cu.*` showed no `usbmodem`). A data cable fixed it immediately. The power-only cable is not useless: it is the **deployment** power cable (XIAO on the tripod → wall adapter, talking to the Pi over WiFi); it simply cannot program the board. Corollary: a deployed XIAO on a power-only cable has no serial — debugging is then over WiFi/MQTT, not USB.

**Not yet (next slice).** WiFi association and the campus-network path, U.FL antenna seating, the HTTP-POST transport to the Pi receiver (`hub/09-camera`), capture cadence, and top-down mounting — the capture-and-POST slice that follows.

**Files.** `firmware/test-sketches/14-xiao-cam/` (sketch + platformio.ini + README).

---

<a id="dl-078"></a>
### DL-078 — Camera node v1: capture and POST to the Pi receiver

**Date:** 2026-06-20 · **Status:** Active — validated end-to-end on hardware.

**Context.** With the Pi receiver (DL-076) and the XIAO bring-up (DL-077) each proven in isolation, this slice connects them — the first real vision-node firmware. Scope deliberately bounded to the new risk (WiFi + capture + HTTP POST); the MQTT capture event/presence, deployment cadence, and mounting are deferred to later slices.

**Decision.** New production firmware project `firmware/camera-node/` (sibling to `firmware/integrated/` — one firmware per node, not a test-sketch, since this code is permanent and accumulates). Modular like the WROVER: `net_wifi` (bounded boot connect + non-blocking reconnect; WiFi loss non-fatal), `camera` (XIAO `CAMERA_MODEL_XIAO_ESP32S3` pin map, SVGA JPEG in PSRAM), `poster` (`HTTPClient` POST of the raw bytes), `config.h` (endpoint, cadence, timeouts), and a gitignored `secrets.h` (template committed) mirroring the WROVER's credentials pattern. Transport is option B (DL-076): bytes over HTTP to `IMAGE_POST_URL`.

**Validation.** Flashed to the XIAO (U.FL antenna seated). Serial showed camera init OK, WiFi associated to JSU_DEVICE (IP 10.6.17.106), SVGA frames captured and POSTed; the receiver returned `POST 200` with a greenness value, and the Pi independently logged matching `stored cam-*.jpg … greenness=…` rows — both sides observed. The **main LAN address worked** (`10.6.19.139:8080`); campus isolation (DL-028) did not block XIAO→Pi HTTP, so no Tailscale fallback was needed. Greenness tracked scene content (≈ 0.001 on a non-plant scene, rising to ~0.10–0.15 as something green entered frame), confirming the ExG metric responds correctly; real plant values await top-down mounting.

**Notes.** WiFi RSSI was −79 dBm — weak-ish (the same mediocre-signal neighborhood as the Shelly, DL-075); fine here, but the first suspect if deployed-spot POSTs turn flaky. Capture cadence is a short TEST value (20 s) in `config.h`; the deployment cadence (hourly, photoperiod-gated) is a later slice. A protective film over the lens (peeled after this test) can soften images.

**Not yet (later slices).** MQTT capture event + presence (DL-059 watchdog), hourly/photoperiod-gated cadence, top-down tripod mounting, dashboard latest-image panel + greenness trend, rolling image retention.

**Files.** `firmware/camera-node/` (platformio.ini, README, src/{main,net_wifi,camera,poster}.{h,cpp}, config.h, secrets.h.example).

---

<a id="dl-079"></a>
### DL-079 — Receiver plant isolation: greenness on the largest green blob

**Date:** 2026-06-21 · **Status:** Active — validated on the live plant.

**Context.** Full-frame greenness (DL-076) has two weaknesses once the camera is mounted over the real plant: it counts any green in frame (a contaminant risk), and it implicitly assumes the plant stays centered. The pot can realistically be bumped — by cleaning, by people, by knocking the grow-light stand — and the drift would not be noticed until hours of corrupted data later. The metric should locate the plant itself.

**Decision.** Measure greenness on the **largest connected green blob** in the ExG mask (the plant is the dominant contiguous green mass), keeping the full image saved. This is self-locating (survives the pot shifting within frame) and ignores small stray green specks (a breadboard status LED, a leaf highlight) — it is not object tracking, which would be overkill for a stationary plant; "find the green" *is* the tracking here. Three values are recorded: `greenness` (full-frame ExG ratio, unchanged — kept for continuity and the regression fixture); `green_area` (largest blob as a **fraction of the frame**, "how much plant" — a fraction, so resolution-independent and comparable across a capture-size change; the growth signal); and `green_ratio` (green fraction within that blob's bounding box, "how green the plant region is"). Connected components via `scipy.ndimage`; if scipy is absent the code falls back to the full-frame ratio. The `camera_readings` schema gains `green_area`/`green_ratio`, added with `ALTER TABLE` guards so an existing table migrates in place. The one non-plant green source in frame (sticky notes) was removed physically rather than filtered.

**Validation.** Deployed to the Pi (scipy installed in the hub venv) and run against the live mounted basil. The regression fixture still yields **greenness ≈ 0.4817** (full-frame metric untouched). On the plant, `green_ratio` (~0.62) sits well above full-frame `greenness` (~0.20), confirming the blob isolates the plant region rather than the whole beige frame; `green_area` (~0.03) and `green_ratio` were **stable frame-to-frame and held across SVGA/UXGA/QXGA** captures (area ~0.029→0.026, ratio ~0.62→0.56), confirming the fraction-based metrics are resolution-independent. The largest-blob selection correctly tracked the basil, not the breadboard LED.

**Notes.** scipy installs on the Pi 4 from the piwheels aarch64 wheel (scipy 1.18.0, no source build). Full-frame `greenness` falls at higher resolution as finer leaf-gap and soil pixels resolve out of the green mask — expected, and the reason `green_ratio`/`green_area` (measured on the isolated blob) are the durable metrics across a resolution change.

**Files.** `hub/09-camera/image_receiver.py`, `hub/09-camera/README.md`.

---

<a id="dl-080"></a>
### DL-080 — Deployment capture resolution: UXGA

**Date:** 2026-06-21 · **Status:** Active — decided from on-hardware comparison.

**Context.** The OV3660 supports SVGA through QXGA. The deployment resolution trades image detail (useful for a future ML plant-health classifier, which would train on the real captured archive) against payload size over a weak WiFi link (RSSI ≈ −79 dBm) and long-term storage. Decided empirically by capturing the same mounted, lit plant at each size and comparing.

**Evidence.** Three captures (`jpeg_quality = 10`) kept under `hub/09-camera/test-fixtures/resolution-comparison/`: SVGA 800×600 (~33 KB), UXGA 1600×1200 (~110 KB), QXGA 2048×1536 (~170 KB). **SVGA → UXGA is a dramatic detail jump** (SVGA blurs leaf veins/edges; UXGA resolves them — the detail a classifier needs). **UXGA → QXGA is marginal** (small sensor + indoor light limit real detail beyond ~2 MP) while the file is ~55% larger.

**Decision.** **UXGA** (`FRAMESIZE_UXGA`). It captures the bulk of the usable detail; QXGA's extra detail is barely visible and not worth ~55% larger files on a marginal link and in long-term storage. Both UXGA and QXGA posted at 100% over the (good-cable) link, but UXGA leaves more headroom on the weak signal. The Pi-side `green_area`/`green_ratio` metrics are fractions and held steady across all three sizes (DL-079), so the resolution choice does not disturb the greenness trend — it is purely an image-quality-vs-cost call. The `FRAMESIZE_UXGA` change itself lands with the deployment firmware (a later slice); this entry records the decision and its evidence.

**Aside (USB flashing).** Repeated `Write timeout` / lost-connection failures while flashing this series were root-caused to a **marginal USB-C data cable**, not the board or QXGA payload (the failures were on the USB upload, not the WiFi POST). Replacing the cable fixed it. The flaky cable is still usable as a power-only deployment cable. Keep the known-good data cable for reflashing the mounted node.

**Files.** `hub/09-camera/test-fixtures/resolution-comparison/` (svga-800x600.jpg, uxga-1600x1200.jpg, qxga-2048x1536.jpg, README.md).

---

<a id="dl-081"></a>
### DL-081 — Deployment firmware: UXGA, hourly cadence

**Date:** 2026-06-21 · **Status:** Active — firmware edited; flashed at go-live.

**Context.** Moving the camera node from bench-test settings to deployment settings, ahead of an unattended multi-day run. Three changes to `firmware/camera-node/`, none of them new logic — they implement decisions already made.

**Changes.** (1) `FRAMESIZE_UXGA` 1600×1200, implementing the resolution decision of DL-080. (2) `jpeg_quality` 12 → 10 (less compression, more detail for the future training archive). (3) `CAPTURE_INTERVAL_MS` 20 s → 3600000 (hourly).

**Photoperiod gating stays off the node.** The node carries no clock and captures every hour, day or night; whether to *keep* a capture is decided Pi-side by the receiver against the shared grow-light window (DL-082). Rationale (DL): the Pi is the always-on, NTP-correct photoperiod authority (it already runs the enforcer, DL-074), so the window is defined once; the XIAO stays dumb, with no NTP/timezone code to fail on a node that is hard to debug once mounted. The cost is a few discarded dark frames per night — trivial.

**Validation.** UXGA capture + POST already validated on the live plant at 100% over the link (DL-080 evidence run). The cadence and quality values are config constants observed individually during that run; the hourly interval is verified in situ at go-live (first hourly capture lands).

**Files.** `firmware/camera-node/src/camera.cpp`, `firmware/camera-node/src/config.h`, `firmware/camera-node/README.md`.

---

## Maintaining this log

When a new decision is made:

1. Pick the next free `DL-NNN` ID
2. Add it to the index table near the top
3. Append a full entry at the bottom (chronological — newer at the end)
4. If it modifies an earlier entry, update that earlier entry's **Status** line to reference the new one (e.g. "Refined by DL-024"), but never rewrite its content

Old entries stay as they were originally written. The history is the value.
