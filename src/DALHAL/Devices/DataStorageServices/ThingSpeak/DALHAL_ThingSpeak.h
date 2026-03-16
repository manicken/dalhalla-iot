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

#include <DALHAL/Core/Device/DALHAL_CachedDeviceRead.h>
#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#elif defined(_WIN32) || defined(__linux__) || defined(__MAC__)
#include <WiFiClient.h>
#include <HTTPClient.h>
#endif

#include "DALHAL_ThingSpeakField.h"

#define DALHAL_THINGSPEAK_MAX_FIELDS 8
#define DALHAL_TYPE_THINGSPEAK_DEFAULT_REFRESHRATE_MS 60*1000

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(THINGSPEAK)
#include "DALHAL_ThingSpeak_Reactive.h"
using ThingSpeak_DeviceBase = DALHAL::ThingSpeak_Reactive;
#else
using ThingSpeak_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    class ThingSpeak : public ThingSpeak_DeviceBase {
    public: // static fields and exposed external structures
        static const Registry::DefineRoot RegistryDefine;
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(DeviceCreateContext& context);

        static const char* TS_ROOT_URL;

    private:
        HTTPClient http;
        WiFiClient wifiClient;
        
        char API_KEY[17];
        std::string urlApi;
        ThingSpeakField* fields;
        int fieldCount;
        bool useOwnTaskLoop = false;
        uint32_t refreshTimeMs = DALHAL_TYPE_THINGSPEAK_DEFAULT_REFRESHRATE_MS;
        uint32_t lastUpdateMs = 0;

    public:
        ThingSpeak(DeviceCreateContext& context);
        ~ThingSpeak();
        HALOperationResult exec() override;

        String ToString() override;

        void loop() override;
    };
}