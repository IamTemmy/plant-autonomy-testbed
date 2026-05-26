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
| DL-001 | 2026-05-16 | Plant choice: basil | Active |
| DL-002 | 2026-05-16 | Pot size and growing medium | Active |
| DL-003 | 2026-05-16 | Sensor suite | Active |
| DL-004 | 2026-05-16 | Actuator switching: MOSFET, not relay | Active |
| DL-005 | 2026-05-16 | Telemetry hub and stack | Active |
| DL-006 | 2026-05-16 | Firmware framework | Active |
| DL-007 | 2026-05-16 | Vision as a separate node | Active |
| DL-008 | 2026-05-16 | PCB deferred, protoboard as final form | Active |
| DL-009 | 2026-05-25 | MOSFET part substitution: IRLZ44N → IRLB8721 | Active |
| DL-010 | 2026-05-25 | Grow light strategy: LBW lamp + Shelly smart plug | Active |
| DL-011 | 2026-05-25 | Protoboard deferred from immediate Phase 1 | Active |
| DL-012 | 2026-05-25 | Power distribution discipline | Active |
| DL-013 | 2026-05-25 | Leak sensor added as a fault input | Active |
| DL-014 | 2026-05-25 | Reservoir construction and float switch mounting | Active |
| DL-015 | 2026-05-26 | BME280 bench test validated | Active |
| DL-016 | 2026-05-26 | Power architecture: keep buck converter | Active |
| DL-017 | 2026-05-26 | LM2596 buck converter validated | Active |

---

## Part 1 — Project Baseline (Project Definition v2.0, 16 May 2026)

These entries record the original decisions from the project definition so the log is self-contained — a reader can understand the project from this file alone.

### DL-001 — Plant choice: basil

**Date:** 2026-05-16 · **Status:** Active

**Context.** The project needs a plant that gives fast visible feedback for control-loop tuning, has real-world parallel, is hardy enough to survive bench testing, and fits a single small pot.

**Decision.** Basil (*Ocimum basilicum*).

**Rationale.** Fast growth means the system's effectiveness shows quickly. Basil is a common commercial indoor crop, giving the project real-world parallel. It is hardy enough for an imperfect first build, edible, and fits a 5–6" pot.

**Alternatives considered.** Succulents (too slow for control-loop feedback). Snake plant or peace lily (too large, slow growth). Mint (invasive). Parsley (slow germination).

---

### DL-002 — Pot size and growing medium

**Date:** 2026-05-16 · **Status:** Active

**Context.** Pot size affects both root development and the dynamics of the soil-moisture control loop. Too small and sensor noise dominates; too large and the soil never reaches the sensor in a meaningful way.

**Decision.** 5–6" pot with mandatory drainage hole. Plain indoor potting mix, no moisture-retention additives.

**Rationale.** 5–6" smooths moisture dynamics enough to give the control loop margin while still being small enough that under-rooted soil does not stay waterlogged. The drainage hole prevents root rot. Plain mix means the sensor reads actual water content rather than the buffered reservoir of polymer crystals.

**Alternatives considered.** 4–5" pot (twitchier control problem). 6–8" pot (oversized for basil). Moisture-control soil with water-retention crystals (would fight the control system).

---

### DL-003 — Sensor suite

**Date:** 2026-05-16 · **Status:** Active

**Context.** The system needs to perceive soil moisture, air conditions, light, and reservoir level.

**Decision.** Capacitive soil moisture sensor; BME280 for air (temperature, humidity, pressure); BH1750 for light (calibrated lux); float switch for reservoir level.

**Rationale.** Capacitive sensors do not corrode (resistive probes do, defeating long-term reliability). BME280 covers three useful air parameters on one I²C device. BH1750 returns calibrated lux and covers the full indoor range. A float switch is the simplest reliable low-water detector.

**Alternatives considered.** Resistive soil sensor (corrodes). DHT22 air sensor (no pressure, lower precision). TSL2591 (wider light range, judged unnecessary indoors). Ultrasonic water-level sensor (more complex than required for a binary low-water signal).

---

### DL-004 — Actuator switching: MOSFET, not relay

**Date:** 2026-05-16 · **Status:** Active. See also DL-009 (specific part) and DL-010 (grow light no longer uses this method).

