#pragma once
#include <stddef.h>
#include <stdint.h>

// POST a JPEG to the Pi image receiver (IMAGE_POST_URL). Returns the HTTP
// status code (200 = stored), or a negative HTTPClient error code on transport
// failure. Logs the receiver's JSON reply (incl. greenness) to serial.
int poster_post_jpeg(const uint8_t* data, size_t len);
