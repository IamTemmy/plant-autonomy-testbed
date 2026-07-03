"""
Watering page (DL-095) - soil moisture over time with watering episodes shaded
(24h/7d). Page-local helpers; shared helpers from dash_common. Verbatim render.
"""

import pandas as pd
import plotly.graph_objects as go
import streamlit as st

from dash_common import LOCAL_TZ, COLORS, query_df


def soil_history(hours: int) -> pd.DataFrame:
    return query_df(
        """SELECT ts, value AS moisture FROM sensor_readings
           WHERE sensor = 'moisture' AND device = 'soil'
             AND ts >= strftime('%Y-%m-%dT%H:%M:%SZ','now', ?)
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
             AND ts >= strftime('%Y-%m-%dT%H:%M:%SZ','now', ?)
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


st.markdown("## Soil moisture & watering")
st.caption("Soil moisture over time, with watering episodes shaded in blue — "
           "watch the soil dry down and rise after each watering.")

tab_soil_24h, tab_soil_7d = st.tabs(["Last 24 hours", "Last 7 days"])
with tab_soil_24h:
    _render_soil_tab(24, "soil_24h")
with tab_soil_7d:
    _render_soil_tab(24 * 7, "soil_7d")
