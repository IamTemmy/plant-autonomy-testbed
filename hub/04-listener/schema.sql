-- Plant Autonomy Testbed — hub services SQLite schema
-- Raw archive + extracted projection tables, with run/message provenance.
-- Foreign keys are enforced per-connection via PRAGMA in the listener.

-- ============================================================
-- runs: registry of all data-collection runs
-- ============================================================
CREATE TABLE IF NOT EXISTS runs (
    run_id      TEXT PRIMARY KEY,
    started_ts  TEXT NOT NULL,
    ended_ts    TEXT,
    phase       TEXT,
    description TEXT,
    notes       TEXT
);

-- ============================================================
-- mqtt_messages: raw archive, source of truth
-- ============================================================
CREATE TABLE IF NOT EXISTS mqtt_messages (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    ts        TEXT NOT NULL,
    topic     TEXT NOT NULL,
    payload   TEXT NOT NULL,
    qos       INTEGER DEFAULT 0,
    retained  INTEGER DEFAULT 0
);

CREATE INDEX IF NOT EXISTS idx_mqtt_messages_topic_ts
    ON mqtt_messages (topic, ts);
CREATE INDEX IF NOT EXISTS idx_mqtt_messages_ts
    ON mqtt_messages (ts);

-- ============================================================
-- sensor_readings: continuous measurements (EAV pattern)
-- ============================================================
CREATE TABLE IF NOT EXISTS sensor_readings (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    ts          TEXT NOT NULL,
    message_id  INTEGER,
    run_id      TEXT,
    device      TEXT NOT NULL,
    sensor      TEXT NOT NULL,
    value       REAL NOT NULL,
    unit        TEXT,
    FOREIGN KEY (message_id) REFERENCES mqtt_messages(id),
    FOREIGN KEY (run_id)     REFERENCES runs(run_id)
);

CREATE INDEX IF NOT EXISTS idx_sensor_readings_sensor_ts
    ON sensor_readings (sensor, ts);
CREATE INDEX IF NOT EXISTS idx_sensor_readings_device_ts
    ON sensor_readings (device, ts);
CREATE INDEX IF NOT EXISTS idx_sensor_readings_ts
    ON sensor_readings (ts);

-- ============================================================
-- actuator_events: discrete state changes
-- ============================================================
CREATE TABLE IF NOT EXISTS actuator_events (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    ts          TEXT NOT NULL,
    message_id  INTEGER,
    run_id      TEXT,
    actuator    TEXT NOT NULL,
    action      TEXT NOT NULL,
    value       REAL,
    unit        TEXT,
    source      TEXT,
    metadata    TEXT,
    FOREIGN KEY (message_id) REFERENCES mqtt_messages(id),
    FOREIGN KEY (run_id)     REFERENCES runs(run_id)
);

CREATE INDEX IF NOT EXISTS idx_actuator_events_actuator_ts
    ON actuator_events (actuator, ts);
CREATE INDEX IF NOT EXISTS idx_actuator_events_ts
    ON actuator_events (ts);

-- ============================================================
-- system_status: device health + online/offline
-- ============================================================
CREATE TABLE IF NOT EXISTS system_status (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    ts          TEXT NOT NULL,
    message_id  INTEGER,
    run_id      TEXT,
    device      TEXT NOT NULL,
    status      TEXT NOT NULL,
    metric      TEXT,
    value       REAL,
    FOREIGN KEY (message_id) REFERENCES mqtt_messages(id),
    FOREIGN KEY (run_id)     REFERENCES runs(run_id)
);

CREATE INDEX IF NOT EXISTS idx_system_status_device_ts
    ON system_status (device, ts);
CREATE INDEX IF NOT EXISTS idx_system_status_ts
    ON system_status (ts);

-- ============================================================
-- fault_events: warnings, critical issues, ack tracking
-- ============================================================
CREATE TABLE IF NOT EXISTS fault_events (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    ts              TEXT NOT NULL,
    message_id      INTEGER,
    run_id          TEXT,
    device          TEXT NOT NULL,
    fault_type      TEXT NOT NULL,
    severity        TEXT NOT NULL,
    message         TEXT,
    acknowledged    INTEGER DEFAULT 0,
    acknowledged_ts TEXT,
    FOREIGN KEY (message_id) REFERENCES mqtt_messages(id),
    FOREIGN KEY (run_id)     REFERENCES runs(run_id)
);

CREATE INDEX IF NOT EXISTS idx_fault_events_device_ts
    ON fault_events (device, ts);
CREATE INDEX IF NOT EXISTS idx_fault_events_acknowledged
    ON fault_events (acknowledged);
CREATE INDEX IF NOT EXISTS idx_fault_events_ts
    ON fault_events (ts);
