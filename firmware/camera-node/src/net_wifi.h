#pragma once
// WiFi for the camera node. Bounded blocking connect at boot, then non-blocking
// reconnects from the loop; WiFi loss is non-fatal. Mirrors the WROVER's
// net_wifi pattern. Named net_wifi to avoid colliding with the framework
// <WiFi.h> on case-insensitive filesystems.

void wifi_begin();      // call once from setup(); reads creds from secrets.h
void wifi_tick();       // call every loop(); non-blocking reconnect cadence
bool wifi_connected();  // true if associated
