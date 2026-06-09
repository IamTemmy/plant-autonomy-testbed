"""
Plant Autonomy Testbed — Streamlit dashboard (polished).

Read-only view of the SQLite database populated by the listener service.
Light cream theme, green primary, semantic status colors.

Run with:
    streamlit run dashboard.py --server.address 0.0.0.0 --server.port 8501
"""

import sqlite3
from datetime import datetime, timezone
from pathlib import Path
from pathlib import Path
from zoneinfo import ZoneInfo

import pandas as pd
import plotly.graph_objects as go
import streamlit as st
from streamlit_autorefresh import st_autorefresh

DB_PATH = Path(__file__).parent / "plant.db"
REFRESH_SECONDS = 10
LOCAL_TZ = ZoneInfo("America/Chicago")

COLORS = {
    "primary": "#2D6A4F",
    "background": "#FAF7F0",
    "card_bg": "#FFFFFF",
    "card_border": "#E8E5DC",
    "text": "#1A1A1A",
    "text_muted": "#6C757D",
    "status_ok": "#2D6A4F",
    "status_warn": "#D08C00",
    "status_fault": "#B23838",
    "status_unknown": "#9CA3AF",
}

# ----------------------------------------------------------------------
# Page setup + custom CSS
# ----------------------------------------------------------------------

st.set_page_config(
    page_title="Plant Autonomy Testbed",
    page_icon="🌱",
    layout="wide",
    initial_sidebar_state="collapsed",
)
st_autorefresh(interval=REFRESH_SECONDS * 1000, key="autorefresh")

st.markdown("""
<style>
.block-container {
    max-width: 1200px;
    padding-top: 1.5rem;
    padding-bottom: 3rem;
}

.metric-card {
    background: #FFFFFF;
    border: 1px solid #E8E5DC;
    border-radius: 12px;
    padding: 1.25rem 1.5rem;
    box-shadow: 0 1px 3px rgba(0, 0, 0, 0.04);
    height: 100%;
}

.metric-card-label {
    color: #6C757D;
    font-size: 0.8rem;
    font-weight: 600;
    margin-bottom: 0.5rem;
    text-transform: uppercase;
    letter-spacing: 0.05em;
}

.metric-card-value {
    font-size: 1.875rem;
    font-weight: 700;
    margin-bottom: 0.25rem;
    line-height: 1.1;
}

.metric-card-meta {
    color: #6C757D;
    font-size: 0.85rem;
}

.status-pill {
    display: inline-flex;
    align-items: center;
    gap: 0.5rem;
    padding: 0.35rem 0.85rem;
    border-radius: 999px;
    font-size: 0.85rem;
    font-weight: 600;
    float: right;
}

.status-pill::before {
    content: "";
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: currentColor;
}

.status-ok { color: #2D6A4F; background: #D8F3DC; }
.status-warn { color: #D08C00; background: #FFF4D6; }
.status-fault { color: #B23838; background: #FCE4E4; }
.status-unknown { color: #6C757D; background: #F1F1F1; }

.fault-banner-ok {
    background: #D8F3DC;
    border-left: 4px solid #2D6A4F;
    padding: 1rem 1.25rem;
    border-radius: 8px;
    color: #2D6A4F;
    font-weight: 500;
}

h1 { color: #2D6A4F; font-weight: 700; margin-bottom: 0.25rem !important; }

h2 {
    color: #1A1A1A;
    font-weight: 600;
    font-size: 1.25rem !important;
    border-bottom: 1px solid #E8E5DC;
    padding-bottom: 0.4rem;
    margin-top: 2rem !important;
    margin-bottom: 1rem !important;
}

@media (max-width: 768px) {
    .metric-card { padding: 1rem; }
    .metric-card-value { font-size: 1.5rem; }
    h1 { font-size: 1.5rem; }
    .status-pill { float: none; margin-top: 0.5rem; }
}
</style>
""", unsafe_allow_html=True)


