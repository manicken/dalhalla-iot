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
    constexpr FunctionEntry<DeviceFunctionTable::ReadToHALValue_FuncType> DHT::readValueFunctions[] = {
        {"temp", &DHT::readTemperature, "read the temperature part of DHT"},
        {"humidity", &DHT::readHumidity, "read the humidity part of DHT"},
        {"", &DHT::readHumidity, "read the default 'humidity' part of DHT"},
    };

     __attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::ReadString_FuncType> DHT::readStringFunctions[] = {
        {"temp", &DHT::readString_temperature_Function, "read the temperature part of DHT, returned as json object"},
        {"humidity", &DHT::readString_humidity_Function, "read the humidity part of DHT, returned as json object"},
        {"", &DHT::readString__default__Function, "read both humidity and temperature, returned as json object"},
    };

    constexpr DeviceFunctionTable DHT::FunctionTable = {
        EmptyFunctionTable<DeviceFunctionTable::Exec_FuncType>,

        {readValueFunctions, sizeof(readValueFunctions) / sizeof(readValueFunctions[0])},
        EmptyFunctionTable<DeviceFunctionTable::WriteHALValue_FuncType>,

        EmptyFunctionTable<DeviceFunctionTable::BracketOpRead_FuncType>,
        EmptyFunctionTable<DeviceFunctionTable::BracketOpWrite_FuncType>,

        {readStringFunctions, sizeof(readStringFunctions) / sizeof(readStringFunctions[0])},
        EmptyFunctionTable<DeviceFunctionTable::WriteString_FuncType>,
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

    HALOperationResult DHT::read(HALValue &val) {
        if (!dataValid) return HALOperationResult::DataNotReady;
        val = data.humidity;
        return HALOperationResult::Success;
    }

    Device::ReadToHALValue_FuncType DHT::GetReadToHALValue_Function(ZeroCopyString& zcFuncName) {
        if (zcFuncName.EqualsIC(F("temp"))) {
            return DHT::readTemperature;
        } else if (zcFuncName.EqualsIC(F("humidity"))) {
            return DHT::readHumidity;
        }
        else {
            if (zcFuncName.Length() != 0) {
                GlobalLogger.Warn(F("DHT::read - cmd not found: "), zcFuncName.ToString().c_str()); // this can then be read by getting the last entry from logger
            }
            return nullptr;
        }
    }

    /* static */
    HALOperationResult DHT::readTemperature(Device* device, HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) { return HALOperationResult::Success; }
        DHT& dht = *static_cast<DHT*>(device);
        if (!dht.dataValid) return HALOperationResult::DataNotReady;
        val = dht.data.temperature;
        return HALOperationResult::Success;
    }
    /* static */
    HALOperationResult DHT::readHumidity(Device* device, HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) { return HALOperationResult::Success; }
        DHT& dht = *static_cast<DHT*>(device);
        if (!dht.dataValid) return HALOperationResult::DataNotReady;
        val = dht.data.humidity;
        return HALOperationResult::Success;
    }

    HALOperationResult DHT::read(const HALReadValueByCmd &val) {
        if (val.out_value.getType() == HALValue::Type::TEST) { return HALOperationResult::Success; }
        if (!dataValid) return HALOperationResult::DataNotReady;

        DeviceFunctionTable::ReadToHALValue_FuncType fn = GetDeviceFunction<DeviceFunctionTable::ReadToHALValue_FuncType>(FunctionTable.readValue, val.cmd);
        if (fn == nullptr) { return HALOperationResult::UnsupportedCommand; }
        return fn(this, val.out_value);
    }

    HALOperationResult DHT::read(const HALReadStringRequestValue &val) {
        DeviceFunctionTable::ReadString_FuncType fn = GetDeviceFunction<DeviceFunctionTable::ReadString_FuncType>(FunctionTable.readString, val.cmd);
        if (fn == nullptr) { GlobalLogger.Error(F("DHT - unsupported cmd:"), val.cmd); return HALOperationResult::UnsupportedCommand; }
        return fn(this, val.parameters, val.sbs);
    }

    HALOperationResult DHT::readString_temperature_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        sbs.write_json_object_begin();
        sbs.write_jsonNumber(F("temp"),  static_cast<DHT*>(device)->data.temperature);
        sbs.write_json_object_end();
        return HALOperationResult::Success;
    }

    HALOperationResult DHT::readString_humidity_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        sbs.write_json_object_begin();
        sbs.write_jsonNumber(F("humidity"), static_cast<DHT*>(device)->data.humidity);
        sbs.write_json_object_end();
        return HALOperationResult::Success;
    }

    HALOperationResult DHT::readString__default__Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        sbs.write_json_object_begin();
        sbs.write_jsonNumber(F("humidity"), static_cast<DHT*>(device)->data.humidity);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("temp"),  static_cast<DHT*>(device)->data.temperature);
        sbs.write_json_object_end();
        return HALOperationResult::Success;
    }
    
}