# Bottom-Watering Calibration Harness (Phase 5 prototype)

A **standalone** firmware, separate from `firmware/integrated/`. It exists to run
supervised bottom/tray-watering sessions: it dispenses the pump in controlled
**doses**, keeps full soil telemetry on the normal MQTT topics (so the Pi
dashboard and SQLite record every reading), and leaves the supplement decision to
you (guarded-manual). It is the isolated prototype whose dose/uptake logic will
graduate into the integrated firmware once bottom-watering is validated.

## Why it exists

The integrated watering algorithm tops-waters in ~5 mL pulses with a 10 s settle
and an 8-pulse no-progress watchdog (DL-049/053). Bottom-watering wicks up over
*minutes*, so that logic would fault at ~40 mL before the plant absorbs anything.
This harness uses a different regime — larger doses, a minutes-long uptake
window, no pulse watchdog — and its own boundaries.

## Safety

- **Leak** (debounced) cuts the pump and latches (`leak_fault`); clears on ACK
  once the pad is dry.
- **ABORT** button cuts the pump and latches (`stopped`); clears on ACK.
- **Reservoir empty** blocks dosing; auto-clears on refill.
- **Per-dose cap** (`MAX_DOSE_ML`) and **session cap** (`MAX_SESSION_ML`) bound
  how much can ever be pumped; water delivered is accounted from real pump-on
  time, so a dose cut short still counts.
- **Uptake lock**: after a dose the firmware refuses new doses for
  `UPTAKE_WAIT_MS` (ACK advances early), enforcing wick-and-observe discipline.

> **Deliberate departure from the integrated firmware:** this harness lets an
> inbound MQTT command (`plant/cmd/dose`) start the pump, because dashboard/CLI
> triggering was wanted for calibration. That is why it is a separate, supervised
> tool. Run it only while you are watching the plant.

## Controls

| Input | Action |
|---|---|
| **DOSE** button (GPIO26) | Start a dose — `INITIAL_DOSE_ML` if the session is empty, else `SUPPLEMENT_ML` |
| **ACK** button (GPIO33) | Clear a latched fault · advance out of the uptake wait early · reset a capped session |
| **ABORT** button (GPIO32) | Immediate pump-off, latched |
| MQTT `plant/cmd/dose` = `<integer mL>` | Dose that many mL (clamped to per-dose and session caps) |
| MQTT `plant/cmd/dose` = `abort` | Stop |

Priming: run a small dose into a cup (e.g. publish `20`) — it is guarded and
counted like any dose, so there is no separate un-capped jog.

## Boundaries (edit at the top of `src/main.cpp`)

`INITIAL_DOSE_ML=150`, `SUPPLEMENT_ML=60`, `MAX_DOSE_ML=200`,
`MAX_SESSION_ML=300`, `UPTAKE_WAIT_MS=30 min`, `PUMP_ML_PER_SEC=1.0` (DL-048).

## Build, flash, run

```bash
cd firmware/bottom-water-calibration
cp src/secrets.h.example src/secrets.h     # or copy your integrated secrets.h
# edit src/secrets.h with real WiFi + MQTT creds
pio run                                     # build-only first: confirm it compiles
pio run -t upload                           # flash (flip the Freenove boot switch PROG->RUN after)
pio device monitor                          # watch the serial dose ledger
```

Trigger a dose over MQTT from the Pi without any dashboard change:

```bash
mosquitto_pub -h 10.6.19.139 -u <user> -P <pass> -t plant/cmd/dose -m 150
```

## What lands where

Soil / leak / float publish on the normal `plant/sensors/*` topics, so the
dashboard shows moisture climbing live and the listener logs it to SQLite —
that time-series, plus the serial dose ledger (mL in, before → after, delta), is
the calibration dataset for tuning the production watering FSM.

BME280 / BH1750 / OLED are intentionally omitted here to keep the harness lean;
the environment panels simply pause for the session.
