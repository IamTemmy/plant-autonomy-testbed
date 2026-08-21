"""
Controls page (DL-095) - the maintenance toggle relocated verbatim from the
DL-094 status header. Banner shows current state; home for future remote actions.
"""

import streamlit as st
from streamlit_autorefresh import st_autorefresh

from dash_common import (
    render_state_banner,
    latest_fsm_state,
    latest_maintenance,
    _FAULT_STATES,
    send_maintenance_cmd,
    send_dose_cmd,
)


st_autorefresh(interval=30_000, key="autorefresh_controls")


st.markdown("## Controls")
render_state_banner()

# Maintenance toggle. Uses the authoritative `maintenance` flag (DL-148/162), not
# the FSM state string — under the integrated firmware the state stays 'monitor'
# while the maintenance flag is on, so the old state=='maintenance' check was wrong.
_fsm = latest_fsm_state()
_maint = latest_maintenance()
if _maint is None:
    st.caption("Maintenance state not yet reported by the controller.")
elif _fsm and _fsm["state"] in _FAULT_STATES:
    st.caption("Controller is in a fault state — clear the fault before arming.")
else:
    if _maint:
        st.info("🛠️ In maintenance — auto-watering is **paused**.")
        _label, _cmd = "Resume watering (arm)", "off"
    else:
        st.success("🌱 Armed — auto-watering is **active**.")
        _label, _cmd = "Pause watering (maintenance)", "on"
    if st.button(_label, key="maint_toggle"):
        if send_maintenance_cmd(_cmd):
            st.success(f"Sent '{_cmd}'. The status will update once the device confirms.")

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
