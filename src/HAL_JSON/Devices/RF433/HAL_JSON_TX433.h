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

#include <ArduinoJson.h>
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_TX433unit.h"
#include "../HAL_JSON_DeviceTypesRegistry.h"

#define HAL_JSON_KEYNAME_TX433_UNITS "units"

namespace HAL_JSON {

    class TX433 : public Device {
    private:
        uint8_t pin = 0; // if pin would be used
        Device/*TX433unit*/** units;
        int unitCount;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            Create,
            VerifyJSON
        };
        TX433(const JsonVariant &jsonObj, const char* type);
        TX433(TX433&) = delete;
        ~TX433();
        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;
        HALOperationResult write(const HALWriteStringRequestValue &val);

        String ToString() override;
    };
}