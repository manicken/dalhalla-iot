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

#include "DALHAL_REST_Value.h"
#if defined(ESP32)
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#endif

#include <DALHAL/Support/DALHAL_Logger.h>
#include "DALHAL_REST_Value_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase REST_Value::RegistryDefine = {
        Create,
        &JsonSchema::REST_Value::Root,
        nullptr /* no events available on obsolete device*/
    };
    //volatile const void* keep_REST_Value = &DALHAL::REST_Value::RegistryDefine;

    Device* REST_Value::Create(DeviceCreateContext& context) {
        return new REST_Value(context);
    }

    REST_Value::REST_Value(DeviceCreateContext& context) : DALHAL::Device(context.deviceType), lastRefresh(0)
    {
        JsonSchema::REST_Value::Extractors::Apply(context, this);
    }

    DALHAL::HALOperationResult REST_Value::read(DALHAL::HALValue& val) {
        val = cachedValue;
        return DALHAL::HALOperationResult::Success;
    }

    void REST_Value::loop() {
        uint32_t now = millis();
        if (now - lastRefresh >= refreshTimeMs) {
            lastRefresh = now;
            fetchRemoteValue();
        }
    }

    void REST_Value::fetchRemoteValue() {
        int getCode = 0;
        HTTPClient client;
        WiFiClient wifiClient;
        if (client.begin(wifiClient, remoteUrl)) {
            getCode = client.GET();
        }

        if (getCode != 200) {
#if defined(_WIN32) || defined(__linux__)
            printf("\n[RemoteFetch_REST] Failed to fetch (%d): %s\n", getCode, remoteUrl.c_str());
#endif
            return;
        }
        String resp = client.getString();
        ZeroCopyString zcStrData(resp.c_str());
        if (zcStrData.FindString("\"error\"") != nullptr) {
            return;
        }
        if (zcStrData.FindString("\"value\"") == nullptr) {
            return;
        }
        zcStrData.SplitOffHead(':');
        zcStrData.end--; // trim off the last }
        float val = 0;
        if (zcStrData.ConvertTo_float(val)) {
            cachedValue = val;
        }
    }
}