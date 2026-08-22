"""
Watering page (DL-095) - soil moisture over time with watering episodes shaded
(24h/7d). Page-local helpers; shared helpers from dash_common. Verbatim render.
"""

import pandas as pd
import plotly.graph_objects as go
import streamlit as st
from streamlit_autorefresh import st_autorefresh

from dash_common import LOCAL_TZ, COLORS, query_df


st_autorefresh(interval=60_000, key="autorefresh_watering")


def soil_history(hours: int) -> pd.DataFrame:
    return query_df(
        """SELECT ts, value AS moisture FROM sensor_readings
           WHERE sensor = 'moisture' AND device = 'soil'
             AND ts >= strftime('%Y-%m-%dT%H:%M:%SZ','now', ?)
           ORDER BY ts""",
        (f"-{hours} hours",),
    )

def watering_episodes(hours: int) -> pd.DataFrame:
    # F8 (DL-176): derive watering bouts from the integrated FSM history. An episode
    # is a contiguous span in an active watering state (dosing/settle/grace); the
    # delivered volume is the peak session_ml reached during the span (session_ml
    # rises through the doses, then resets to 0 when the session ends). The old code
    # keyed off retired states ('watering'/'manual') and a retired daily_pump_ms
    # delta, so it never rendered anything under the integrated firmware.
    out_cols = ["start", "end", "duration_s", "ml", "trigger"]
    df = query_df(
        """SELECT ts, status AS state, metric, value FROM system_status
           WHERE device = 'wrover' AND metric IN ('fsm_state','session_ml')
             AND ts >= strftime('%Y-%m-%dT%H:%M:%SZ','now', ?)
           ORDER BY id""",
        (f"-{hours} hours",),
    )
    if df.empty:
        return pd.DataFrame(columns=out_cols)
    df["ts"] = pd.to_datetime(df["ts"], utc=True)
    active = {"dosing", "settle", "grace"}
    eps, in_ep, start_ts, end_ts, peak_ml = [], False, None, None, 0.0
    last_state = None
    for _, row in df.iterrows():
        if row["metric"] == "session_ml":
            if in_ep and row["value"] is not None:
                peak_ml = max(peak_ml, float(row["value"]))
            continue
        # metric == fsm_state
        is_active = row["state"] in active
        if is_active and not in_ep:
            in_ep, start_ts, end_ts, peak_ml = True, row["ts"], row["ts"], 0.0
        elif is_active and in_ep:
            end_ts = row["ts"]
        elif not is_active and in_ep:
            eps.append((start_ts, row["ts"], peak_ml))
            in_ep = False
    if in_ep:  # episode still open at the window edge
        eps.append((start_ts, df.iloc[-1]["ts"], peak_ml))
    if not eps:
        return pd.DataFrame(columns=out_cols)
    out = pd.DataFrame(eps, columns=["start", "end", "ml"])
    out["duration_s"] = (out["end"] - out["start"]).dt.total_seconds()
    out["trigger"] = "Auto"   # integrated doesn't distinguish auto vs manual in state history
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
    st.plotly_chart(plot_soil(sdf, edf), width="stretch", key=key)
    if edf.empty:
        st.caption("No watering episodes in this window.")
    else:
        tbl = pd.DataFrame({
            "Time": edf["start"].dt.strftime("%Y-%m-%d %H:%M"),
            "Duration": edf["duration_s"].round().astype(int).astype(str) + " s",
            "Water (approx)": edf["ml"].round().astype(int).astype(str) + " mL",
            "Trigger": edf["trigger"],
        })
        st.dataframe(tbl, hide_index=True, width="stretch")


st.markdown("## Soil moisture & watering")
st.caption("Soil moisture over time, with watering episodes shaded in blue — "
           "watch the soil dry down and rise after each watering.")

tab_soil_24h, tab_soil_7d = st.tabs(["Last 24 hours", "Last 7 days"])
with tab_soil_24h:
    _render_soil_tab(24, "soil_24h")
with tab_soil_7d:
    _render_soil_tab(24 * 7, "soil_7d")
