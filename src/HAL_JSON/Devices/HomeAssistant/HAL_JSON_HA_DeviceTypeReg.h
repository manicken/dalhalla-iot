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

#include "../../HAL_JSON_Device.h"
#include <PubSubClient.h>

namespace HAL_JSON {

    typedef Device* (*HA_HAL_DEVICE_CREATE_FUNC)(const JsonVariant &json, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonGlobal, const JsonVariant& jsonObjRoot);
    typedef bool (*HA_HAL_DEVICE_VERIFY_JSON_FUNC)(const JsonVariant &json);
    //typedef void (*HA_HAL_DEVICE_SEND_DISCOVERY_FUNC)(PubSubClient& mqttClient, const JsonVariant &json);

    typedef struct HA_DeviceTypeDef {
        const char* typeName;
        HA_HAL_DEVICE_CREATE_FUNC Create_Function;
        HA_HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function;
        //HA_HAL_DEVICE_SEND_DISCOVERY_FUNC SendDiscovery_Function;
    } HA_DeviceTypeDef ;

    extern const HA_DeviceTypeDef HA_DeviceRegistry[];
    const HA_DeviceTypeDef* Get_HA_DeviceTypeDef(const char* type);

}