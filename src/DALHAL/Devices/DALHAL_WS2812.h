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

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>

#include "../Core/DALHAL_Device.h"
#include "DALHAL_DeviceTypesRegistry.h"

#include <WS2812FX.h>

namespace DALHAL {

    class WS2812 : public Device {
    private:
        uint8_t pin = 0; // if pin would be used
        
    public:
        WS2812FX* ws2812fx;
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            Create,
            VerifyJSON
        };

        WS2812(const JsonVariant &jsonObj, const char* type);

        HALOperationResult write(const HALWriteValueByCmd& val) override;
        HALOperationResult write(const HALWriteStringRequestValue& val) override;
        Device::WriteHALValue_FuncType GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) override;

        static HALOperationResult writeBrightness(Device* context, const HALValue& val);
        static HALOperationResult writeColor(Device* context, const HALValue& val);

        void loop() override;

        String ToString() override;
    };
}