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

#include "../../Core/HAL_JSON_Device.h"
#include "../HAL_JSON_DeviceTypesRegistry.h"

#include "HAL_JSON_HA_Constants.h"

#include <WiFiClient.h>
#include <PubSubClient.h>




#define HAL_JSON_HOME_ASSISTANT_DEFAULT_PORT 1883

namespace HAL_JSON {

    class HomeAssistant : public Device {
    private:
        std::string deviceID;
        std::string username;
        std::string password;
        std::string host; // just for debug???
        IPAddress ip;
        uint16_t port;

        WiFiClient wifiClient;
        PubSubClient mqttClient;

        Device** devices;
        int deviceCount;

        unsigned long lastReconnectAttempt = 0;
        const unsigned long reconnectInterval = 10000; // 10 seconds

        bool GetFlattenGroupsValidItem(const JsonVariant& jsonObj);
        void ConstructDevicesNonGrouped(const JsonVariant& jsonObj);
        void ConstructDevicesFromFlattenGroupsItems(const JsonVariant& jsonObj);

        void mqttCallback(char* topic, byte* payload, unsigned int length);

        void SubscribeToCommandTopic();

        void Connect();
        
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            Create,
            VerifyJSON
        };

        /** called regulary from the main loop */
        void loop() override;
        /** called when all hal devices has been loaded */
        void begin() override;
        /** used to find sub/leaf devices @ "group devices" */
        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;

        
        HomeAssistant(const JsonVariant &jsonObj, const char* type);
        ~HomeAssistant();


        String ToString() override;
    };
}