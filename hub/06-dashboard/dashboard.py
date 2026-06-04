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


# ----------------------------------------------------------------------
# Header + overall status
# ----------------------------------------------------------------------

faults_df = unacked_faults()
online = device_online_status("grow-light")

if not faults_df.empty:
    overall_pill = render_status_pill(f"{len(faults_df)} active fault(s)", "fault")
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
st.caption("Sensor data will populate once the ESP32 WROVER firmware is publishing.")

cols = st.columns(3)
placeholders = [
    ("Air temperature", "BME280"),
    ("Humidity", "BME280"),
    ("Light level", "BH1750"),
    ("Soil moisture", "Capacitive sensor"),
    ("Reservoir level", "Float switch"),
    ("Leak sensor", "Conductive strip"),
]

for i, (label, meta) in enumerate(placeholders):
    with cols[i % 3]:
        render_card(label, "—", meta, "unknown")
