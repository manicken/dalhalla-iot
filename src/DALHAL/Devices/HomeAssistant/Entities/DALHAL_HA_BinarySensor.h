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

#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_TopicBasePath.h>
#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_DeviceTypeReg.h>

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Device/DALHAL_CachedDeviceRead.h>

#define DALHAL_HA_SENSOR_DEFAULT_REFRESH_MS 5000

namespace DALHAL {

    class BinarySensor : public Device {
    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static bool VerifyJSON(const JsonVariant& jsonObj);
        static Device* Create(DeviceCreateContext& context);

    private:
        static void SendDeviceDiscovery(PubSubClient& mqtt, const JsonVariant& jsonObj, TopicBasePath& topicBasePath);
        
        PubSubClient& mqttClient;
        CachedDeviceRead* cdr;
        TopicBasePath topicBasePath;

        uint32_t refreshMs;
        uint32_t lastMs;
        bool wasOnline;

    public:
        BinarySensor(HA_CreateFunctionContext& context);
        ~BinarySensor();
        
        void begin() override;
        void loop() override;
        
        HALOperationResult read(HALValue& val) override;
        HALOperationResult write(const HALValue& val) override;

        String ToString() override;
    };
}