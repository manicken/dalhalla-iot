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

#include <DALHAL/Core/Types/DALHAL_Registry.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//#include <PubSubClient.h> // uses a modded PubSubClient placed in lib folder

//#include <DALHAL/Core/Device/DALHAL_Device.h>


namespace DALHAL {

    /*typedef Device* (*HA_HAL_DEVICE_CREATE_FUNC)(const JsonVariant &json, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonGlobal, const JsonVariant& jsonObjRoot);
    typedef bool (*HA_HAL_DEVICE_VERIFY_JSON_FUNC)(const JsonVariant &json);

    typedef struct HA_Registry::Define {
        HA_HAL_DEVICE_CREATE_FUNC Create_Function;
        HA_HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function;
    } HA_Registry::Define;

    typedef struct HA_DeviceRegistryItem {
        const char* typeName;
        HA_Registry::Define def;
    } HA_DeviceRegistryItem ;

    
    const HA_DeviceRegistryItem& Get_HA_DeviceRegistryItem(const char* type);
*/

    extern const Registry::Item HA_DeviceRegistry[];

    struct HA_CreateFunctionContext /*: DeviceCreateContext */{
        PubSubClient& mqttClient;
        const JsonVariant& jsonGlobal;
        const JsonVariant& jsonObjRoot; // thing this need to mutable to avoid the need to recreate contexts
        HA_CreateFunctionContext(PubSubClient& mqttClient, const JsonVariant& jsonGlobal, const JsonVariant& jsonObjRoot) : mqttClient(mqttClient), jsonGlobal(jsonGlobal), jsonObjRoot(jsonObjRoot) {}
        //HA_CreateFunctionContext(const JsonVariant& jsonObj, const char* const type, PubSubClient& mqttClient, const JsonVariant& jsonGlobal, const JsonVariant& jsonObjRoot) : DeviceCreateContext(jsonObj, type), mqttClient(mqttClient), jsonGlobal(jsonGlobal), jsonObjRoot(jsonObjRoot) {}
    };
}