# ----------------------------------------------------------------------
# Database access
# ----------------------------------------------------------------------

def get_conn() -> sqlite3.Connection:
    return sqlite3.connect(f"file:{DB_PATH}?mode=ro", uri=True)


def query_df(sql: str, params: tuple = ()) -> pd.DataFrame:
    with get_conn() as conn:
        return pd.read_sql_query(sql, conn, params=params)


def latest_sensor(sensor: str, device: str = "grow-light"):
    df = query_df(
        """SELECT ts, value, unit FROM sensor_readings
           WHERE sensor = ? AND device = ?
           ORDER BY id DESC LIMIT 1""",
        (sensor, device),
    )
    return df.iloc[0].to_dict() if not df.empty else None


def latest_grow_light_state():
    df = query_df(
        """SELECT ts, action, source FROM actuator_events
           WHERE actuator = 'grow-light'
           ORDER BY id DESC LIMIT 1""",
    )
    return df.iloc[0].to_dict() if not df.empty else None


def current_run():
    df = query_df(
        """SELECT run_id, started_ts, phase FROM runs
           WHERE ended_ts IS NULL
           ORDER BY started_ts DESC LIMIT 1""",
    )
    return df.iloc[0].to_dict() if not df.empty else None


def device_online_status(device: str = "grow-light") -> str:
    df = query_df(
        """SELECT status FROM system_status
           WHERE device = ? AND metric IS NULL
           ORDER BY id DESC LIMIT 1""",
        (device,),
    )
    return df.iloc[0]["status"] if not df.empty else "unknown"


def latest_fsm_state():
    df = query_df(
        """SELECT ts, status, value FROM system_status
           WHERE device = 'wrover' AND metric = 'fsm_state'
           ORDER BY id DESC LIMIT 1"""
    )
    if df.empty:
        return None
    return {"ts": df.iloc[0]["ts"], "state": df.iloc[0]["status"],
            "daily_pump_ms": df.iloc[0]["value"]}


def latest_wrover_pump() -> str:
    df = query_df(
        """SELECT status FROM system_status
           WHERE device = 'wrover' AND metric = 'pump'
           ORDER BY id DESC LIMIT 1"""
    )
    return df.iloc[0]["status"] if not df.empty else "off"


def power_history(hours: int) -> pd.DataFrame:
    return query_df(
        """SELECT ts, value AS watts FROM sensor_readings
           WHERE sensor = 'power' AND device = 'grow-light'
             AND ts >= datetime('now', ?)
           ORDER BY ts""",
        (f"-{hours} hours",),
    )


def recent_actuator_events(limit: int = 10) -> pd.DataFrame:
    return query_df(
        """SELECT ts, actuator, action, source FROM actuator_events
           ORDER BY id DESC LIMIT ?""",
        (limit,),
    )


def unacked_faults() -> pd.DataFrame:
    return query_df(
        """SELECT ts, device, fault_type, severity, message FROM fault_events
           WHERE acknowledged = 0
           ORDER BY id DESC""",
    )

def format_local(ts_str: str) -> str:
    """Convert an ISO 8601 UTC timestamp from the DB to America/Chicago display."""
    if not ts_str:
        return ""
    ts = pd.to_datetime(ts_str, utc=True)
    return ts.tz_convert(LOCAL_TZ).strftime("%Y-%m-%d %H:%M:%S %Z")

# ----------------------------------------------------------------------
# Custom rendering helpers
# ----------------------------------------------------------------------

def render_card(label: str, value: str, meta: str = "", status: str = "neutral"):
    color_map = {
        "ok": COLORS["status_ok"],
        "warn": COLORS["status_warn"],
        "fault": COLORS["status_fault"],
        "unknown": COLORS["status_unknown"],
        "neutral": COLORS["text"],
    }
    color = color_map.get(status, COLORS["text"])
    meta_html = f'<div class="metric-card-meta">{meta}</div>' if meta else ""
    st.markdown(f"""
    <div class="metric-card">
        <div class="metric-card-label">{label}</div>
        <div class="metric-card-value" style="color: {color};">{value}</div>
        {meta_html}
    </div>
    """, unsafe_allow_html=True)


