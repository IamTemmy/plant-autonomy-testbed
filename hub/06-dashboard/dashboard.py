"""
Plant Autonomy Testbed - Streamlit dashboard entry (DL-095).

Multipage restructure of the former single-file dashboard. Sets page config, the
global theme, and the 30s auto-refresh, then hands off to the page router. Pages
under dash_pages/ import shared helpers from dash_common.

Run with:
    streamlit run dashboard.py --server.address 0.0.0.0 --server.port 8501
"""

import streamlit as st

st.set_page_config(
    page_title="Plant Autonomy Testbed",
    page_icon="🌱",
    layout="wide",
    initial_sidebar_state="expanded",
)

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


pages = [
    st.Page("dash_pages/overview.py",  title="Overview",   icon="🌿", default=True),
    st.Page("dash_pages/watering.py",  title="Watering",   icon="💧"),
    st.Page("dash_pages/camera.py",    title="Camera",     icon="📷"),
    st.Page("dash_pages/growlight.py", title="Grow light", icon="💡"),
    st.Page("dash_pages/controls.py",  title="Controls",   icon="⚙️"),
]
st.navigation(pages).run()
