# hub/11-shelly-monitor — Shelly uptime/RSSI instrumentation

The grow-light Shelly Plug (`10.6.17.32`) sits on the flaky campus WiFi
(DL-028). The DL-066 incident raised a question the data couldn't answer: is the
plug **rebooting** (device restarting, defaults to ON) or just **dropping off
WiFi** (stays powered, schedule keeps running locally)? A telemetry gap looks
identical either way.

`shelly_monitor.py` resolves it by sampling the Shelly's own `uptime` + `rssi`
once a minute over a direct HTTP RPC call (`Shelly.GetStatus`):

- **uptime resets to ~0** → the device rebooted.
- **a gap, but uptime keeps climbing afterward** → just a WiFi dropout.
- **rssi time-series** → correlate dropouts/reboots with signal strength, as
  evidence for the campus-network conversation (DL-028).

## What it logs

- `sensor_readings` (device=`grow-light`): `uptime` (s) and `rssi` (dBm), one each
  per minute. Covered by the 30-day `sensor_readings` retention window (DL-068).
- `system_status` (device=`grow-light`, metric=`reboot`): a marker when uptime goes
  backwards, mirroring the WROVER reboot detection (DL-060); `value` = uptime
  before the reboot.

Shelly reboots are recorded for analysis and the dashboard but **not** wired
into the alerter on purpose — the plug's WiFi is known-flaky and the grow-light
verification alert (DL-063) already pages on any real functional impact.

## Design

- One `Shelly.GetStatus` call per minute (atomic snapshot of sys + wifi).
- Stateless: previous uptime is read from the DB, so reboot detection survives
  poller restarts. Unreachable poll → logged and skipped; the series gap is the
  dropout record.
- WAL + busy_timeout, like every other hub writer.

Env knobs: `SHELLY_HOST` (default `10.6.17.32`), `SHELLY_POLL_TIMEOUT` (default
4 s), `PLANT_DB`.

## Install

```bash
scp shelly_monitor.py basilpi@<pi>:/home/basilpi/plant-hub/shelly_monitor.py
scp plant-shelly-monitor.service plant-shelly-monitor.timer basilpi@<pi>:/tmp/
ssh -t basilpi@<pi> "sudo cp /tmp/plant-shelly-monitor.{service,timer} /etc/systemd/system/ \
  && sudo systemctl daemon-reload \
  && sudo systemctl enable --now plant-shelly-monitor.timer"
```

Verify one sample by hand, then let the timer run:

```bash
ssh -t basilpi@<pi> "sudo systemctl start plant-shelly-monitor.service \
  && journalctl -u plant-shelly-monitor.service --no-pager | tail -3"
```

## Reading the data (after a day or so)

```sql
-- reboots vs. dropouts: reboot markers, plus the uptime series around any gap
SELECT ts, value AS uptime_before FROM system_status
  WHERE device='grow-light' AND metric='reboot' ORDER BY ts DESC;

-- RSSI distribution by local hour (UTC-5), to correlate with dropout timing
SELECT strftime('%H', replace(replace(ts,'T',' '),'Z',''), '-5 hours') AS local_hr,
       ROUND(AVG(value),1) avg_rssi, ROUND(MIN(value),1) worst_rssi
  FROM sensor_readings WHERE device='grow-light' AND sensor='rssi' GROUP BY local_hr;
```
