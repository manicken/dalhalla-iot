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
  ArduinoOTA on plain ESP-IDF (esp_ota_ops): the same network protocol as arduino-esp32 (espota.py),
  so "pio run -t upload" with an IP address as upload_port works as with the Arduino firmware.
  Firmware (U_FLASH) and filesystem (U_SPIFFS, the "spiffs" partition = LittleFS) images,
  the MD5 sent by the host is checked before anything is activated. Optional password (MD5 challenge).
*/

#pragma once

#include <cstdint>
#include <functional>
#include <string>

#define U_FLASH  0
#define U_SPIFFS 100
#define U_AUTH   200

typedef enum {
    OTA_AUTH_ERROR,
    OTA_BEGIN_ERROR,
    OTA_CONNECT_ERROR,
    OTA_RECEIVE_ERROR,
    OTA_END_ERROR
} ota_error_t;

class ArduinoOTAClass {
public:
    using THandlerFunction = std::function<void()>;
    using THandlerFunction_Error = std::function<void(ota_error_t)>;
    using THandlerFunction_Progress = std::function<void(unsigned int, unsigned int)>;

    void setPort(uint16_t port) { otaPort = port; }
    void setHostname(const char* name) { hostname = name ? name : ""; }
    const char* getHostname() const { return hostname.c_str(); }
    void setPassword(const char* password);
    void setRebootOnSuccess(bool reboot) { rebootOnSuccess = reboot; }

    void onStart(THandlerFunction fn) { startCb = fn; }
    void onEnd(THandlerFunction fn) { endCb = fn; }
    void onError(THandlerFunction_Error fn) { errorCb = fn; }
    void onProgress(THandlerFunction_Progress fn) { progressCb = fn; }

    void begin();
    void end();
    /** call often from the main loop, an update runs (blocking) from here when the host sends an invitation */
    void handle();
    int getCommand() const { return cmd; }

private:
    enum class State { Idle, WaitAuth };

    void runUpdate(uint32_t hostIp);
    void error(ota_error_t e, const char* text);
    void udpReply(uint32_t ip, uint16_t port, const char* text);

    uint16_t otaPort = 3232; // arduino-esp32 default
    std::string hostname;
    std::string passwordMd5; // md5 hex of the password, empty = no password
    bool rebootOnSuccess = true;

    int udpFd = -1;
    State state = State::Idle;
    int cmd = U_FLASH;
    uint16_t hostTcpPort = 0;
    uint32_t imageSize = 0;
    std::string imageMd5;
    std::string nonce;

    THandlerFunction startCb;
    THandlerFunction endCb;
    THandlerFunction_Error errorCb;
    THandlerFunction_Progress progressCb;
};

extern ArduinoOTAClass ArduinoOTA;
