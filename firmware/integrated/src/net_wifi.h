#pragma once
// WiFi connection management for the Plant Autonomy Testbed.
// Non-blocking by design (DL-040 Principles 1 & 5): a bounded blocking connect
// at boot, then periodic non-blocking reconnect attempts from the main loop.
// WiFi loss is non-fatal — the control loop and watering continue regardless.
//
// Named net_wifi (not wifi) to avoid colliding with the framework's <WiFi.h>
// on case-insensitive filesystems.

// Bring up WiFi. Reads credentials from secrets.h. Blocks up to
// WIFI_CONNECT_TIMEOUT_MS waiting for an initial connection at boot, then
// returns regardless of outcome. Call once from setup().
void wifi_begin();

// Service the connection. Call every loop() iteration. On a fixed cadence
// (WIFI_RECONNECT_INTERVAL_MS) it checks the link and, if down, kicks off a
// non-blocking reconnect. Returns immediately — never stalls the loop.
void wifi_tick();

// True if currently associated with an access point. Cheap to call.
bool wifi_connected();
