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
#pragma once

#include "HAL_JSON_RemoteFetch_REST.h"
#include <Arduino.h> // Needed for String class

#include <ArduinoJson.h>

#include <HTTPClient.h>
#include <string>

#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"


namespace HAL_JSON {

    class RemoteFetch_REST : public HAL_JSON::Device {
    public:
        RemoteFetch_REST(const char* type, const std::string& url, uint32_t refreshMs = 1000);

        HAL_JSON::HALOperationResult read(HAL_JSON::HALValue& val) override;
        void loop() override;

    private:
        HAL_JSON::HALValue cachedValue;
        std::string remoteUrl;
        uint32_t refreshTimeMs;
        uint32_t lastRefresh;

        HAL_JSON::HALValue fetchRemoteValue();
    };
}