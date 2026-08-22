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
# DL-183: the firmware now handles both remote commands on plant/cmd/dose — "abort"
# (DL-169) and "start" (DL-182, which respects maintenance). The controls here mirror
# exactly what the firmware will honour, so no button ever implies control it lacks:
#   * active session (dosing/settle/grace)  -> Abort
#   * armed + monitor (idle, ready)          -> Start
#   * maintenance / fault / reservoir-empty  -> honest caption (nothing offered)
_ACTIVE_STATES = {"dosing", "settle", "grace"}
_state = _fsm["state"] if _fsm else None
_active = _state in _ACTIVE_STATES
_in_fault = _state in _FAULT_STATES

if _active:
    st.warning(f"A watering session is active (**{_state}**).")
    if st.button("Abort watering now", key="dose_abort"):
        if send_dose_cmd("abort"):
            st.success("Sent 'abort'. The pump will stop and the session will end; "
                       "the state will update to 'stopped' once the device confirms.")
elif _maint:
    st.caption("In maintenance — remote start is disabled (arm above first). "
               "A dose can still be started at the plant with the MANUAL button.")
elif _in_fault:
    st.caption("Controller is in a fault state — clear it before starting a session.")
elif _state == "monitor":
    st.caption("Armed and idle. Start a full plateau-gated watering session now:")
    if st.button("Start watering session", key="dose_start"):
        if send_dose_cmd("start"):
            st.success("Sent 'start'. The controller will begin a session if soil is "
                       "fresh and the reservoir isn't empty; watch the state and your phone.")
else:
    st.caption("Session controls unavailable in the current state.")
