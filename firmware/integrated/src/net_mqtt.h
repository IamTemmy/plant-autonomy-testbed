#pragma once
// MQTT telemetry for the Plant Autonomy Testbed.
// Mirrors net_wifi: non-blocking, non-fatal on loss (DL-040 Principles 1 & 5).
// Connects to the broker once WiFi is up, maintains a RETAINED presence
// message on plant/status/wrover, and registers a Last Will so the broker
// marks the device offline if it drops without a clean disconnect.

// Configure the MQTT client (server address). Call once from setup(), after
// wifi_begin(). Does NOT connect — mqtt_tick() connects once WiFi is up.
void mqtt_begin();

// Service the connection. Call every loop() iteration. Connects/reconnects on
// a fixed cadence when needed and pumps the client. Returns immediately.
void mqtt_tick();

// Publish the retained "online" presence/status message. No-op if not
// connected. Call on the heartbeat cadence from the main loop.
void mqtt_publish_status(unsigned long heartbeat);

// True if currently connected to the broker.
bool mqtt_connected();
