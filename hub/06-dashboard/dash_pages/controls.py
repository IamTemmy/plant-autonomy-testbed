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
# DL-170: remote abort is now real — the integrated firmware handles
# plant/cmd/dose "abort" (DL-169), routing it into the STOP-button path. The Abort
# button is shown only while a session is actually active, so it never falsely
# implies control when there's nothing to stop. Remote "start" is NOT offered yet
# (deferred, riskier half) — a dose is still started at the plant via MANUAL.
_ACTIVE_STATES = {"dosing", "settle", "grace"}
_active = bool(_fsm) and _fsm["state"] in _ACTIVE_STATES
if _active:
    st.warning(f"A watering session is active (**{_fsm['state']}**).")
    if st.button("Abort watering now", key="dose_abort"):
        if send_dose_cmd("abort"):
            st.success("Sent 'abort'. The pump will stop and the session will end; "
                       "the state will update to 'stopped' once the device confirms.")
else:
    st.caption(
        "No active watering session to abort. Remote *start* isn't available yet — "
        "to force a dose, use the MANUAL button at the plant. To pause auto-watering "
        "remotely, use the maintenance toggle above."
    )
