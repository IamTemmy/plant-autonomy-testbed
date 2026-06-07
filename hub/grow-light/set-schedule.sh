#!/usr/bin/env bash
# Plant Autonomy Testbed - grow-light daily schedule (DL-054)
#
# Sets the Shelly Plus Plug's built-in scheduler to run the grow light on a
# fixed daily photoperiod, DEVICE-SIDE. Running on the Shelly itself means the
# light keeps its schedule even if the Pi, the WROVER, or the network is down.
#
# Photoperiod: 07:00 ON -> 19:00 OFF (12h light / 12h dark).
#
# Usage:   ./set-schedule.sh <SHELLY_IP>
# Verify:  the script prints Schedule.List at the end.
#
# PREREQUISITE: the Shelly's timezone/location must be set (Shelly app ->
# device Settings -> Geolocation/Time, or via Sys.SetConfig) so the schedule
# fires in local time (America/Chicago), not UTC.

set -euo pipefail

SHELLY_IP="${1:?Usage: set-schedule.sh <SHELLY_IP>}"
ON_HOUR=7       # 07:00 light ON
OFF_HOUR=19     # 19:00 light OFF

rpc() {  # rpc <method> <params-json>
    curl -fsS -X POST "http://${SHELLY_IP}/rpc" \
        -H 'Content-Type: application/json' \
        -d "{\"id\":1,\"method\":\"$1\",\"params\":$2}"
    echo
}

echo "Clearing existing schedules (idempotent re-run)..."
rpc Schedule.DeleteAll '{}' || true

echo "Creating ON schedule (daily ${ON_HOUR}:00)..."
rpc Schedule.Create \
    "{\"enable\":true,\"timespec\":\"0 0 ${ON_HOUR} * * *\",\"calls\":[{\"method\":\"Switch.Set\",\"params\":{\"id\":0,\"on\":true}}]}"

echo "Creating OFF schedule (daily ${OFF_HOUR}:00)..."
rpc Schedule.Create \
    "{\"enable\":true,\"timespec\":\"0 0 ${OFF_HOUR} * * *\",\"calls\":[{\"method\":\"Switch.Set\",\"params\":{\"id\":0,\"on\":false}}]}"

echo "Current schedules on the Shelly:"
rpc Schedule.List '{}'
