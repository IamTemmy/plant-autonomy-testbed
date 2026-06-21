#pragma once
#include "esp_camera.h"

// Camera for the XIAO ESP32-S3 Sense (OV3660/OV2640, auto-detected).
// camera_begin() returns true on successful init. camera_capture() returns a
// framebuffer the caller MUST release with esp_camera_fb_return(), or nullptr.
bool camera_begin();
camera_fb_t* camera_capture();
