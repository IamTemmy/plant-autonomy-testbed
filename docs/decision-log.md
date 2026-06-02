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
| [DL-031](#dl-031) | 2026-06-02 | Shelly Plus Plug US ("basilplug") paired and joined JSU_DEVICE | Active |

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

**Alternatives considered.** None — this is a validation outcome, not a design choice. Evidence: `docs/images/01-bme280-validation.png`.

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

**Pre-power continuity verification.** Before power was applied, the wiring was verified by a continuity-test matrix covering common ground, gate signal path through the 220Ω series and 10kΩ pull-down resistors, drain path through the pump, diode orientation (forward voltage measured at ~0.596V — within normal range for a silicon diode), and absence of a 12V-to-ground short. An apparent +12V-to-drain short was correctly diagnosed as conduction through the pump's motor winding, not a wiring fault — verified by disconnecting one pump lead and re-running the relevant check. This diagnostic discipline (verify continuity before power, distinguish real shorts from coil-mediated paths) is now part of the project's bench-test practice. Evidence: `docs/images/02-pump-mosfet-validation.png`.

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

**Date:** 2026-05-31 · **Status:** Resolved 2026-06-02 — JSU_DEVICE confirmed viable across three devices (Pi, Mac, Shelly Plus Plug); proceeding with project on JSU_DEVICE as the deployment network

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
3. **WPA2-Personal (PSK), not WPA2-Enterprise (EAP)** — the Shelly Plus Plug US does not support enterprise authentication. WPA3-Personal is acceptable on the Pi and ESP32 sides but the Shelly may not support it; verify before committing.
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
- Before the Shelly Plus Plug US can be told to use this broker, the broker must be reachable from the LAN. That requires configuring `mosquitto.conf` to bind a listener to the appropriate interface.
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

**Rationale — authentication.** `allow_anonymous false` enforced from the start, before any client connects. The broker is on a shared campus network with ~200 other devices, MQTT port 1883 is widely scanned, and the cost of adding auth later (after firmware has been written, after the Shelly has been paired, after the dashboard has been built) is substantially higher than the cost of doing it now (one extra config line, one extra command-line flag in client invocations). Username `basilpi` mirrors the Pi user account name for simplicity — this is acceptable because the MQTT password is independent of the Pi user password.

**Verification — three tests in increasing scope.**
1. **Anonymous rejected.** `mosquitto_sub` without credentials → immediate `Connection Refused: not authorised`. Confirms auth is enforced rather than bypassed.
2. **Authenticated loopback.** `mosquitto_sub`/`mosquitto_pub` with credentials, both on the Pi → messages flow correctly. Confirms the credential check accepts valid logins.
3. **External client.** `mosquitto_sub` on the Mac to the Pi's IP with credentials, `mosquitto_pub` on the Pi → messages flow across the LAN. **This is the architecturally meaningful test** — it proves the broker is reachable, authenticated, and operational from a device that is not the Pi.

All three tests passed.

**On the password file ownership puzzle.** During initial setup, the broker repeatedly failed to start with `status=13` (EACCES). Investigation revealed that `mosquitto_passwd -c` creates the file owned by `root:root`, but the broker daemon runs as the `mosquitto` user and needs read access. The naive fix (`chown mosquitto:mosquitto`) makes the broker happy but produces a deprecation warning from `mosquitto_passwd` saying future versions will refuse to load files not owned by root. The correct configuration is **owner `root`, group `mosquitto`, permissions `640`** — root retains write access, the broker reads via group membership, and no one else has access at all. Recorded in `hub/03-broker-config/README.md` so this isn't re-diagnosed next time.

**On the duplicate-key error.** First config attempt re-declared `persistence_location`, which is already set in the default `mosquitto.conf`. Mosquitto refuses to load configurations with duplicate keys and the broker fails to start. Recorded similarly. **General principle:** override settings in `conf.d/` only when the override is actually different from the default. Defaults are not problems to be solved by re-stating.

**On the rotated password.** During the verification session, the MQTT password was inadvertently captured in plain text in chat logs while pasting command output. Out of professional habit, the password was rotated to a fresh value after testing completed. This is recorded not because the exposure was high-risk (the broker is LAN-only, the chat is private, no exploitable surface was opened), but because the pattern is the one to internalize for systems where the stakes are real: when a secret might have leaked, rotate it.

**Implications for the next steps.**
- The Shelly Plus Plug US can now be configured to connect to `10.6.19.139:1883` with the `basilpi` username and the current password. The Shelly's MQTT support is built in; this is a configuration step in its web UI, not new code.
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
### DL-031 — Shelly Plus Plug US paired and joined JSU_DEVICE

**Date:** 2026-06-02 · **Status:** Active. MQTT configuration is the next step (separate DL entry).

**Context.** The Shelly Plus Plug US is the project's grow-light controller per DL-010. It is the project's first commercial, off-the-shelf networked client of the Mosquitto broker. The project's two ESP32 nodes are still on the bench and not yet networked, so the Shelly is the first opportunity to validate the broker's authenticated LAN access path (DL-030) with a real third-party MQTT client rather than command-line test tools.

**Decision.** Shelly Plus Plug US successfully paired, joined JSU_DEVICE WiFi, and named "basilplug." Current IP on JSU_DEVICE: `10.6.17.32`. The plug is approved as the project's grow-light controller and is ready for MQTT configuration.

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

## Maintaining this log

When a new decision is made:

1. Pick the next free `DL-NNN` ID
2. Add it to the index table near the top
3. Append a full entry at the bottom (chronological — newer at the end)
4. If it modifies an earlier entry, update that earlier entry's **Status** line to reference the new one (e.g. "Refined by DL-024"), but never rewrite its content

Old entries stay as they were originally written. The history is the value.
