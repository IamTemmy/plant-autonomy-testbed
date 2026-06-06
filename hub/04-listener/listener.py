"""
Plant Autonomy Testbed — hub services MQTT listener.

Subscribes to plant/# on the local Mosquitto broker, writes every message
to mqtt_messages (raw archive), and projects structured fields into the
specialized tables (sensor_readings, actuator_events, system_status,
fault_events).

Run as: python listener.py
Credentials: MQTT_USER and MQTT_PASS environment variables.
Run identifier: RUN_ID (literal), or RUN_PHASE (prefix for auto-generated),
or neither (defaults to "auto" prefix).

Graceful shutdown: SIGINT or SIGTERM updates runs.ended_ts and exits.
"""

import json
import logging
import os
import signal
import sqlite3
import sys
from datetime import datetime, timezone
from pathlib import Path

import paho.mqtt.client as mqtt

# ----------------------------------------------------------------------
# Configuration
# ----------------------------------------------------------------------

DB_PATH = Path(__file__).parent / "plant.db"
BROKER_HOST = "localhost"
BROKER_PORT = 1883
SUBSCRIBE_TOPIC = "plant/#"
CLIENT_ID = "plant-hub-listener"

# ----------------------------------------------------------------------
# Logging
# ----------------------------------------------------------------------

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)
log = logging.getLogger("listener")


# ----------------------------------------------------------------------
# Helpers
# ----------------------------------------------------------------------

def utc_now_iso() -> str:
    """ISO 8601 UTC timestamp with seconds precision."""
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def resolve_run_id() -> tuple[str, str]:
    """
    Return (run_id, phase) per the rules:
      1. If RUN_ID is set, use it; phase = RUN_PHASE if set else None.
      2. Else if RUN_PHASE is set, generate '<phase>_<UTC stamp>'.
      3. Else generate 'auto_<UTC stamp>'.
    """
    explicit = os.environ.get("RUN_ID")
    phase = os.environ.get("RUN_PHASE")

    if explicit:
        return explicit, phase

    stamp = datetime.now(timezone.utc).strftime("%Y%m%d_%H%M%S")
    prefix = phase if phase else "auto"
    return f"{prefix}_{stamp}", phase


def ensure_run_row(conn: sqlite3.Connection, run_id: str, phase: str | None) -> None:
    """INSERT OR IGNORE the run row; existing rows are not touched."""
    conn.execute(
        "INSERT OR IGNORE INTO runs (run_id, started_ts, phase) VALUES (?, ?, ?)",
        (run_id, utc_now_iso(), phase),
    )
    conn.commit()


def mark_run_ended(conn: sqlite3.Connection, run_id: str) -> None:
    """Update ended_ts on graceful shutdown."""
    conn.execute(
        "UPDATE runs SET ended_ts = ? WHERE run_id = ? AND ended_ts IS NULL",
        (utc_now_iso(), run_id),
    )
    conn.commit()


# ----------------------------------------------------------------------
# Topic routing
# ----------------------------------------------------------------------

# Tracks last-known actuator state to avoid logging duplicate state messages.
# Key: actuator name (e.g. "grow-light"), Value: last known output (bool)
_last_actuator_state: dict[str, bool] = {}