def render_status_pill(label: str, status: str) -> str:
    return f'<div class="status-pill status-{status}">{label}</div>'


# Maps each FSM state to (display label, status tier, one-line description).
STATE_DISPLAY = {
    "monitoring":      ("Monitoring",          "ok",    "Watching soil moisture"),
    "watering":        ("Watering",            "ok",    "Pump pulsing toward target"),
    "manual":          ("Manual watering",     "ok",    "Manual override running"),
    "reservoir_empty": ("Reservoir empty",     "warn",  "Refill needed \u2014 watering blocked"),
    "daily_limit":     ("Daily limit reached", "warn",  "Paused until the daily window resets"),
    "leak_fault":      ("Leak fault",          "fault", "Leak detected \u2014 pump stopped, needs ACK"),
    "stopped":         ("Emergency stop",      "fault", "Halted \u2014 needs ACK"),
    "watering_fault":  ("Watering fault",      "fault", "Soil not responding \u2014 pump stopped, needs ACK"),
}

_BANNER_PALETTE = {
    "ok":      ("#D8F3DC", "#2D6A4F", "#2D6A4F"),
    "warn":    ("#FFF4D6", "#D08C00", "#D08C00"),
    "fault":   ("#FCE4E4", "#B23838", "#B23838"),
    "unknown": ("#F1F1F1", "#9CA3AF", "#6C757D"),
}


def render_state_banner():
    fsm = latest_fsm_state()
    if device_online_status("wrover") == "offline":
        bg, bd, col = _BANNER_PALETTE["unknown"]
        label = "WROVER offline"
        if fsm and fsm["state"]:
            last = STATE_DISPLAY.get(fsm["state"], (fsm["state"],))[0]
            meta = (f"No telemetry &middot; last state \u201c{last}\u201d "
                    f"at {format_local(fsm['ts'])}")
        else:
            meta = "No telemetry from the WROVER"
    elif not fsm or not fsm["state"]:
        bg, bd, col = _BANNER_PALETTE["unknown"]
        label, meta = "No state received yet", "Waiting for the WROVER to report"
    else:
        label, tier, sub = STATE_DISPLAY.get(
            fsm["state"], (fsm["state"], "unknown", ""))
        bg, bd, col = _BANNER_PALETTE.get(tier, _BANNER_PALETTE["unknown"])
        pump = latest_wrover_pump()
        daily_ml = int(fsm["daily_pump_ms"] / 1000) if fsm["daily_pump_ms"] is not None else 0
        meta = (f"{sub} &middot; Pump: {'ON' if pump == 'on' else 'off'} "
                f"&middot; Daily watering: {daily_ml} mL &middot; "
                f"Updated {format_local(fsm['ts'])}")
    st.markdown(f"""
    <div style="background:{bg}; border-left:5px solid {bd};
                padding:1rem 1.25rem; border-radius:8px; margin-bottom:1.5rem;">
        <div style="font-size:0.75rem; text-transform:uppercase;
                    letter-spacing:0.06em; color:{col}; opacity:0.85;">
            Watering system</div>
        <div style="font-size:1.6rem; font-weight:700; color:{col};">{label}</div>
        <div style="font-size:0.85rem; color:#6C757D; margin-top:0.25rem;">{meta}</div>
    </div>
    """, unsafe_allow_html=True)


# ----------------------------------------------------------------------
# Header + overall status
# ----------------------------------------------------------------------

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


# ----------------------------------------------------------------------
# Current state
# ----------------------------------------------------------------------

render_state_banner()

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


# ----------------------------------------------------------------------
# Power draw chart (Plotly)
# ----------------------------------------------------------------------

st.markdown("## Power draw")

