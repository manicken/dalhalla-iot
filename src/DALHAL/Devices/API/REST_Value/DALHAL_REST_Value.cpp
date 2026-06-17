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
#if defined(ESP32) || (defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
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
        &REST_Value::FunctionTable
    };

    /* override */
    const Registry::DefineBase* REST_Value::GetRegistryDefine() {
        return &RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> REST_Value::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "read value")
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable REST_Value::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };

    Device* REST_Value::Create(DeviceCreateContext& context) {
        return new REST_Value(context);
    }

    REST_Value::REST_Value(DeviceCreateContext& context) : DALHAL::Device(context.deviceType), lastRefresh(0)
    {
        JsonSchema::REST_Value::Extractors::Apply(context, this);
    }

    /* static */
    HALOperationResult REST_Value::HALValue_primary_read(Device* device, HALValue& val) {
        val = static_cast<REST_Value&>(*device).cachedValue;
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