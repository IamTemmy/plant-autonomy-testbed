#pragma once
// MQTT capture-trigger for the camera node (DL-196). Mirrors the WROVER's
// net_mqtt: non-blocking, non-fatal on loss. Connects to the broker once WiFi
// is up, maintains a RETAINED presence on plant/status/camera, registers a
// Last Will so the broker marks the node offline on an unclean drop, and
// subscribes to plant/cmd/capture.
//
// The inbound handler does NOT capture directly (a blocking HTTP POST from
// inside the PubSubClient callback would starve the MQTT keepalive). It latches
// a one-shot request + the capture context; the main loop performs the capture.

#include <stddef.h>

void mqtt_begin();       // call once from setup(), after wifi_begin(); does not connect
void mqtt_tick();        // call every loop(); connects/reconnects + pumps the client
bool mqtt_connected();   // true if connected to the broker

// If a capture was requested since the last check, copy its context string into
// `out` (NUL-terminated, up to out_len) and return true, clearing the request.
// Returns false if no capture is pending. The main loop polls this.
bool mqtt_take_capture_request(char* out, size_t out_len);

// Publish the retained "online" presence. No-op if not connected.
void mqtt_publish_status();
