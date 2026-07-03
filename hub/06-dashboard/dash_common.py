"""
Plant Autonomy Testbed — shared dashboard helpers (DL-095).

Extracted verbatim from the pre-restructure single-file dashboard.py so the
multipage split is move-only: no query, render, or control behavior changes.
Imported by dashboard.py (entry) and every page under dash_pages/.
"""

import os
import sqlite3
from datetime import datetime, timezone
from pathlib import Path
from zoneinfo import ZoneInfo

import pandas as pd
import streamlit as st
import paho.mqtt.publish as mqtt_publish
from PIL import Image

REFRESH_SECONDS = 30




# --- constants ------------------------------------------------------

DB_PATH = Path(__file__).parent / "plant.db"

LOCAL_TZ = ZoneInfo("America/Chicago")

GREEN_RATIO_BASELINE_LOW = 0.464

GREEN_RATIO_BASELINE_HIGH = 0.556

GREEN_RATIO_BASELINE_MEDIAN = 0.534

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

STATE_DISPLAY = {
    "monitoring":      ("Monitoring",          "ok",    "Watching soil moisture"),
    "watering":        ("Watering",            "ok",    "Pump pulsing toward target"),
    "manual":          ("Manual watering",     "ok",    "Manual override running"),
    "reservoir_empty": ("Reservoir empty",     "warn",  "Refill needed \u2014 watering blocked"),
    "daily_limit":     ("Daily limit reached", "warn",  "Paused until the daily window resets"),
    "leak_fault":      ("Leak fault",          "fault", "Leak detected \u2014 pump stopped, needs ACK"),
    "stopped":         ("Emergency stop",      "fault", "Halted \u2014 needs ACK"),
    "watering_fault":  ("Watering fault",      "fault", "Soil not responding \u2014 pump stopped, needs ACK"),
    "maintenance":     ("Maintenance",         "warn",  "Watering paused intentionally; long-press MANUAL to resume"),
}

_BANNER_PALETTE = {
    "ok":      ("#D8F3DC", "#2D6A4F", "#2D6A4F"),
    "warn":    ("#FFF4D6", "#D08C00", "#D08C00"),
    "fault":   ("#FCE4E4", "#B23838", "#B23838"),
    "unknown": ("#F1F1F1", "#9CA3AF", "#6C757D"),
}

MAINT_CMD_TOPIC = "plant/cmd/maintenance"

_FAULT_STATES = {"leak_fault", "stopped", "watering_fault"}


# --- helpers ---------------------------------------------------------

def get_conn() -> sqlite3.Connection:
    # Read-write handle (needed for WAL shared-memory), but writes are forbidden
    # via query_only, and busy_timeout waits for a writer instead of erroring.
    conn = sqlite3.connect(str(DB_PATH))
    conn.execute("PRAGMA busy_timeout = 5000")
    conn.execute("PRAGMA query_only = ON")
    return conn

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

def _fmt_duration(seconds) -> str:
    try:
        s = int(seconds)
    except (TypeError, ValueError):
        return "unknown"
    if s < 60:
        return f"{s}s"
    m, s = divmod(s, 60)
    if m < 60:
        return f"{m}m"
    h, m = divmod(m, 60)
    if h < 24:
        return f"{h}h {m}m"
    d, h = divmod(h, 24)
    return f"{d}d {h}h"

def latest_reboot(device: str = "wrover"):
    df = query_df(
        """SELECT ts, value FROM system_status
           WHERE device = ? AND metric = 'reboot'
           ORDER BY id DESC LIMIT 1""",
        (device,),
    )
    if df.empty:
        return None
    return {"ts": df.iloc[0]["ts"], "prev_uptime_s": df.iloc[0]["value"]}

def reboots_recent(device: str = "wrover", hours: int = 24) -> int:
    df = query_df(
        """SELECT COUNT(*) AS n FROM system_status
           WHERE device = ? AND metric = 'reboot'
             AND ts >= strftime('%Y-%m-%dT%H:%M:%SZ','now', ?)""",
        (device, f"-{hours} hours"),
    )
    return int(df.iloc[0]["n"]) if not df.empty else 0

def send_maintenance_cmd(value: str) -> bool:
    user = os.environ.get("MQTT_USER")
    password = os.environ.get("MQTT_PASS")
    if not user or not password:
        st.error("MQTT credentials are not available to the dashboard service.")
        return False
    try:
        mqtt_publish.single(
            MAINT_CMD_TOPIC, value, hostname="localhost",
            auth={"username": user, "password": password},
        )
        return True
    except Exception as e:
        st.error(f"Could not send command: {e}")
        return False

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

def latest_camera():
    with get_conn() as conn:
        return conn.execute(
            "SELECT ts, path, greenness, green_area, green_ratio "
            "FROM camera_readings ORDER BY ts DESC LIMIT 1"
        ).fetchone()

def load_camera_image(path: str, max_width: int = 800):
    """Downscaled copy of a capture for the dashboard (cached per path)."""
    from PIL import Image
    img = Image.open(path)
    img.load()
    if img.width > max_width:
        img = img.resize((max_width, round(img.height * max_width / img.width)))
    return img
