# Bottom-Watering Control Loop (Phase 5 prototype)

> ⚠️ **SUPERSEDED (DL-147–151, 2026-08-19).** This prototype's watering loop has
> graduated into the production `firmware/integrated/` firmware (P1-8), which now
> runs the same hardened DL-142 plateau-gated dosing **plus** full I²C sensing
> (BME280 temp/RH/pressure + BH1750 lux) that this standalone harness never had.
> The integrated build was flash-validated (DL-149) and armed on the plant
> (DL-151); it is the firmware the board runs. This directory is kept for history
> and as the calibration rig of record — **do not flash it to the production plant**
> (it reads no I²C, so lux/temp go dark and the DL-137/139 workarounds would be
> needed again). Use it only for isolated bench calibration if ever required.

A **standalone** firmware, separate from `firmware/integrated/`. It runs the
**autonomous** bottom/tray-watering loop from DL-104–107: trigger at a low
moisture %, dose a measured volume, wait a long settle + plateau, evaluate the
trend, and decide (done / supplement / stall). Full soil telemetry stays on the
normal MQTT topics, so the Pi dashboard and SQLite record every reading and state
transition. This is the prototype whose loop graduates into the integrated
firmware once validated; prototyping here loses no data (same topics, same DB).

> **Run supervised on first cycles.** This firmware drives the pump on its own.

## The loop

- **Trigger** — in `monitor`, moisture ≤ **20%** starts a session (or force it:
  DOSE button / MQTT `start`).
- **Dose 1** — **200 mL**.
- **Settle** — pump off; wait **3 h minimum**, then until the reading **plateaus**
  (change ≤1% over 30 min). The mid-column probe lags a bottom fill by *hours*
  (DL-104), so judgement never rests on an early reading.
- **Evaluate** (vs the reading before the dose):
  - **≥ 85%** → done.
  - **rose ≥ 7%**, < 85%, under cap → **supplement 100 mL**, settle again.
  - **rose ≥ 7%**, at the **400 mL** session cap → stop, "capped".
  - **rose < 7%** → *stall* (tray likely still holding water) → alert, **grace**.
- **Grace** (one per session) — pump off **1.5 h**, then vs the stall reading:
  rose ≥ 7% → recovered, resume evaluate; else → stop, "failed, needs help".

Worst case ≈ 3 h settle + 1.5 h grace before a failed session gives up.

## Why the probe is both gauge and tray sensor

A dose that does **not** raise moisture after a long settle means the water is
sitting in the tray, not absorbing — so the ≥7% "did it rise?" test is also the
overflow guard. Supplements only fire after confirmed absorption, which is what
makes chasing an aggressive 85% target safe without a physical tray sensor.

## Safety

- **Leak** (debounced) cuts the pump and latches; clears on ACK when dry.
- **ABORT** cuts + latches; **reservoir empty** stops an active session / blocks
  triggering.
- **Per-dose cap** 200 mL, **session cap** 400 mL (runaway backstop; the
  absorption gate is the real overflow protection). Water is accounted from real
  pump-on time, so a dose cut short still counts.

## Controls

| Input | Action |
|---|---|
| **DOSE** button (GPIO26) | Force-start a session now (ignores the 20% trigger) |
| **ACK** button (GPIO33) | Clear a latched stop · force-advance a settle/grace wait (testing) |
| **ABORT** button (GPIO32) | Immediate pump-off, latched |
| MQTT `plant/cmd/dose` = `start` | Force-start a session |
| MQTT `plant/cmd/dose` = `abort` | Stop |

State + reason are published (retained) to `plant/state/wrover` and printed to
serial. Push notifications (ntfy) are **not** wired here yet — for supervised
runs, serial + the dashboard state are the alert surface; a small Pi-listener
rule on the state `reason` can forward to ntfy later.

## Parameters (top of `src/main.cpp`, DL-107)

`TRIGGER_PCT=20`, `TARGET_PCT=85`, `DOSE1_ML=200`, `SUPPLEMENT_ML=100`,
`ABSORB_RISE_PCT=7`, `SETTLE_MIN=3h`, `PLATEAU_WINDOW=30m`/`SLOPE=1%`,
`GRACE=1.5h`, `SESSION_CAP_ML=400`. Soil anchors 2585/2250 (DL-106),
flow 1.0 mL/s (DL-048). Plateau constants are first guesses — tune from the first
cycle's curve.

## Build, flash, run

```bash
cd firmware/bottom-water-calibration
cp ../integrated/src/secrets.h src/secrets.h     # or fill from the .example
pio run                                           # build-only first
pio run -t upload
pio device monitor
```

Force a test cycle without waiting for 20%:

```bash
mosquitto_pub -h 10.6.19.139 -u <user> -P <pass> -t plant/cmd/dose -m start
```

BME280 / BH1750 / OLED are intentionally omitted to keep the harness lean; the
environment dashboard panels pause during a session.