**Context.** The pump and grow light both need to be switched by the ESP32. The two main options are mechanical relays and logic-level MOSFETs.

**Decision.** Logic-level N-channel MOSFETs in a low-side configuration, with a flyback diode across the pump.

**Rationale.** MOSFETs are silent (no relay click), have no mechanical wear, switch fast enough for PWM dimming if needed, and tolerate frequent cycling. Relays would have meant audible clicking every watering cycle and limited operational life.

**Alternatives considered.** Mechanical relay (noise, wear, no PWM). Solid-state relay (overkill for low-voltage DC). Direct GPIO drive (insufficient current).

---

### DL-005 — Telemetry hub and stack

**Date:** 2026-05-16 · **Status:** Active

**Context.** The system needs remote telemetry, persistent logging, a dashboard, and push alerts. Options span paid cloud SaaS to fully self-hosted local.

**Decision.** Raspberry Pi 4 hub running Mosquitto (MQTT broker), a Python data service, SQLite database, and a Streamlit web dashboard. Push alerts via ntfy.sh; non-urgent summaries by email.

**Rationale.** The Pi 4 is already owned; the stack has no recurring cost and no message limits. MQTT is the industry-standard telemetry protocol, so the skills are transferable. Streamlit gives a browser dashboard with minimal code. ntfy.sh is free and self-hostable.

**Alternatives considered.** AWS IoT or Azure IoT Hub (recurring cost, vendor lock-in). Blynk or Adafruit IO (free tier message limits). Custom mobile app (out of scope for a portfolio piece).

---

### DL-006 — Firmware framework

**Date:** 2026-05-16 · **Status:** Active

**Context.** ESP32 firmware can be written against ESP-IDF (native), the Arduino framework, or in MicroPython.

**Decision.** C++ on the Arduino framework, built and managed by PlatformIO.

**Rationale.** Arduino keeps the firmware approachable while running bare-metal. PlatformIO pins library versions in `platformio.ini` (in the repo), giving reproducible builds — anyone who clones the repo gets an identical environment. This matters for a project whose goal includes replicability.

**Alternatives considered.** ESP-IDF (more capable, harder to replicate). MicroPython (slower, less suited to real-time sensor reads). Arduino IDE without PlatformIO (no environment lock — versions drift across machines).

---

### DL-007 — Vision as a separate node

**Date:** 2026-05-16 · **Status:** Active

**Context.** The system needs a camera for time-lapse and basic visual health observation. The main ESP32 already manages many sensors and actuators.

**Decision.** ESP32-CAM as a dedicated second node for capture and upload. All image processing happens on the Pi.

**Rationale.** The ESP32-CAM uses most of its GPIO pins for the camera bus and cannot simultaneously drive sensors and actuators. Dedicating it keeps responsibilities clean (single-responsibility per node) and matches the broader architectural principle of separation of concerns. Pi-side processing has access to more compute and Python's image ecosystem.

**Alternatives considered.** Camera on the main ESP32 (impossible — pin conflict). USB camera on the Pi (no — vision should be at the plant). On-device ML on the ESP32-CAM (deferred to v2 per the project definition).

---

### DL-008 — PCB deferred, protoboard as final form

**Date:** 2026-05-16 · **Status:** Active. Refined further by DL-011 (deferred even out of Phase 1).

**Context.** The final assembly needs to be cleaner than a breadboard for a portfolio-quality demonstration.

**Decision.** A clean protoboard build is the acceptable final form. A custom KiCad PCB is optional, pursued only if time allows.

**Rationale.** Protoboard transcription of a verified breadboard gives a professional finish without the PCB design cycle (schematic, layout, fabrication, assembly, debug). The engineering content of this project is in the firmware, system design, and autonomy logic — not the board layout — so PCB time is not the highest-leverage investment.

**Alternatives considered.** Stay on breadboard (looks unfinished). PCB now (significant time cost). Wire-wrap (legible but slow).

---

## Part 2 — Phase 1 Refinements (25 May 2026)

These entries record decisions made after the project definition was published, during procurement and bench-test planning. Where they modify the PDF baseline, that supersession is called out explicitly.

### DL-009 — MOSFET part substitution: IRLZ44N → IRLB8721

**Date:** 2026-05-25 · **Status:** Active. Supersedes the IRLZ44N specification in PDF §2 and §7.

