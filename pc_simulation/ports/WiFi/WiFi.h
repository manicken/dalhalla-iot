#pragma once

#include <string>
#include <cstdint>
#include <cstring>
#include "../IPAddress/IPAddress.h"

class WiFiClass {
public:
    // Resolves hostname to uint8_t[4] IPv4 address
    bool hostByName(const char* host, IPAddress ip);
};

// Provide a global instance like ESP32/ESP8266 does
extern WiFiClass WiFi;
