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
# F4 (DL-168): the Start/Abort dose buttons published to plant/cmd/dose, which the
# integrated (production) firmware does NOT subscribe to — so "Abort" would report
# "the pump will stop" while nothing happened: a dangerous false safety affordance.
# Removed until remote dose/abort is actually implemented in the firmware (planned).
# In the meantime, arming/maintenance above is the working remote control, and a
# real dose can be started at the plant with the MANUAL button.
st.caption(
    "Remote start/abort of a watering dose isn't available yet — the production "
    "firmware doesn't accept dose commands. To pause watering remotely, use the "
    "maintenance toggle above; to force a dose, use the MANUAL button at the plant."
)
