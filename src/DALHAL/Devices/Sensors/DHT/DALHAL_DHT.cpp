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

#include "DALHAL_DHT.h"
#include <strings.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include "DALHAL_DHT_JSON_Schema.h"

#include <math.h>

namespace DALHAL {
    
    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase DHT::RegistryDefine = {
        Create,
        &JsonSchema::DHT::Root,
        DALHAL_REACTIVE_EVENT_TABLE(DHT),
        &DHT::FunctionTable
    };
    /* override */
    const Registry::DefineBase* DHT::GetRegistryDefine() {
        return &RegistryDefine;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> DHT::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(DHT::readHumidity, "read the default 'humidity' part of DHT"),
        DALHAL_FUNCTION_ENTRY("temp", DHT::readTemperature, "read the temperature part of DHT"),
        DALHAL_FUNCTION_ENTRY("humidity", DHT::readHumidity, "read the humidity part of DHT"),
        
    };

     __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::ReadString> DHT::readStringFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(DHT::readString__default__Function, "read both humidity and temperature, returned as json object"),
        DALHAL_FUNCTION_ENTRY("temp", DHT::readString_temperature_Function, "read the temperature part of DHT, returned as json object"),
        DALHAL_FUNCTION_ENTRY("humidity", DHT::readString_humidity_Function, "read the humidity part of DHT, returned as json object"),
    };

    constexpr DeviceFunctionTable DHT::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,

        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,

        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,

        DALHAL_FUNCTION_TABLE_ENTRY(readStringFunctions),
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };

    DHT::DHT(DeviceCreateContext& context) : DHTDeviceBase(context.deviceType) {
        JsonSchema::DHT::Extractors::Apply(context, this);
        lastUpdateMs = millis()-refreshTimeMs; // direct update
    }

    Device* DHT::Create(DeviceCreateContext& context) {
        return new DHT(context);
    }

    void DHT::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pin"), pin);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("humidity"), data.humidity);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("temperature"), data.temperature);

    }

    void DHT::loop() {
        uint32_t now = millis();
        if ((now - lastUpdateMs) >= refreshTimeMs) {
            lastUpdateMs = millis();
            TempAndHumidity tempData = dht.getTempAndHumidity(); // this could take up to 250mS (of what i have read, but the timing spec only make it to max ~23mS)
            if (!isnan(tempData.humidity) && !isnan(tempData.temperature)) {
                data = tempData;
                dataValid = true;
#if HAS_REACTIVE_VALUE_CHANGE(DHT)
                triggerValueChange();
#endif
            } else {
                //Serial.println("could not read DHT sensor");
            }
        }
    }

    /* static */
    HALOperationResult DHT::readTemperature(Device* device, HALValue& val) {
        DHT& dht = static_cast<DHT&>(*device);
        if (!dht.dataValid) return HALOperationResult::DataNotReady;
        val = dht.data.temperature;
        return HALOperationResult::Success;
    }
    /* static */
    HALOperationResult DHT::readHumidity(Device* device, HALValue& val) {
        DHT& dht = static_cast<DHT&>(*device);
        if (!dht.dataValid) return HALOperationResult::DataNotReady;
        val = dht.data.humidity;
        return HALOperationResult::Success;
    }

    HALOperationResult DHT::readString_temperature_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        //sbs.write_json_object_begin();
        sbs.write_jsonNumber(F("temp"),  static_cast<DHT*>(device)->data.temperature);
        //sbs.write_json_object_end();
        return HALOperationResult::Success;
    }

    HALOperationResult DHT::readString_humidity_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        //sbs.write_json_object_begin();
        sbs.write_jsonNumber(F("humidity"), static_cast<DHT*>(device)->data.humidity);
        //sbs.write_json_object_end();
        return HALOperationResult::Success;
    }

    HALOperationResult DHT::readString__default__Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        //sbs.write_json_object_begin();
        sbs.write_jsonNumber(F("humidity"), static_cast<DHT*>(device)->data.humidity);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("temp"),  static_cast<DHT*>(device)->data.temperature);
        //sbs.write_json_object_end();
        return HALOperationResult::Success;
    }
    
}