"""
Power page (DL-095) - grow-light power draw (1h/24h) and the listener-run status.
Page-local query/plot helpers; shared helpers from dash_common. Verbatim render.
"""

import pandas as pd
import plotly.graph_objects as go
import streamlit as st

from dash_common import LOCAL_TZ, COLORS, query_df, current_run, format_local


def power_history(hours: int) -> pd.DataFrame:
    return query_df(
        """SELECT ts, value AS watts FROM sensor_readings
           WHERE sensor = 'power' AND device = 'grow-light'
             AND ts >= strftime('%Y-%m-%dT%H:%M:%SZ','now', ?)
           ORDER BY ts""",
        (f"-{hours} hours",),
    )

def plot_power(df: pd.DataFrame, hours: int) -> go.Figure:
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
        # vertical gridlines + denser ticks so a point is easy to place in time:
        # 10-min ticks on the 1-hour view, 2-hour ticks on the 24-hour view.
        xaxis=dict(showgrid=True, gridcolor="#E8E5DC", color=COLORS["text_muted"],
                   dtick=(600000 if hours <= 1 else 7200000), tickformat="%H:%M"),
        yaxis=dict(showgrid=True, gridcolor="#E8E5DC",
                   color=COLORS["text_muted"], title="Watts"),
        hoverlabel=dict(bgcolor="white", font_size=13),
    )
    return fig


st.markdown("## Power draw")

tab_1h, tab_24h = st.tabs(["Last 1 hour", "Last 24 hours"])

with tab_1h:
    df = power_history(1)
    if df.empty:
        st.info("No power readings in the last hour.")
    else:
        df["ts"] = pd.to_datetime(df["ts"], utc=True).dt.tz_convert(LOCAL_TZ)
        st.plotly_chart(plot_power(df, 1), use_container_width=True, key="power_1h")

with tab_24h:
    df = power_history(24)
    if df.empty:
        st.info("No power readings in the last 24 hours.")
    else:
        df["ts"] = pd.to_datetime(df["ts"], utc=True).dt.tz_convert(LOCAL_TZ)
        st.plotly_chart(plot_power(df, 24), use_container_width=True, key="power_24h")


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
