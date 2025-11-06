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
#include "../HAL_JSON_Value.h"
#include "../HAL_JSON_Device.h"

#include "../HAL_JSON_Device_GlobalDefines.h"


namespace HAL_JSON {
    typedef Device* (*HAL_DEVICE_CREATE_FUNC)(const JsonVariant &json, const char* type);
    typedef bool (*HAL_DEVICE_VERIFY_JSON_FUNC)(const JsonVariant &json);

    enum class UseRootUID {
        Mandatory,
        Optional,
        Void
    };

    typedef struct DeviceTypeDef {
        UseRootUID useRootUID;
        const char* typeName;
        HAL_DEVICE_CREATE_FUNC Create_Function;
        HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function;
    } DeviceTypeDef ;

    extern const DeviceTypeDef DeviceRegistry[];

    const DeviceTypeDef* GetDeviceTypeDef(const char* type);
}