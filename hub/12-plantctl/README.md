# plantctl — hub diagnostics CLI

A **read-only** command-line tool that runs the operational checks we otherwise
type by hand. It turns "paste a page of SQL and reason about it" into one
command. It never writes to the DB, never actuates the pump, never changes
firmware or services — safe to run anytime.

## Why

Recurring incidents (probe faults, network drops, watering behaviour, light
schedule) were all diagnosed by running the same queries and applying the same
reasoning. `plantctl` encodes that runbook so a check is `plantctl health`, not a
paragraph of pasted SQL — and it bakes in the lessons that bit us: the DB stores
**UTC** (age math is done in UTC, only display is local), and soil `%` uses the
**same anchors as the firmware**.

## Commands

- `plantctl health` — full sweep: services, WROVER heartbeat + FSM state, soil
  (raw + % on current anchors, trigger proximity), grow light vs schedule, pump
  mode + last pump-on, DB freshness. Prints ✓ / ⚠ / ✗ per line.

_(More commands — `soil`, `light`, `watering`, `incident` — are being added
incrementally.)_

## Install (on the Pi)

```bash
# deploy the script
scp hub/12-plantctl/plantctl.py basilpi@10.6.19.139:/home/basilpi/plant-hub/
# optional: put it on PATH
ssh basilpi@10.6.19.139
ln -sf /home/basilpi/plant-hub/plantctl.py ~/.local/bin/plantctl 2>/dev/null || true
chmod +x /home/basilpi/plant-hub/plantctl.py
# run
python3 /home/basilpi/plant-hub/plantctl.py health
```

## Configuration

Reads env overrides (defaults match the firmware / services); the calibration
constants **must be kept in sync with `firmware/.../config.h` on each
recalibration**:

| var | default | meaning |
|---|---|---|
| `PLANT_DB` | `/home/basilpi/plant-hub/plant.db` | database path |
| `LOCAL_TZ` | `America/Chicago` | display timezone (DB is UTC) |
| `SOIL_RAW_DRY` | `2585` | 0% anchor |
| `SOIL_RAW_WET` | `1700` | 100% anchor (DL-121) |
| `TRIGGER_PCT` | `20` | interim; finalize in pass-2 recal |
| `GROW_ON_HOUR` / `GROW_OFF_HOUR` | `7` / `19` | photoperiod window |
| `GROW_LUX_THRESHOLD` | `30` | lit/dark line |
| `SHELLY_HOST` | `10.6.17.32` | grow-light plug |

> **Planned:** replace the in-file `ANCHORS` with a shared config file that both
> the firmware-adjacent tooling and `plantctl` read, so recalibration updates one
> place. Deferred to the pass-2 recalibration (DL-121).
