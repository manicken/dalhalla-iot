/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2026 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

/**
  TCP client on lwIP sockets, the Arduino WiFiClient API. Works on every lwIP interface
  (WiFi and Ethernet), the name is kept because DALHAL and its libraries use Network::TcpClient = WiFiClient.
*/

#pragma once

#include "Client.h"

class WiFiClient : public Client {
public:
    WiFiClient() = default;
    WiFiClient(const WiFiClient&) = delete;
    WiFiClient& operator=(const WiFiClient&) = delete;
    WiFiClient(WiFiClient&& other) noexcept : fd(other.fd), timeoutMs(other.timeoutMs) { other.fd = -1; }
    WiFiClient& operator=(WiFiClient&& other) noexcept;
    ~WiFiClient() override { stop(); }

    int connect(IPAddress ip, uint16_t port) override;
    int connect(const char* host, uint16_t port) override;
    size_t write(uint8_t b) override;
    size_t write(const uint8_t* buf, size_t size) override;
    int available() override;
    int read() override;
    int read(uint8_t* buf, size_t size) override;
    int peek() override;
    void flush() override {}
    void stop() override;
    uint8_t connected() override;
    operator bool() override { return connected(); }
    using Print::write;

    /** connect and write timeout */
    void setTimeout(uint32_t ms) { timeoutMs = ms; }

private:
    int fd = -1;
    uint32_t timeoutMs = 3000; // as arduino-esp32
};