**Context.** The PDF specifies two IRLZ44N MOSFETs. The MOSFET actually ordered (Adafruit's standard logic-level N-channel) is the IRLB8721.

**Decision.** Use IRLB8721 as the pump driver. Pin order, gate resistor, and flyback-diode configuration are identical to the original IRLZ44N plan.

**Rationale.** IRLB8721 is also a logic-level N-channel MOSFET fully turned on at 3.3 V gate drive. Rds(on) at logic-level drive is actually lower than IRLZ44N (~16 mΩ vs ~22 mΩ), giving slightly less heat dissipation. Pinout (G/D/S) is identical. No design changes required.

**Alternatives considered.** Order IRLZ44N separately (extra shipping and delay). Use a dedicated MOSFET driver IC (overkill at this current).

---

### DL-010 — Grow light strategy: LBW lamp + Shelly smart plug

**Date:** 2026-05-25 · **Status:** Active. **Supersedes** the grow light specification in PDF §2 (Concluded Decisions), §6 (Wiring — grow-light MOSFET on GPIO26), and §7 (BOM grow light entry).

**Context.** The PDF specified a full-spectrum 12 V DC LED grow light panel switched and PWM-dimmed by a MOSFET, with Daily Light Integral (DLI) ramping for graceful supplementation. The grow light actually purchased is an AC-powered floor lamp with an integrated driver and a mechanical on/off switch (LBW Grow Light with adjustable tripod stand, 15–48").

**Decision.** Keep the LBW lamp. Control it via a Shelly smart plug speaking MQTT to the existing Mosquitto broker on the Pi. The 12 V DC MOSFET-controlled panel is deferred to "later, if needed."

**Rationale.** The LBW lamp cannot be MOSFET-switched or PWM-dimmed because its driver is internal and runs on AC mains. The Shelly smart plug speaks MQTT natively, so it integrates cleanly into the existing telemetry stack: the ESP32 (or the Pi-side orchestrator) publishes a `light/cmd` topic, the Shelly subscribes and acts. The DLI control logic adapts from a smooth PWM ramp to an on-until-target-then-off pattern — the requirement (supplement light when the day's accumulated DLI will fall short) is still satisfied, just with coarser control granularity.

**Alternatives considered.** Return the LBW lamp and order a 12 V DC panel (restores original design, but discards a part already in hand). SSR or mechanical relay on the AC line (works, but adds AC mains complexity to the build — undesirable for a portfolio piece unless the demonstration intent is AC switching). Keep the LBW lamp manual or always-on (defeats the autonomy framing for the lighting subsystem).

---

### DL-011 — Protoboard deferred from immediate Phase 1

**Date:** 2026-05-25 · **Status:** Active. Refines DL-008.

**Context.** The PDF (§7 BOM, §8 Phase 1) included the protoboard in the Phase 1 shopping list. On reflection, breadboard testing is sufficient until integration begins, and protoboard transcription belongs to a later phase.

**Decision.** Remove the protoboard from the immediate Phase 1 BOM. Treat it as deferred or optional, to be revisited when Phase 2 integration begins (if at all).

**Rationale.** Phase 1's goal is component validation, not permanence. Breadboard is faster to iterate on, easier to rewire when a sensor needs to move. Protoboard transcription belongs to Phase 2, when the integrated wiring is stable and ready to be made semi-permanent.

**Alternatives considered.** Build to protoboard from day one (slows iteration). Skip protoboard entirely in favor of PCB (re-opens the PCB time-cost discussion).

---

### DL-012 — Power distribution discipline

**Date:** 2026-05-25 · **Status:** Active. Refines PDF §6 (Wiring).

**Context.** The PDF describes the power tree at a logical level (12 V → buck → 5 V to ESP32; 12 V → pump and light) but does not specify the physical hardware for cleanly splitting the 12 V rail among multiple loads, nor does it state explicitly that actuator current must not flow through the breadboard or the ESP32 board.

**Decision.** Use WAGO / lever-nut connectors for clean 12 V rail and ground distribution. Explicit ground-discipline rule: all grounds (12 V supply, MOSFET source, buck converter output, ESP32) are tied together at a single common ground. The ESP32 only sources gate signals; actuator current never flows through breadboard traces or through the ESP32 board's regulator.

**Rationale.** WAGO lever connectors are professional, reusable, and far more reliable than twisted-wire splices or breadboard rails for the kind of current the pump and grow-light circuits carry. Stating the ground-discipline rule in writing makes it inspectable and replicable. Forgetting common ground is a classic bug; documenting it prevents it.

**Alternatives considered.** Soldered splices (less reusable, harder to reconfigure). Terminal strip (acceptable, bulkier). Running actuator current through the breadboard (unsafe — breadboard contact resistance and current limits make this risky).

---

### DL-013 — Leak sensor added as a fault input

**Date:** 2026-05-25 · **Status:** Active. Extends PDF §4 (Control State Machine, Fault state).

**Context.** The PDF defines fault detection for sensor failure and closed-loop verification of watering, but does not include a sensor for detecting water *outside* the reservoir — leaks, overflow, or burst tubing.

**Decision.** Add a water-detection pad (leak sensor) under the reservoir/pump area, wired as a digital input to the ESP32. Firmware behavior: wet detected outside the reservoir → immediately stop the pump → enter Fault state → alert the operator.

**Rationale.** This fits the project's engineering framing directly. Detecting a leak before it floods is exactly the kind of failure mode an autonomous system should handle gracefully. The cost is small, the safety upside is significant, and it makes the closed-loop fault detection story stronger.

**Alternatives considered.** No leak sensor (the original PDF position — relies on tubing not failing). Pressure sensor on the reservoir (more complex, less direct signal). Camera-based leak detection (deferred — falls in the ML scope for v2).

---

### DL-014 — Reservoir construction and float switch mounting

**Date:** 2026-05-25 · **Status:** Active. Refines PDF §5 (Physical Setup) and §7 (BOM — reservoir entry).

**Context.** The PDF specifies a ~2 L food-grade reservoir with a float switch mounted through the lid or wall, but does not detail tube routing, float-switch mount placement, or the relationship between the lid and the internal components.

**Decision.** Select a food-safe reservoir container with a drillable lid, target cost under ~$15. Drill the lid for: (1) the pump inlet tube, which extends down to reach near (but not flush against) the bottom of the reservoir, and (2) the float switch, which hangs from the lid into the water. The pump outlet tube routes to the pot. The float switch's NO/NC orientation is tested with a multimeter before firmware logic is committed.

**Rationale.** A drilled lid keeps the assembly clean, contains splashes, suppresses evaporation, and provides a stable mount point for the float switch. Inlet tube reaching near the bottom (but not flush) lets the pump draw nearly all the water without the tube end being blocked by the container floor. Testing NO/NC orientation up front prevents inverted-logic bugs in firmware.

**Alternatives considered.** Open reservoir with side-mounted float (less clean, more evaporation, weaker float mount). Inlet tube fixed flat against the bottom (flow blockage risk). Skip the lid entirely (no float mount, evaporation, debris ingress).

---

### DL-015 — BME280 bench test validated

**Date:** 2026-05-25 · **Status:** Active

**Context.** Phase 1 component validation requires every sensor to be exercised individually before integration. BME280 is the first sensor under test.

**Decision.** BME280 bench test passes. The sensor is detected on the ESP32's default I²C bus, returns plausible indoor temperature, humidity, and pressure readings, and responds correctly to a breath stimulus (temperature and humidity both rise within seconds and recover to baseline within ~60 seconds). The sensor is approved for integration.

**Rationale.** The sketch confirms: (1) the I²C bus is wired correctly and the auto-detect across addresses 0x76/0x77 works as written; (2) the Adafruit library returns calibrated values, not raw counts; (3) values pass plausibility-bound checks (no `[WARN: implausible reading]` lines); (4) the sensor is not stuck — the breath response proves real-time measurement, not a constant fake value. This last point matters more than the absolute accuracy: a sensor that always reads "24°C, 45%" looks healthy but is useless to a control loop.

**Alternatives considered.** None — this is a validation outcome, not a design choice. Evidence: `docs/images/01-bme280-validation.png`.

---

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

## Maintaining this log

When a new decision is made:

1. Pick the next free `DL-NNN` ID
2. Add it to the index table near the top
3. Append a full entry at the bottom (chronological — newer at the end)
4. If it modifies an earlier entry, update that earlier entry's **Status** line to reference the new one (e.g. "Refined by DL-024"), but never rewrite its content

Old entries stay as they were originally written. The history is the value.
