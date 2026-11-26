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
  WebSocketAPI for the ESP-IDF backend, on esp_http_server. Same behaviour as the Arduino version
  (src/DALHAL/API/WebSocket, ESPAsyncWebServer): port 82, WebSocket at /ws, files from LittleFS /api/...
  or else the built in frontend (FrontendFiles, gzip), commands go through the CommandExecutor queue.
*/

#pragma once

#include <string>
#include <cstddef>
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>
#include <DALHAL/API/DALHAL_CommandCallback.h>

#define DALHAL_WEBSOCKET_API_PORT 82

namespace DALHAL {

    class WebSocketAPI {
    public:
        /** starts the server, calling it again (e.g. after every network up) keeps the running server */
        static void setup(bool failsafeMode = false);
        static void loop() {}

        static bool Broadcast(const char* msg, size_t len, CmdCbType type = CmdCbType::Control);
        static bool Broadcast(std::string& msg, CmdCbType type = CmdCbType::Control);
        static bool Broadcast(const char* msg, CmdCbType type = CmdCbType::Control);
        static bool Broadcast(const ZeroCopyString& zcStr, CmdCbType type = CmdCbType::Control);
        static bool BroadcastCb(const ZeroCopyString& zcStr, CmdCbType type);
        /** can be used to combine two messages */
        static bool Broadcast(const char* source, const char* msg, CmdCbType type = CmdCbType::Control);
    };

}
