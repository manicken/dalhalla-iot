#pragma once
#undef INPUT
#undef OUTPUT

#include <cstdint>
#include <cstddef>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "Client.h"

#pragma comment(lib, "Ws2_32.lib") // Link Winsock



// Windows implementation
class WiFiClient : public Client {
public:
    WiFiClient();
    ~WiFiClient();

    int connect(const char* host, uint16_t port) override;
    int available() override;
    int read() override;
    int read(uint8_t* buf, size_t size) override;
    size_t write(uint8_t data) override;
    size_t write(const uint8_t* buf, size_t size) override;
    void stop() override;
    uint8_t connected() override;
    operator bool() override;

private:
    SOCKET sock;
    bool isConnected;
};
