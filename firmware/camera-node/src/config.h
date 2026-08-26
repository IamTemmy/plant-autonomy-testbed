#pragma once
// Non-secret configuration for the camera node. Real WiFi credentials live in
// secrets.h (gitignored); see secrets.h.example.

// ---- Pi image receiver (hub/09-camera) ----
// Main LAN address. If the campus network blocks it (DL-028 isolation), swap to
// the Tailscale address: "http://100.79.225.18:8080/image".
static constexpr char IMAGE_POST_URL[] = "http://10.6.19.139:8080/image";

// ---- Capture cadence ----
// Capture is now Pi-triggered (DL-196): the Pi orchestrator publishes
// plant/cmd/capture at the scheduled moments (5 grow-light-OFF measurement
// windows + a 30-min lit time-lapse), because only the Pi knows the wall clock
// and owns the grow-light. The self-timer below is a resilience FALLBACK only:
// if the orchestrator dies the node still captures occasionally, but those
// frames are tagged "fallback" so they never feed the calibrated metric.
static constexpr unsigned long CAPTURE_INTERVAL_MS = 7200000UL;  // 2 h fallback

// ---- MQTT (DL-196): inbound capture trigger ----
// Mirrors the WROVER's net_mqtt. The node subscribes to plant/cmd/capture; the
// payload is the capture CONTEXT ("dark"|"lit"), echoed to the receiver so it
// can tag the frame. Broker + creds pattern match the WROVER (creds in secrets.h).
static constexpr char     MQTT_BROKER_HOST[]         = "10.6.19.139";
static constexpr uint16_t MQTT_BROKER_PORT           = 1883;
static constexpr char     MQTT_CLIENT_ID[]           = "camera";
static constexpr char     MQTT_TOPIC_CMD_CAPTURE[]   = "plant/cmd/capture";     // inbound: context "dark"|"lit"
static constexpr char     MQTT_TOPIC_STATUS[]        = "plant/status/camera";   // retained presence + Last-Will
static constexpr uint32_t MQTT_RECONNECT_INTERVAL_MS = 5000;
static constexpr int      CAPTURE_CONTEXT_MAX        = 16;   // longest accepted context string

// ---- WiFi (mirrors the WROVER net_wifi timing) ----
static constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS    = 15000;
static constexpr unsigned long WIFI_RECONNECT_INTERVAL_MS = 10000;

// ---- HTTP ----
static constexpr int HTTP_TIMEOUT_MS = 10000;
