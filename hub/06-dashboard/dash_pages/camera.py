"""
Camera page (DL-095) - latest capture plus the green_ratio/green_area trend
(24h/7d). Page-local query/plot helpers; shared helpers from dash_common.
Render blocks extracted verbatim from the pre-restructure dashboard.py.
"""

from pathlib import Path

import pandas as pd
import plotly.graph_objects as go
import streamlit as st

from dash_common import (
    LOCAL_TZ,
    COLORS,
    GREEN_RATIO_BASELINE_LOW,
    GREEN_RATIO_BASELINE_HIGH,
    GREEN_RATIO_BASELINE_MEDIAN,
    query_df,
    latest_camera,
    load_camera_image,
    render_card,
    format_local,
)


def camera_history(hours: int) -> pd.DataFrame:
    return query_df(
        """SELECT ts, green_ratio, green_area FROM camera_readings
           WHERE ts >= strftime('%Y-%m-%dT%H:%M:%SZ','now', ?)
           ORDER BY ts""",
        (f"-{hours} hours",),
    )

def plot_camera(df: pd.DataFrame) -> go.Figure:
    fig = go.Figure()
    fig.add_hrect(y0=GREEN_RATIO_BASELINE_LOW, y1=GREEN_RATIO_BASELINE_HIGH,
                  fillcolor="rgba(45, 106, 79, 0.10)", line_width=0, layer="below")
    fig.add_hline(y=GREEN_RATIO_BASELINE_MEDIAN, line_dash="dot",
                  line_color="rgba(45, 106, 79, 0.45)", line_width=1)
    fig.add_trace(go.Scatter(
        x=df["ts"], y=df["green_ratio"], mode="lines", name="green_ratio",
        line=dict(color=COLORS["primary"], width=2),
        hovertemplate="ratio <b>%{y:.3f}</b><br>%{x|%Y-%m-%d %H:%M}<extra></extra>",
    ))
    fig.add_trace(go.Scatter(
        x=df["ts"], y=df["green_area"], mode="lines", name="green_area", yaxis="y2",
        line=dict(color=COLORS["status_warn"], width=1.5, dash="dash"),
        hovertemplate="area <b>%{y:.3f}</b><br>%{x|%Y-%m-%d %H:%M}<extra></extra>",
    ))
    fig.update_layout(
        margin=dict(l=20, r=20, t=20, b=20), height=300,
        paper_bgcolor="rgba(0,0,0,0)", plot_bgcolor="rgba(0,0,0,0)",
        xaxis=dict(showgrid=False, color=COLORS["text_muted"]),
        yaxis=dict(showgrid=True, gridcolor="#E8E5DC", color=COLORS["text_muted"],
                   title="green_ratio (foliage density)"),
        yaxis2=dict(overlaying="y", side="right", showgrid=False,
                    color=COLORS["text_muted"], title="green_area (frame fraction)"),
        legend=dict(orientation="h", yanchor="bottom", y=1.0, xanchor="left", x=0),
        hoverlabel=dict(bgcolor="white", font_size=13),
    )
    return fig

def _render_camera_tab(hours: int, key: str):
    cdf = camera_history(hours)
    if cdf.empty:
        st.info("No camera readings in this window.")
        return
    cdf["ts"] = pd.to_datetime(cdf["ts"], utc=True).dt.tz_convert(LOCAL_TZ)
    st.plotly_chart(plot_camera(cdf), use_container_width=True, key=key)
    st.caption(
        "Shaded band is the documented normal range for green_ratio (DL-087); "
        "dotted line is the baseline median. green_ratio (foliage density) is the "
        "primary metric; green_area is the plant's fraction of the frame. Full-frame "
        "greenness is omitted as noise."
    )


st.markdown("## Plant camera")

_cam = latest_camera()
if _cam is None:
    st.info("No camera captures recorded yet.")
else:
    _cam_ts, _cam_path, _cam_greenness, _cam_area, _cam_ratio = _cam
    img_col, meta_col = st.columns([2, 1])
    with img_col:
        if _cam_path and Path(_cam_path).exists():
            try:
                st.image(load_camera_image(_cam_path), use_container_width=True)
            except Exception:
                st.image(_cam_path, use_container_width=True)
            st.caption(f"Latest capture: {format_local(_cam_ts)}")
        else:
            st.warning("Latest image file is not available on disk.")
            st.caption(f"Recorded {format_local(_cam_ts)}")
    with meta_col:
        _ratio_ok = (_cam_ratio is not None and
                     GREEN_RATIO_BASELINE_LOW <= _cam_ratio <= GREEN_RATIO_BASELINE_HIGH)
        render_card("green_ratio",
                    f"{_cam_ratio:.3f}" if _cam_ratio is not None else "n/a",
                    f"foliage density (baseline ~{GREEN_RATIO_BASELINE_MEDIAN:.2f})",
                    "ok" if _ratio_ok else "warn")
        render_card("green_area",
                    f"{_cam_area:.3f}" if _cam_area is not None else "n/a",
                    "fraction of frame that is plant", "neutral")

    tab_cam_24h, tab_cam_7d = st.tabs(["Last 24 hours", "Last 7 days"])
    with tab_cam_24h:
        _render_camera_tab(24, "cam_24h")
    with tab_cam_7d:
        _render_camera_tab(24 * 7, "cam_7d")
