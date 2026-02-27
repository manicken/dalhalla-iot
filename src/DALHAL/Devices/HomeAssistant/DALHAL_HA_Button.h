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


#include "../../Core/Device/DALHAL_Device.h"

#include <WiFiClient.h>
#include <PubSubClient.h>
#include "../../Core/Device/DALHAL_CachedDeviceAccess.h"
#include "DALHAL_HA_TopicBasePath.h"
#include "DALHAL_HA_DeviceTypeReg.h"

namespace DALHAL {

    class Button : public Device {

    private:
        PubSubClient& mqttClient;
        CachedDeviceAccess* cda;
        TopicBasePath topicBasePath;

    public:

        static void SendDeviceDiscovery(PubSubClient& mqtt, const JsonVariant& jsonObj, TopicBasePath& topicBasePath);

        HALOperationResult exec(const ZeroCopyString& cmd) override;
        HALOperationResult exec() override;
        
        static bool VerifyJSON(const JsonVariant& jsonObj);
        static Device* Create(const JsonVariant& jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot);
        static constexpr HA_DeviceRegistryDefine RegistryDefine = {
            Create,
            VerifyJSON
        };
        Button(const JsonVariant& jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot);
        ~Button();


        String ToString() override;
    };
}