def plot_power(df: pd.DataFrame) -> go.Figure:
    fig = go.Figure()
    fig.add_trace(go.Scatter(
        x=df["ts"], y=df["watts"], mode="lines",
        line=dict(color=COLORS["primary"], width=2),
        fill="tozeroy", fillcolor="rgba(45, 106, 79, 0.12)",
        hovertemplate="<b>%{y:.2f} W</b><br>%{x|%Y-%m-%d %H:%M:%S}<extra></extra>",
    ))
    fig.update_layout(
        margin=dict(l=20, r=20, t=20, b=20),
        height=280,
        paper_bgcolor="rgba(0,0,0,0)",
        plot_bgcolor="rgba(0,0,0,0)",
        xaxis=dict(showgrid=False, color=COLORS["text_muted"]),
        yaxis=dict(showgrid=True, gridcolor="#E8E5DC",
                   color=COLORS["text_muted"], title="Watts"),
        hoverlabel=dict(bgcolor="white", font_size=13),
    )
    return fig


tab_1h, tab_24h = st.tabs(["Last 1 hour", "Last 24 hours"])

with tab_1h:
    df = power_history(1)
    if df.empty:
        st.info("No power readings in the last hour.")
    else:
        df["ts"] = pd.to_datetime(df["ts"], utc=True).dt.tz_convert(LOCAL_TZ)
        st.plotly_chart(plot_power(df), use_container_width=True, key="power_1h")

with tab_24h:
    df = power_history(24)
    if df.empty:
        st.info("No power readings in the last 24 hours.")
    else:
        df["ts"] = pd.to_datetime(df["ts"], utc=True).dt.tz_convert(LOCAL_TZ)
        st.plotly_chart(plot_power(df), use_container_width=True, key="power_24h")


# ----------------------------------------------------------------------
# Soil moisture & watering history (DL-057)
# ----------------------------------------------------------------------

st.markdown("## Soil moisture & watering")
st.caption("Soil moisture over time, with watering episodes shaded in blue — "
           "watch the soil dry down and rise after each watering.")


def soil_history(hours: int) -> pd.DataFrame:
    return query_df(
        """SELECT ts, value AS moisture FROM sensor_readings
           WHERE sensor = 'moisture' AND device = 'soil'
             AND ts >= datetime('now', ?)
           ORDER BY ts""",
        (f"-{hours} hours",),
    )


def watering_episodes(hours: int) -> pd.DataFrame:
    # Derive watering bouts from FSM-state history: a span in 'watering' or
    # 'manual' is one episode. daily_pump_ms (cumulative) gives the volume via
    # its delta across the span (~1.0 mL/s, DL-048).
    out_cols = ["start", "end", "duration_s", "ml", "trigger"]
    df = query_df(
        """SELECT ts, status AS state, value AS daily_ms FROM system_status
           WHERE device = 'wrover' AND metric = 'fsm_state'
             AND ts >= datetime('now', ?)
           ORDER BY id""",
        (f"-{hours} hours",),
    )
    if df.empty:
        return pd.DataFrame(columns=out_cols)
    df["ts"] = pd.to_datetime(df["ts"], utc=True)
    eps, in_ep, start_ts, start_ms, trigger = [], False, None, 0.0, None
    for _, row in df.iterrows():
        watering = row["state"] in ("watering", "manual")
        if watering and not in_ep:
            in_ep, start_ts, start_ms, trigger = True, row["ts"], (row["daily_ms"] or 0.0), row["state"]
        elif not watering and in_ep:
            in_ep = False
            eps.append((start_ts, row["ts"], start_ms, row["daily_ms"] or start_ms, trigger))
    if in_ep:  # episode still open at the window edge
        last = df.iloc[-1]
        eps.append((start_ts, last["ts"], start_ms, last["daily_ms"] or start_ms, trigger))
    if not eps:
        return pd.DataFrame(columns=out_cols)
    out = pd.DataFrame(eps, columns=["start", "end", "start_ms", "end_ms", "trigger"])
    out["duration_s"] = (out["end"] - out["start"]).dt.total_seconds()
    out["ml"] = ((out["end_ms"] - out["start_ms"]).clip(lower=0)) / 1000.0
    out["trigger"] = out["trigger"].map({"watering": "Auto", "manual": "Manual"}).fillna("Auto")
    return out[out_cols]


