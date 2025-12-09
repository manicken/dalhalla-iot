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
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"
#include "../HAL_JSON_CachedDeviceRead.h"
#include "HAL_JSON_DeviceTypesRegistry.h"

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#elif defined(_WIN32) || defined(__linux__) || defined(__MAC__)
#include <WiFiClient.h>
#include <HTTPClient.h>
#endif

#include "../HAL_JSON_SimpleEventDevice.h"

#define DALHALLA_THINGSPEAK_MAX_FIELDS 8
#define HAL_JSON_TYPE_THINGSPEAK_DEFAULT_REFRESHRATE_MS 60*1000

namespace HAL_JSON {

    struct ThingSpeakField {
        int index;
        uint32_t valueChangedCounter;
        Device* eventCheckDevice;
        Device::EventCheck_FuncType eventFunc;
        CachedDeviceRead* cdr;

        inline bool DataReady() {
            if (eventFunc == nullptr || eventCheckDevice == nullptr) return true; // allways return true if no event device
            return eventFunc(eventCheckDevice, valueChangedCounter);
        }
    };

    class ThingSpeak : public Device {
    private:
        HTTPClient http;
        WiFiClient wifiClient;
        static const char TS_ROOT_URL[];
        char API_KEY[17];
        std::string urlApi;
        ThingSpeakField* fields;
        int fieldCount;
        bool useOwnTaskLoop = false;
        uint32_t refreshTimeMs = HAL_JSON_TYPE_THINGSPEAK_DEFAULT_REFRESHRATE_MS;
        uint32_t lastUpdateMs = 0;

    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            Create,
            VerifyJSON
        };
        ThingSpeak(const JsonVariant &jsonObj, const char* type);
        ~ThingSpeak();
        HALOperationResult exec() override;

        String ToString() override;

        void loop() override;
    };
}