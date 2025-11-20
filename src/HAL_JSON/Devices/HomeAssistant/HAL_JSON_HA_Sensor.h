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
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"

#include <WiFiClient.h>
#include <PubSubClient.h>
#include "../../HAL_JSON_CachedDeviceRead.h"

namespace HAL_JSON {

    class Sensor : public Device {
    private:
        PubSubClient& mqttClient;
        CachedDeviceRead* cdr;
        std::string state_topic;
        std::string availability_topic;
        uint32_t refreshMs;
        uint32_t lastMs;
        bool wasOnline;
    public:
        static void SendSpecificDeviceDiscovery(PubSubClient& mqttClient, const JsonVariant& jsonObj);
        static void SendDeviceDiscovery(PubSubClient& mqttClient, const JsonVariant& jsonObj, const JsonVariant& jsonObjGlobal);

        HALOperationResult read(HALValue& val) override;
        HALOperationResult write(const HALValue& val) override;
        
        /** called regulary from the main loop */
        void loop() override;
        /** called when all hal devices has been loaded */
        void begin() override;

        static bool VerifyJSON(const JsonVariant& jsonObj);
        static Device* Create(const JsonVariant& jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal);
        Sensor(const JsonVariant& jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal);
        ~Sensor();


        String ToString() override;
    };
}