def plot_soil(soil_df: pd.DataFrame, ep_df: pd.DataFrame) -> go.Figure:
    fig = go.Figure()
    for _, e in ep_df.iterrows():
        fig.add_vrect(x0=e["start"], x1=e["end"],
                      fillcolor="rgba(58, 124, 165, 0.20)", line_width=0, layer="below")
    fig.add_trace(go.Scatter(
        x=soil_df["ts"], y=soil_df["moisture"], mode="lines",
        line=dict(color=COLORS["primary"], width=2),
        hovertemplate="<b>%{y:.0f}%</b><br>%{x|%Y-%m-%d %H:%M}<extra></extra>",
    ))
    fig.update_layout(
        margin=dict(l=20, r=20, t=20, b=20), height=300,
        paper_bgcolor="rgba(0,0,0,0)", plot_bgcolor="rgba(0,0,0,0)",
        xaxis=dict(showgrid=False, color=COLORS["text_muted"]),
        yaxis=dict(showgrid=True, gridcolor="#E8E5DC", color=COLORS["text_muted"],
                   title="Soil moisture (%)", rangemode="tozero"),
        hoverlabel=dict(bgcolor="white", font_size=13),
    )
    return fig


def _render_soil_tab(hours: int, key: str):
    sdf = soil_history(hours)
    if sdf.empty:
        st.info("No soil readings in this window.")
        return
    sdf["ts"] = pd.to_datetime(sdf["ts"], utc=True).dt.tz_convert(LOCAL_TZ)
    edf = watering_episodes(hours)
    if not edf.empty:
        edf["start"] = edf["start"].dt.tz_convert(LOCAL_TZ)
        edf["end"] = edf["end"].dt.tz_convert(LOCAL_TZ)
    st.plotly_chart(plot_soil(sdf, edf), use_container_width=True, key=key)
    if edf.empty:
        st.caption("No watering episodes in this window.")
    else:
        tbl = pd.DataFrame({
            "Time": edf["start"].dt.strftime("%Y-%m-%d %H:%M"),
            "Duration": edf["duration_s"].round().astype(int).astype(str) + " s",
            "Water (approx)": edf["ml"].round().astype(int).astype(str) + " mL",
            "Trigger": edf["trigger"],
        })
        st.dataframe(tbl, hide_index=True, use_container_width=True)


tab_soil_24h, tab_soil_7d = st.tabs(["Last 24 hours", "Last 7 days"])
with tab_soil_24h:
    _render_soil_tab(24, "soil_24h")
with tab_soil_7d:
    _render_soil_tab(24 * 7, "soil_7d")


# ----------------------------------------------------------------------
# Recent activity
# ----------------------------------------------------------------------

st.markdown("## Recent activity")

events = recent_actuator_events(10)
if events.empty:
    st.info("No actuator events recorded yet.")
else:
    events["ts"] = events["ts"].apply(format_local)
    events_display = events.rename(columns={
        "ts": "Time", "actuator": "Device",
        "action": "Action", "source": "Source",
    })
    st.dataframe(events_display, hide_index=True, use_container_width=True)


# ----------------------------------------------------------------------
# Faults
# ----------------------------------------------------------------------

st.markdown("## Active faults")

if faults_df.empty:
    st.markdown('<div class="fault-banner-ok">✓ No unacknowledged faults</div>',
                unsafe_allow_html=True)
else:
    st.dataframe(faults_df, hide_index=True, use_container_width=True)


# ----------------------------------------------------------------------
# Listener run
# ----------------------------------------------------------------------

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


# ----------------------------------------------------------------------
# Future sensors placeholder
# ----------------------------------------------------------------------

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
