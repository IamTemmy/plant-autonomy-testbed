# 08 — Grow light (Shelly scheduler)

Grow-light control for the testbed. A Shelly Plug US (Gen4) switches the grow
light and runs a fixed daily photoperiod — 07:00 ON → 19:00 OFF (12 h light /
12 h dark) — on the Shelly's own built-in scheduler, configured via its RPC
API. Running the schedule on the device means the light keeps time even if the
Pi, the WROVER, or the network is down. Lighting is intentionally independent
of the watering state machine; the BH1750 light sensor stays report-only,
verifying actual light rather than controlling it.

See [DL-054](../../docs/decision-log.md) for the design rationale (device-side
vs. Pi-side scheduling, the full-photoperiod choice), and DL-010 / DL-030 /
DL-031 for the Shelly selection and MQTT integration history.

## Files

| File | Purpose |
|---|---|
| `set-schedule.sh` | Sets the Shelly's daily ON/OFF schedule via RPC (`Schedule.Create`) |
| `README.md` | This file |

## Usage

Run `./set-schedule.sh <SHELLY_IP>`. The script clears any existing schedules
and creates two — `Switch.Set` ON at 07:00 and OFF at 19:00, daily — then
prints `Schedule.List` to confirm. Re-run to change the times (edit `ON_HOUR` /
`OFF_HOUR` first); it is idempotent.

**Prerequisites**

- The Shelly's IP (DHCP-assigned; verify before running — see DL-054). Last known: `10.6.17.32`.
- The Shelly's timezone/location must be set (America/Chicago), or the schedule fires at the wrong local time.
- RPC auth is disabled on the device (`auth_en: false`), so no credentials are needed.

## Notes

The device is a Shelly Plug US **Gen4** (model S4PL-00116US), confirmed via
`Shelly.GetDeviceInfo`. The grow light's relay state and power telemetry are
published over MQTT and surfaced on the dashboard (`hub/06-dashboard/`); this
folder covers only the scheduling control path.
