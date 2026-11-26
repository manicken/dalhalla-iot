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
  WebSocketAPI for the ESP-IDF backend.
  PLACEHOLDER until the web server on esp_http_server exists (step 4): same interface as the
  Arduino version, but nothing is sent, the Broadcast functions return false (no client).
*/

#pragma once

#include <string>
#include <cstddef>
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>
#include <DALHAL/API/DALHAL_CommandCallback.h>

namespace DALHAL {

    class WebSocketAPI {
    public:
        static void setup(bool failsafeMode = false) { (void)failsafeMode; }
        static void loop() {}

        static bool Broadcast(const char*, size_t, CmdCbType = CmdCbType::Control) { return false; }
        static bool Broadcast(std::string&, CmdCbType = CmdCbType::Control) { return false; }
        static bool Broadcast(const char*, CmdCbType = CmdCbType::Control) { return false; }
        static bool Broadcast(const ZeroCopyString&, CmdCbType = CmdCbType::Control) { return false; }
        static bool BroadcastCb(const ZeroCopyString&, CmdCbType) { return false; }
        /** can be used to combine two messages */
        static bool Broadcast(const char*, const char*, CmdCbType = CmdCbType::Control) { return false; }
    };

}
