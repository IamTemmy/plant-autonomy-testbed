# 04 — Hub listener: MQTT to SQLite

Python service running on the Pi that subscribes to `plant/#` on the local
Mosquitto broker, archives every message in `mqtt_messages`, and projects
structured fields into specialized tables (`sensor_readings`,
`actuator_events`, `system_status`, `fault_events`).

See [DL-035](../../docs/decision-log.md) for design rationale.

## Files

| File | Purpose |
|---|---|
| `schema.sql` | SQLite schema; idempotent (safe to re-apply) |
| `listener.py` | The listener service; subscribes, parses, persists |
| `README.md` | This file |

## Database tables

| Table | Contains |
|---|---|
| `runs` | Registry of listener sessions: run_id, started_ts, ended_ts, phase, description, notes |
| `mqtt_messages` | Raw archive of every MQTT message received (source of truth) |
| `sensor_readings` | Continuous measurements; EAV pattern (sensor + value + unit) |
| `actuator_events` | Discrete state changes (on/off/dose/PWM); deduplicated |
| `system_status` | Online/offline + health metrics (uptime, RAM, broker connection) |
| `fault_events` | Warnings, critical issues, acknowledgement tracking |

Each row in the bottom five tables references a `message_id` (foreign key to
`mqtt_messages.id`) and a `run_id` (foreign key to `runs.run_id`) for full
provenance.

## Setup procedure on the Pi

These steps were used during DL-035. Re-running them on a fresh Pi reproduces
the listener environment.

### 1. Install sqlite3 CLI

```text
sudo apt update && sudo apt install -y sqlite3
```

### 2. Create the project directory and Python virtualenv

```text
mkdir -p ~/plant-hub && cd ~/plant-hub
python3 -m venv venv
source venv/bin/activate
pip install paho-mqtt
```

### 3. Copy `schema.sql` and `listener.py` from this repo to `~/plant-hub/`

`scp` from your developer machine, or `git clone` the repo onto the Pi and
symlink. Either works.

### 4. Initialize the database

```text
cd ~/plant-hub
sqlite3 plant.db < schema.sql
sqlite3 plant.db ".tables"
```

Should list all six tables: `actuator_events`, `fault_events`, `mqtt_messages`,
`runs`, `sensor_readings`, `system_status`.

## Running the listener manually

The listener reads MQTT credentials from environment variables and the run
identifier from a separate optional variable.

```text
source venv/bin/activate
export MQTT_USER=basilmqtt
read -s -p "MQTT password: " MQTT_PASS && export MQTT_PASS && echo
export RUN_PHASE=phase3_hub_bringup   # optional
python listener.py
```

Expected output:

```text
... [INFO] Run identifier: phase3_hub_bringup_YYYYMMDD_HHMMSS (phase=phase3_hub_bringup)
... [INFO] Connecting to localhost:1883 as basilmqtt
... [INFO] Connected to broker; subscribing to plant/#
```

Then silence — the listener is now absorbing messages. Ctrl+C for graceful
shutdown (updates `runs.ended_ts` before exiting).

## Run identifier rules

The listener resolves `run_id` at startup:

1. If `RUN_ID` is set, use it verbatim.
2. Else if `RUN_PHASE` is set, generate `<phase>_<YYYYMMDD>_<HHMMSS>` in UTC.
3. Else generate `auto_<YYYYMMDD>_<HHMMSS>`.

Use cases:

- `RUN_PHASE=phase3_hub_bringup` for ad-hoc bench sessions
- `RUN_ID=basil_longrun_week1` for named long-term runs
- Neither set during a quick test → defaults to `auto_*`

## Example queries

Most recent sensor readings:

```text
sqlite3 -header -column plant.db "SELECT ts, device, sensor, value, unit FROM sensor_readings ORDER BY id DESC LIMIT 10;"
```

Grow light state-change history:

```text
sqlite3 -header -column plant.db "SELECT ts, actuator, action, source FROM actuator_events WHERE actuator = 'grow-light' ORDER BY ts;"
```

Times the grow light was turned on this week:

```text
sqlite3 plant.db "SELECT COUNT(*) FROM actuator_events WHERE actuator = 'grow-light' AND action = 'on' AND ts >= datetime('now', '-7 days');"
```

## What this layer does not yet handle

- **Not running as a systemd service.** Manual start only. Next step (separate DL).
- **No Streamlit dashboard.** SQL queries only. Coming next.
- **No retention policy.** Database grows unbounded. Acceptable for project scale; revisit at year scale.
- **No ESP32 publishers yet.** The listener subscribes to `plant/#`, so it will pick up future ESP32 traffic automatically without code changes.
