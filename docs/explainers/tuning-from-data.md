# Tuning from data: how thresholds are set in this project

Several parts of this system depend on a threshold — the lux level that means
"the grow light is on" (DL-063), the soil-moisture jump that means "someone
watered the plant" (DL-064), the silence timeout that means "a device hung"
(DL-059). None of these numbers were guessed. Each was read off the system's own
logged history. This note documents that method, because *how* a threshold was
derived matters as much as the value.

## The principle: instrument first, then tune from the distribution

A capacitive soil probe is noisy. A lux sensor reads a blurple grow light far
lower than a daylight-calibrated table would predict. Office lighting in this
particular lab measures ~7–18 lux at the sensor, not the 300–500 lux a generic
reference would claim (DL-021). So a threshold copied from a datasheet or a
textbook is usually wrong *for this rig in this room*.

The reliable approach is to let the system run, log everything, and then look at
the real data to separate two populations: the **signal** we want to detect and
the **noise floor** we must stay above. The threshold goes in the stable gap
between them, with enough margin to survive the rig being bumped or the room
changing. The goal is not the "correct" number in the abstract — it's a number
that cleanly separates the two populations *as they actually occur here*.

## The access pattern: read-only queries against the hub database

The hub logs every sensor reading and state change to SQLite
(`/home/basilpi/plant-hub/plant.db`). To characterise behaviour, query it
read-only over SSH — never edit the live database:

```
ssh basilpi@<hub> "sqlite3 -header -column /home/basilpi/plant-hub/plant.db \
  \"<query>\""
```

The workhorse query is an **hourly aggregate** that exposes the daily pattern
and, crucially, the per-hour spread (the `swing` = max − min), which is where the
noise floor reveals itself:

```sql
SELECT substr(ts,1,13) AS hour_utc,
       COUNT(*) AS n,
       ROUND(MIN(value),1)  AS min,
       ROUND(AVG(value),1)  AS avg,
       ROUND(MAX(value),1)  AS max,
       ROUND(MAX(value)-MIN(value),1) AS swing
FROM sensor_readings
WHERE sensor = '<sensor>'
GROUP BY hour_utc
ORDER BY hour_utc DESC
LIMIT 72;
```

Reading it: quiet hours show the **noise floor** (how much the reading wanders
when nothing is happening); active hours show the **signal** (how far a real
event moves it). The gap between those is the room available for a threshold.
Timestamps are stored UTC; the local photoperiod or schedule has to be mapped
through the offset when interpreting the pattern.

## Worked example 1 — grow-light lux threshold (DL-063)

The hourly lux query showed OFF windows at ~0 lux every night and ON windows at
~40–55 lux, with the transitions landing exactly on the 07:00/19:00 schedule.
A separate manual reading gave room-lights-only ≈ 7–18 lux at the sensor. Two
clean populations: ambient (≤ ~18) and grow light (~35–47, depending on the
light's height). A threshold of **30 lux** sits in the gap with roughly equal
margin on each side. Because the rig's geometry drifts as the light and sensor
are handled, the value is env-overridable and is being validated over several
days of real-handling data before it's locked.

## Worked example 2 — external-watering soil threshold (DL-064)

The hourly *soil* query showed quiet-hour swings of ~2–5% (the probe's noise
floor) and a slow downward drift as the plant dried — moisture only ever rises
when water is added. A real watering jumps far more than the noise. Setting the
"someone watered it" threshold at **15%** (≈ 3× the noise floor) means only a
genuine pour clears the bar while routine jitter never does. The same query also
made the test artifacts obvious (the probe-pull episode showed up as a single
hour with 100% swing), which is itself a reason to look at the data rather than
trust a single reading.

## A correctness note: comparing timestamps in windowed queries (DL-065)

When a query filters "the last N hours/minutes," compare timestamps numerically,
not as strings. The database stores UTC ISO with a `T` separator and `Z` suffix
(`2026-06-12T13:45:00Z`), while SQLite's `datetime('now', …)` returns a
space-separated string with no `Z`. A direct `ts >= datetime('now','-24 hours')`
is a *string* comparison, and at the separator position `T` sorts after a space —
so any row sharing the threshold's calendar date is wrongly included regardless
of its time. Normalise first:

```sql
julianday(replace(replace(ts,'T',' '),'Z','')) >= julianday('now', ?)
```

This converts both sides to numeric Julian days, so the window is exact. The
data-derived analysis queries above are read-only and forgiving of this, but any
query whose *result drives logic* (alert counts, history windows) must use the
normalised form.

## The discipline in one line

Instrument everything, read the noise floor and the signal off the real logged
distribution, set the threshold in the gap with margin, make it env-overridable,
and validate it over time — then write down how the number was derived.
