#pragma once
// Non-secret configuration for the camera node. Real WiFi credentials live in
// secrets.h (gitignored); see secrets.h.example.

// ---- Pi image receiver (hub/09-camera) ----
// Main LAN address. If the campus network blocks it (DL-028 isolation), swap to
// the Tailscale address: "http://100.79.225.18:8080/image".
static constexpr char IMAGE_POST_URL[] = "http://10.6.19.139:8080/image";

// ---- Capture cadence ----
// TEST value for bring-up validation — intentionally short so a POST is visible
// within seconds. Final deployment cadence is hourly and photoperiod-gated
// (a later slice); change this one line then.
static constexpr unsigned long CAPTURE_INTERVAL_MS = 20000;  // 20 s (test)

// ---- WiFi (mirrors the WROVER net_wifi timing) ----
static constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS    = 15000;
static constexpr unsigned long WIFI_RECONNECT_INTERVAL_MS = 10000;

// ---- HTTP ----
static constexpr int HTTP_TIMEOUT_MS = 10000;
