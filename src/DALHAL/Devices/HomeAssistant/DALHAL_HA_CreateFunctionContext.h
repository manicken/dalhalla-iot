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

#pragma once

#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <DALHAL/Core/Types/DALHAL_DeviceCreateContext.h>

namespace DALHAL {

    struct HA_CreateFunctionContext : DeviceCreateContext {
        // set from the jsonObjRoot as the HA devices dont have/should not have access to the base Schema
        const char* deviceId_cStr;

        PubSubClient& mqttClient;
        /** this need to be cleaner and whatever fields that are used from this need to be extracted using the schema at the set level */
        const JsonVariant* jsonGlobal;
        //const JsonVariant* jsonObjRoot; // cannot be used anymore as it breaks schema separation

        HA_CreateFunctionContext(PubSubClient& mqttClient) : DeviceCreateContext(), mqttClient(mqttClient), jsonGlobal(nullptr) {}
        //HA_CreateFunctionContext(const JsonVariant& jsonObj, const char* const type, PubSubClient& mqttClient, const JsonVariant& jsonGlobal, const JsonVariant& jsonObjRoot) : DeviceCreateContext(context), mqttClient(mqttClient), jsonGlobal(jsonGlobal), jsonObjRoot(jsonObjRoot) {}
    };
}