/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

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

#include "HAL_JSON_RemoteFetch_REST.h"

namespace HAL_JSON {
    RemoteFetch_REST::RemoteFetch_REST(const char* type, const std::string& url, uint32_t refreshMs)
        : HAL_JSON::Device(HAL_JSON::UIDPathMaxLength::One, type),
        remoteUrl(url),
        refreshTimeMs(refreshMs),
        lastRefresh(0)
    {
    }

    HAL_JSON::HALOperationResult RemoteFetch_REST::read(HAL_JSON::HALValue& val) {
        val = cachedValue;
        return HAL_JSON::HALOperationResult::Success;
    }

    void RemoteFetch_REST::loop() {
        uint32_t now = millis();
        if (now - lastRefresh >= refreshTimeMs) {
            lastRefresh = now;
            cachedValue = fetchRemoteValue();
        }
    }

    HAL_JSON::HALValue RemoteFetch_REST::fetchRemoteValue() {
    #if defined(_WIN32)
        HTTPClient client;
        if (client.begin(remoteUrl)) {
            if (client.GET() == 200) {
                std::string resp = client.getString();
                uint32_t val = 0;
                try { val = std::stoul(resp); } catch (...) {}
                return HAL_JSON::HALValue(val);
            }
        }
    #else
        WiFiClient wifiClient;
        HTTPClient client;
        if (client.begin(wifiClient, remoteUrl.c_str())) {
            if (client.GET() == 200) {
                std::string resp = client.getString().c_str();
                uint32_t val = 0;
                try { val = std::stoul(resp); } catch (...) {}
                return HAL_JSON::HALValue(val);
            }
        }
    #endif
        return HAL_JSON::HALValue(0); // fallback on error
    }
}