def route_message(
    conn: sqlite3.Connection,
    message_id: int,
    run_id: str,
    ts: str,
    topic: str,
    payload: str,
) -> None:
    """Parse the topic + payload, write to the appropriate specialized tables."""
    try:
        # Most Shelly topics carry JSON; some (online) carry plain text.
        parts = topic.split("/")

        # plant/sensors/<device> -> sensor_readings (EAV fan-out)
        # WROVER telemetry: one JSON blob per sensor device, projected into one
        # sensor_readings row per metric. Added when BME280 publishing landed.
        if len(parts) == 3 and parts[1] == "sensors":
            device = parts[2]
            try:
                data = json.loads(payload)
            except json.JSONDecodeError:
                log.warning("Non-JSON sensor payload on %s: %r", topic, payload[:80])
                return
            sensor_map = [
                ("temperature", "C",   data.get("temperature_c")),
                ("humidity",    "%",   data.get("humidity_pct")),
                ("pressure",    "hPa", data.get("pressure_hpa")),
                ("lux",         "lx",  data.get("lux")),
                ("soil_raw",    "adc", data.get("soil_raw")),
                ("moisture",    "%",   data.get("moisture_pct")),
            ]
            rows = [
                (ts, message_id, run_id, device, sensor, float(value), unit)
                for sensor, unit, value in sensor_map
                if value is not None
            ]
            if rows:
                conn.executemany(
                    """INSERT INTO sensor_readings
                       (ts, message_id, run_id, device, sensor, value, unit)
                       VALUES (?, ?, ?, ?, ?, ?, ?)""",
                    rows,
                )
            return

        # plant/grow-light/online -> system_status
        if len(parts) == 3 and parts[2] == "online":
            device = parts[1]
            status = "online" if payload.strip().lower() == "true" else "offline"
            conn.execute(
                """INSERT INTO system_status
                   (ts, message_id, run_id, device, status)
                   VALUES (?, ?, ?, ?, ?)""",
                (ts, message_id, run_id, device, status),
            )
            return

        # plant/grow-light/status/<component> -> route by component
        if len(parts) == 4 and parts[2] == "status":
            device = parts[1]
            component = parts[3]
            try:
                data = json.loads(payload)
            except json.JSONDecodeError:
                log.warning("Non-JSON status payload on %s: %r", topic, payload[:80])
                return

            if component.startswith("switch"):
                _handle_switch_status(conn, message_id, run_id, ts, device, data)
                return

            if component == "mqtt":
                _handle_mqtt_status(conn, message_id, run_id, ts, device, data)
                return

            if component == "cloud":
                _handle_cloud_status(conn, message_id, run_id, ts, device, data)
                return

            if component == "sys":
                _handle_sys_status(conn, message_id, run_id, ts, device, data)
                return

        # plant/grow-light/events/rpc -> raw only (no projection)
        if len(parts) == 4 and parts[2] == "events":
            return

        # Anything else: raw only.

    except Exception:
        # Never crash on a malformed message; the raw row is already saved.
        log.exception("Failed to project message from topic %s", topic)


def _handle_switch_status(conn, message_id, run_id, ts, device, data) -> None:
    """
    plant/<device>/status/switch:0 carries both relay state and electrical
    readings. State change -> actuator_events. Readings -> sensor_readings.
    """
    actuator = device

    # State change?
    if "output" in data:
        output = bool(data["output"])
        prev = _last_actuator_state.get(actuator)
        if prev is None or prev != output:
            source = data.get("source")
            conn.execute(
                """INSERT INTO actuator_events
                   (ts, message_id, run_id, actuator, action, value, unit, source)
                   VALUES (?, ?, ?, ?, ?, ?, ?, ?)""",
                (
                    ts, message_id, run_id, actuator,
                    "on" if output else "off",
                    1.0 if output else 0.0,
                    "boolean",
                    source,
                ),
            )
            _last_actuator_state[actuator] = output

    # Continuous electrical readings
    sensor_map = [
        ("voltage",       "V",  data.get("voltage")),
        ("current",       "A",  data.get("current")),
        ("power",         "W",  data.get("apower")),
        ("frequency",     "Hz", data.get("freq")),
    ]
    temp = data.get("temperature")
    if isinstance(temp, dict) and "tC" in temp:
        sensor_map.append(("temperature", "C", temp["tC"]))
    energy = data.get("aenergy")
    if isinstance(energy, dict) and "total" in energy:
        sensor_map.append(("energy_cumulative", "Wh", energy["total"]))

    rows = [
        (ts, message_id, run_id, device, sensor, float(value), unit)
        for sensor, unit, value in sensor_map
        if value is not None
    ]
    if rows:
        conn.executemany(
            """INSERT INTO sensor_readings
               (ts, message_id, run_id, device, sensor, value, unit)
               VALUES (?, ?, ?, ?, ?, ?, ?)""",
            rows,
        )


def _handle_mqtt_status(conn, message_id, run_id, ts, device, data) -> None:
    connected = bool(data.get("connected", False))
    conn.execute(
        """INSERT INTO system_status
           (ts, message_id, run_id, device, status, metric, value)
           VALUES (?, ?, ?, ?, ?, ?, ?)""",
        (ts, message_id, run_id, device, "online" if connected else "offline",
         "broker_connected", 1.0 if connected else 0.0),
    )


