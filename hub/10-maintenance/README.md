# hub/10-maintenance — database retention

`plant.db` grows continuously (per-minute Shelly power telemetry, ~30s WROVER
sensor pushes, and every MQTT message archived). Query speed is bounded by the
time indexes (see DL-067), but the file and the SD card are not. `retention.py`
trims the high-frequency tables to a rolling window on a nightly timer.

## What it prunes

| Table | Default window | Env override | Notes |
|-------|----------------|--------------|-------|
| `mqtt_messages` | 7 days | `RETENTION_MQTT_DAYS` | Raw archive; never read historically — pruned hard |
| `system_status` | 14 days | `RETENTION_STATUS_DAYS` | Alerter/dashboard only look back 24h |
| `sensor_readings` | 30 days | `RETENTION_SENSOR_DAYS` | Dashboard maxes at 7d; this is the dominant table — lower to 14 to ~halve steady-state size |

`runs`, `actuator_events`, and `fault_events` are sparse and valuable and are
**kept forever** (not listed in the script's `TABLES`).

Other env knobs: `RETENTION_BATCH` (rows per delete batch, default 5000),
`PLANT_DB` (path, default `/home/basilpi/plant-hub/plant.db`). Set a window to
`0` to disable pruning that table.

## Design

- **Sargable deletes** — `ts < <cutoff>` with the cutoff built in the stored ISO
  format, so each delete range-scans the table's `ts` index.
- **Batched** — commit per batch and briefly yield between batches, so a large
  first-run backlog can't hold a long write lock or balloon the WAL against the
  live listener.
- **WAL + busy_timeout** — waits for the listener's writes instead of erroring,
  and never blocks its reads.
- Ends with `wal_checkpoint(TRUNCATE)` so the WAL file doesn't sit large after a
  big delete. The DB file itself plateaus via free-page reuse (no VACUUM needed);
  run `VACUUM` manually off-hours only if you want to physically shrink it.

## Install

```bash
scp retention.py basilpi@<pi>:/home/basilpi/plant-hub/retention.py
scp plant-retention.service plant-retention.timer basilpi@<pi>:/tmp/
ssh -t basilpi@<pi> "sudo cp /tmp/plant-retention.{service,timer} /etc/systemd/system/ \
  && sudo systemctl daemon-reload \
  && sudo systemctl enable --now plant-retention.timer"
```

First run by hand to clear the initial backlog and watch it work:

```bash
ssh -t basilpi@<pi> "sudo systemctl start plant-retention.service \
  && journalctl -u plant-retention.service --no-pager | tail -8"
systemctl list-timers plant-retention.timer   # confirm next scheduled run
```
