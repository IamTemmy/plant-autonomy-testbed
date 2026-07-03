"""
Controls page (DL-095) - the maintenance toggle relocated verbatim from the
DL-094 status header. Banner shows current state; home for future remote actions.
"""

import streamlit as st

from dash_common import (
    render_state_banner,
    latest_fsm_state,
    _FAULT_STATES,
    send_maintenance_cmd,
)


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