def _handle_cloud_status(conn, message_id, run_id, ts, device, data) -> None:
    connected = bool(data.get("connected", False))
    conn.execute(
        """INSERT INTO system_status
           (ts, message_id, run_id, device, status, metric, value)
           VALUES (?, ?, ?, ?, ?, ?, ?)""",
        (ts, message_id, run_id, device, "online" if connected else "offline",
         "cloud_connected", 1.0 if connected else 0.0),
    )


def _handle_sys_status(conn, message_id, run_id, ts, device, data) -> None:
    """Extract uptime + ram_free; rest stays in raw archive."""
    rows = []
    for metric_name, payload_key in [
        ("uptime",   "uptime"),
        ("ram_free", "ram_free"),
    ]:
        value = data.get(payload_key)
        if value is not None:
            rows.append(
                (ts, message_id, run_id, device, "online", metric_name, float(value))
            )
    if rows:
        conn.executemany(
            """INSERT INTO system_status
               (ts, message_id, run_id, device, status, metric, value)
               VALUES (?, ?, ?, ?, ?, ?, ?)""",
            rows,
        )


# ----------------------------------------------------------------------
# MQTT callbacks
# ----------------------------------------------------------------------

def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        log.info("Connected to broker; subscribing to %s", SUBSCRIBE_TOPIC)
        client.subscribe(SUBSCRIBE_TOPIC)
    else:
        log.error("Connect failed: reason_code=%s", reason_code)


def on_disconnect(client, userdata, flags, reason_code, properties):
    log.warning("Disconnected from broker: reason_code=%s; auto-reconnect will retry", reason_code)


def on_message(client, userdata, msg):
    conn: sqlite3.Connection = userdata["conn"]
    run_id: str = userdata["run_id"]

    ts = utc_now_iso()
    topic = msg.topic
    try:
        payload = msg.payload.decode("utf-8")
    except UnicodeDecodeError:
        payload = repr(msg.payload)

    # Always: raw archive first
    try:
        cursor = conn.execute(
            """INSERT INTO mqtt_messages (ts, topic, payload, qos, retained)
               VALUES (?, ?, ?, ?, ?)""",
            (ts, topic, payload, msg.qos, int(msg.retain)),
        )
        message_id = cursor.lastrowid
    except Exception:
        log.exception("Failed to write raw message; topic=%s", topic)
        return

    # Then: route to specialized tables
    route_message(conn, message_id, run_id, ts, topic, payload)
    conn.commit()


# ----------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------

def main() -> int:
    user = os.environ.get("MQTT_USER")
    password = os.environ.get("MQTT_PASS")
    if not user or not password:
        log.error("MQTT_USER and MQTT_PASS environment variables must both be set")
        return 1

    run_id, phase = resolve_run_id()
    log.info("Run identifier: %s (phase=%s)", run_id, phase or "<none>")

    if not DB_PATH.exists():
        log.error("Database not found at %s; initialize with schema.sql first", DB_PATH)
        return 1

    conn = sqlite3.connect(str(DB_PATH))
    conn.execute("PRAGMA foreign_keys = ON")
    ensure_run_row(conn, run_id, phase)

    client = mqtt.Client(
        client_id=CLIENT_ID,
        callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
        userdata={"conn": conn, "run_id": run_id},
    )
    client.username_pw_set(user, password)
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_message = on_message

    # Graceful shutdown on SIGINT/SIGTERM
    def shutdown(signum, frame):
        log.info("Received signal %s; shutting down", signum)
        mark_run_ended(conn, run_id)
        client.disconnect()
        client.loop_stop()
        conn.close()
        sys.exit(0)

    signal.signal(signal.SIGINT, shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    log.info("Connecting to %s:%d as %s", BROKER_HOST, BROKER_PORT, user)
    client.connect(BROKER_HOST, BROKER_PORT, keepalive=60)
    client.loop_forever()
    return 0


if __name__ == "__main__":
    sys.exit(main())
