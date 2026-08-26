#include "poster.h"

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "config.h"

int poster_post_jpeg(const uint8_t* data, size_t len, const char* context) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("POST skipped: WiFi down");
        return -1;
    }

    HTTPClient http;
    http.begin(IMAGE_POST_URL);
    http.setTimeout(HTTP_TIMEOUT_MS);
    http.addHeader("Content-Type", "image/jpeg");
    if (context != nullptr && context[0] != '\0') {
        http.addHeader("X-Capture-Context", context);   // DL-196: dark|lit|fallback
    }

    int code = http.POST(const_cast<uint8_t*>(data), len);
    if (code > 0) {
        String body = http.getString();
        Serial.printf("POST %d [%s]: %s\n",
                      code, (context && context[0]) ? context : "-", body.c_str());
    } else {
        Serial.printf("POST failed: %s\n", http.errorToString(code).c_str());
    }
    http.end();
    return code;
}
