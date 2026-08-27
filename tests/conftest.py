"""Shared pytest fixtures and import paths for the hub host-tests (P2-13).

The hub scripts live in numbered directories under hub/ (e.g. hub/10-maintenance)
that are not Python packages, so we add the relevant ones to sys.path here. Tests
exercise the real functions against synthetic in-memory SQLite databases and temp
directories -- no hardware, no network, no live plant.db.
"""
import os
import sys

_REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Hub script directories that host-tests import from. Add more as coverage grows.
_HUB_DIRS = [
    "hub/10-maintenance",   # retention.py
    "hub/04-listener",      # alerter.py (board-liveness + reboot-classification helpers)
    "hub/08-grow-light",    # photoperiod.py (desired_on window logic)
    "hub/12-plantctl",      # plantctl.py (age/format/soil-pct helpers)
    "hub/06-dashboard",     # dash_common.py (latest_camera + other query helpers)
    "hub/13-capture-scheduler",  # capture_scheduler.py (decide + parse_centers)
]
for _d in _HUB_DIRS:
    _p = os.path.join(_REPO_ROOT, _d)
    if _p not in sys.path:
        sys.path.insert(0, _p)
