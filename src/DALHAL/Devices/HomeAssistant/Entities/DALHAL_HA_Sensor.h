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

#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_TopicBasePath.h>
#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_DeviceTypeReg.h>
#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_CreateFunctionContext.h>

#include <Arduino.h> // Needed for String class
#include <string>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Device/DALHAL_CachedDeviceRead.h>
#include <DALHAL/Core/Reactive/DALHAL_ReactiveEvent.h>
#include <DALHAL/Core/Types/DALHAL_Consumer.h>



#define DALHAL_HA_SENSOR_DEFAULT_REFRESH_MS 5000

namespace DALHAL {

    namespace JsonSchema { namespace HA_Sensor { struct Extractors; } } // forward declaration

    class HA_Sensor : public Device {
        friend struct JsonSchema::HA_Sensor::Extractors; // allow access to private memebers of this class from the schema extractor

    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static Device* Create(DeviceCreateContext& context);

    private:
        static void SendDeviceDiscovery(PubSubClient& mqtt, TopicBasePath& topicBasePath);

        PubSubClient& mqttClient;
        TopicBasePath topicBasePath;
        
        Consumer::Mode consumerMode = Consumer::Mode::Manual;
        CachedDeviceRead* cdr = nullptr;
        ReactiveEvent* eventSource = nullptr;
        uint32_t refreshMs;
        uint32_t lastMs;
        bool wasOnline;

    private:
        bool IsTimedRefresh_NOT_Due();
        void SetAvailability(bool online);

    public:
        HA_Sensor(HA_CreateFunctionContext& context);
        ~HA_Sensor() override;

        void begin() override;
        void loop() override;

        HALOperationResult read(HALValue& val) override;
        HALOperationResult write(const HALValue& val) override;
   
        String ToString() override;
    };
}