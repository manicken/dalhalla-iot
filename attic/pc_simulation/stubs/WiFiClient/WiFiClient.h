#pragma once
#include <cstdint>
#include <cstddef>




class WiFiClient {
public:
    WiFiClient() = default;
    ~WiFiClient() = default;

    // Optionally add dummy methods if code references them
    int connect(const char*, uint16_t) { return 0; }
    void stop() {}
    int connected() { return 0; }
    size_t write(uint8_t) { return 1; }
    int read() { return -1; }
};
