"""
Controls page (DL-095) - the maintenance toggle relocated verbatim from the
DL-094 status header. Banner shows current state; home for future remote actions.
"""

import streamlit as st
from streamlit_autorefresh import st_autorefresh

from dash_common import (
    render_state_banner,
    latest_fsm_state,
    _FAULT_STATES,
    send_maintenance_cmd,
    send_dose_cmd,
)


st_autorefresh(interval=30_000, key="autorefresh_controls")


st.markdown("## Controls")
render_state_banner()

_fsm = latest_fsm_state()
if _fsm and _fsm["state"] and _fsm["state"] not in _FAULT_STATES:
    _in_maint = _fsm["state"] == "maintenance"
    _label = "Resume watering" if _in_maint else "Pause watering (maintenance)"
    _cmd = "off" if _in_maint else "on"
    if st.button(_label, key="maint_toggle"):
        if send_maintenance_cmd(_cmd):
            st.success(f"Sent '{_cmd}'. The banner will update once the device confirms.")

st.markdown("### Bottom-watering session")
st.caption(
    "Commands the bottom-watering harness (`plant/cmd/dose`). No effect unless the "
    "harness firmware is running. **Start** forces a full session immediately "
    "(a real dose) \u2014 use only when supervised."
)
_c1, _c2 = st.columns(2)
with _c1:
    if st.button("Start session", key="dose_start"):
        if send_dose_cmd("start"):
            st.success("Sent 'start'. Watch the state banner and your phone for progress.")
with _c2:
    if st.button("Abort watering", key="dose_abort"):
        if send_dose_cmd("abort"):
            st.warning("Sent 'abort'. The pump will stop and the session will end.")
