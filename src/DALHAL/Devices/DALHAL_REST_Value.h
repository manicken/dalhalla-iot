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

#include "DALHAL_REST_Value.h"


#include <ArduinoJson.h>

#include <HTTPClient.h>
#include <string>

#include <Arduino.h> // Needed for String class
#include "../Core/DALHAL_Device.h"
#include "DALHAL_DeviceTypesRegistry.h"

namespace DALHAL {

    class REST_Value : public DALHAL::Device {
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            Create,
            VerifyJSON
        };

        REST_Value(const JsonVariant &jsonObj, const char* type);

        DALHAL::HALOperationResult read(DALHAL::HALValue& val) override;
        void loop() override;

    private:
        DALHAL::HALValue cachedValue;
        String remoteUrl;
        uint32_t refreshTimeMs;
        uint32_t lastRefresh;

        void fetchRemoteValue();
    };
}