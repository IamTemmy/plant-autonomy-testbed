"""
Overview page (DL-095) - the vital glance: status, faults, current readings, and
latest capture. Trend charts live on the Camera / Watering / Power pages. All
render blocks are extracted verbatim from the pre-restructure dashboard.py.
"""

from datetime import datetime
from pathlib import Path

import streamlit as st
from streamlit_autorefresh import st_autorefresh

from dash_common import (
    REFRESH_SECONDS,
    LOCAL_TZ,
    unacked_faults,
    device_online_status,
    render_status_pill,
    render_state_banner,
    latest_reboot,
    reboots_recent,
    format_local,
    _fmt_duration,
    latest_grow_light_state,
    latest_sensor,
    render_card,
    latest_camera,
    load_camera_image,
    current_run,
)


st_autorefresh(interval=REFRESH_SECONDS * 1000, key="autorefresh_overview")


faults_df = unacked_faults()
online = device_online_status("grow-light")
wrover_online = device_online_status("wrover")

if not faults_df.empty:
    overall_pill = render_status_pill(f"{len(faults_df)} active fault(s)", "fault")
elif wrover_online == "offline":
    overall_pill = render_status_pill("WROVER offline", "fault")
elif online == "offline":
    overall_pill = render_status_pill("Shelly offline", "warn")
elif online == "online":
    overall_pill = render_status_pill("All systems nominal", "ok")
else:
    overall_pill = render_status_pill("Status unknown", "unknown")

st.markdown(f"""
<div style="display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap;">
    <div>
        <h1 style="margin: 0;">🌱 Plant Autonomy Testbed</h1>
    </div>
    <div>{overall_pill}</div>
</div>
""", unsafe_allow_html=True)

now_local = datetime.now(LOCAL_TZ).strftime("%Y-%m-%d %H:%M:%S %Z")
st.caption(f"Last refreshed: {now_local}  ·  auto-refresh every {REFRESH_SECONDS}s")

render_state_banner()

_reboot = latest_reboot("wrover")
if _reboot is not None:
    _msg = (f"↻ Last WROVER reboot: {format_local(_reboot['ts'])} "
            f"(was up {_fmt_duration(_reboot['prev_uptime_s'])} before)")
    _n24 = reboots_recent("wrover", 24)
    if _n24 >= 2:
        st.warning(f"{_msg} &middot; {_n24} reboots in the last 24h — check power stability")
    else:
        st.caption(_msg)

st.markdown("## Active faults")

if faults_df.empty:
    st.markdown('<div class="fault-banner-ok">✓ No unacknowledged faults</div>',
                unsafe_allow_html=True)
else:
    st.dataframe(faults_df, hide_index=True, use_container_width=True)

st.markdown("## Listener run")

run = current_run()
if run is None:
    st.warning("No active listener run.")
else:
    c1, c2 = st.columns(2)
    with c1:
        st.markdown(f"**Run ID:** `{run['run_id']}`")
    with c2:
        st.markdown(f"**Started:** {format_local(run['started_ts'])}")


st.markdown("## Current state")

c1, c2, c3, c4 = st.columns(4)

gl_state = latest_grow_light_state()
with c1:
    if gl_state is None:
        render_card("Grow light", "—", "no data", "unknown")
    else:
        on = gl_state["action"] == "on"
        render_card("Grow light", "ON" if on else "OFF",
                    f"via {gl_state['source']}", "ok" if on else "neutral")

power = latest_sensor("power")
with c2:
    if power is None:
        render_card("Power", "—", "no data", "unknown")
    else:
        render_card("Power", f"{power['value']:.1f} W", "real-time", "neutral")

voltage = latest_sensor("voltage")
with c3:
    if voltage is None:
        render_card("Voltage", "—", "no data", "unknown")
    else:
        v = voltage["value"]
        status = "ok" if 114 <= v <= 126 else "warn"
        render_card("Voltage", f"{v:.1f} V", "Shelly circuit", status)

with c4:
    if online == "online":
        render_card("Shelly", "Online", "Connected", "ok")
    elif online == "offline":
        render_card("Shelly", "Offline", "Disconnected", "fault")
    else:
        render_card("Shelly", "—", "no data", "unknown")

st.markdown("## Plant camera")

_cam = latest_camera()
if _cam is None:
    st.info("No camera captures recorded yet.")
else:
    _cam_ts, _cam_path = _cam[0], _cam[1]
    if _cam_path and Path(_cam_path).exists():
        try:
            st.image(load_camera_image(_cam_path), use_container_width=True)
        except Exception:
            st.image(_cam_path, use_container_width=True)
        st.caption(f"Latest capture: {format_local(_cam_ts)}")
    else:
        st.warning("Latest image file is not available on disk.")
        st.caption(f"Recorded {format_local(_cam_ts)}")

st.markdown("## Plant environment")
st.caption("Live readings from the ESP32 WROVER; cards fill in as each sensor comes online.")

# Pull latest BME280 metrics published by the WROVER (device='bme280').
air_temp = latest_sensor("temperature", "bme280")
air_hum  = latest_sensor("humidity", "bme280")
air_pres = latest_sensor("pressure", "bme280")
light    = latest_sensor("lux", "bh1750")
soil     = latest_sensor("moisture", "soil")
reservoir = latest_sensor("reservoir_empty", "float")
leak      = latest_sensor("leak_detected", "leak")


def _fmt(reading, digits=1):
    if not reading:
        return "—", "unknown"
    return f"{reading['value']:.{digits}f} {reading['unit']}", "ok"


def _fmt_state(reading, true_label, false_label, true_status, false_status):
    # Boolean-style sensor: value 1.0 => true_label, 0.0 => false_label.
    if not reading:
        return "—", "unknown"
    is_true = reading["value"] >= 0.5
    return (true_label, true_status) if is_true else (false_label, false_status)


cols = st.columns(3)

# Live cards (BME280)
temp_val, temp_status = _fmt(air_temp)
hum_val, hum_status   = _fmt(air_hum)
pres_val, pres_status   = _fmt(air_pres, digits=0)
light_val, light_status = _fmt(light, digits=0)
soil_val, soil_status   = _fmt(soil, digits=0)
res_val, res_status     = _fmt_state(reservoir, "Empty", "OK", "warn", "ok")
leak_val, leak_status   = _fmt_state(leak, "Leak!", "Dry", "fault", "ok")

live_cards = [
    ("Air temperature", temp_val,  "BME280", temp_status),
    ("Humidity",        hum_val,   "BME280", hum_status),
    ("Pressure",        pres_val,  "BME280", pres_status),
    ("Light level",     light_val, "BH1750", light_status),
    ("Soil moisture",   soil_val,  "Capacitive sensor", soil_status),
    ("Reservoir level", res_val,   "Float switch", res_status),
    ("Leak sensor",     leak_val,  "Conductive strip", leak_status),
]

# When the WROVER is offline these readings are frozen, not live — show them
# greyed and flagged stale rather than implying they're current (DL-056).
wrover_offline = device_online_status("wrover") == "offline"
_stale_candidates = [r["ts"] for r in (air_temp, air_hum, air_pres, light, soil, reservoir, leak)
                     if r and r.get("ts")]
_stale_as_of = format_local(max(_stale_candidates)) if _stale_candidates else None

for i, (label, value, meta, status) in enumerate(live_cards):
    if wrover_offline:
        status = "unknown"
        meta = f"{meta} · stale" + (f" (as of {_stale_as_of})" if _stale_as_of else "")
    with cols[i % 3]:
        render_card(label, value, meta, status)
