#pragma once
// Non-secret configuration for the camera node. Real WiFi credentials live in
// secrets.h (gitignored); see secrets.h.example.

// ---- Pi image receiver (hub/09-camera) ----
// Main LAN address. If the campus network blocks it (DL-028 isolation), swap to
// the Tailscale address: "http://100.79.225.18:8080/image".
static constexpr char IMAGE_POST_URL[] = "http://10.6.19.139:8080/image";

// ---- Capture cadence ----
// Deployment cadence: hourly. Photoperiod gating (only keep daytime captures)
// is enforced Pi-side by the receiver, which shares the grow-light window
// (GROW_ON_HOUR/GROW_OFF_HOUR), so the node stays dumb and has no clock.
static constexpr unsigned long CAPTURE_INTERVAL_MS = 3600000;  // 1 hour

// ---- WiFi (mirrors the WROVER net_wifi timing) ----
static constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS    = 15000;
static constexpr unsigned long WIFI_RECONNECT_INTERVAL_MS = 10000;

// ---- HTTP ----
static constexpr int HTTP_TIMEOUT_MS = 10000;
