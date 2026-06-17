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

#include "DALHAL_HA_Number.h"
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_DeviceDiscovery.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_PubSubClient_JsonWriter.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_CountingPubSubClient.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_Constants.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_CreateFunctionContext.h>

#include "DALHAL_HA_Number_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase HA_Number::RegistryDefine = {
        Create,
        &JsonSchema::HA_Number::Root,
    };
    
    /* override */
    const Registry::DefineBase* HA_Number::GetRegistryDefine() {
        return &RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> HA_Number::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "read from the target device, if it's defined")
    };

    constexpr FunctionEntry<FunctionTypes::WriteHALValue> HA_Number::writeValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY_WITH_VAL_TYPE(HALValue_primary_write, "write the value to HomeAssistant", FunctionValueType::_Number_),
    };

    constexpr DeviceFunctionTable HA_Number::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,

        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeValueFunctions),

        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,

        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };

    void HA_Number::SendDeviceDiscovery(PubSubClient& mqtt, const HA_DD_Context& ctx) {
        HA_DeviceDiscovery::SendCommandTopicCfg(mqtt, ctx); // adds , before
        HA_DeviceDiscovery::SendStateTopicCfg(mqtt, ctx); // adds , before
    }

    Device* HA_Number::Create(DeviceCreateContext& context) {
        return new HA_Number(static_cast<HA_CreateFunctionContext&>(context));
    }
    
    HA_Number::HA_Number(HA_CreateFunctionContext& context) : HA_DeviceEntity(context) {
        JsonSchema::HA_Number::Extractors::Apply(context, this);
    }

    HA_Number::~HA_Number() {
        delete cda;
    }

    void HA_Number::PrintTo(StringBuilderStreamer& sbs) {
        //String ret = Device::ToString();
        Device::PrintTo(sbs);       
        //return ret;
    }

    HALOperationResult HA_Number::HALValue_primary_read(Device* device, HALValue& val) {
        HA_Number& self = static_cast<HA_Number&>(*device);
        if (self.cda != nullptr) {
            return self.cda->ReadSimple(val);
        } else {
            val = self.currentValue;
            return HALOperationResult::Success;
        }
    }
    HALOperationResult HA_Number::HALValue_primary_write(Device* device, const HALValue& val) {
        HA_Number& self = static_cast<HA_Number&>(*device);

        if (val.isNaN()) return HALOperationResult::WriteValueNaN;

        self.currentValue = val;
        if (self.sendCurrentValue() == false) {
            return HALOperationResult::ExecutionFailed;
        }

        if (self.cda != nullptr) {
            return self.cda->WriteSimple(val);
        }
        return HALOperationResult::UnsupportedOperation;
    };

    HALOperationResult HA_Number::ha_apply(const ZeroCopyString& zcVal) {

        NumberResult numberRes = zcVal.ConvertStringToNumber();     
        HALValue valState;
        switch (numberRes.type) {
            case NumberType::UINT32:
                Serial.println("val type was uint");
                valState = numberRes.u32;
                break;
            case NumberType::INT32:
                Serial.println("val type was int");
                valState = numberRes.i32;
                break;
            case NumberType::FLOAT:
                Serial.println("val type was float");
                valState = numberRes.f32;
                break;
            default:
                // send back old value on fail
                if (sendCurrentValue() == false) {
                    return HALOperationResult::ExecutionFailed;
                }
                return HALOperationResult::WriteValueNaN;
        }

        HALOperationResult res = (cda!=nullptr)?cda->WriteSimple(valState):HALOperationResult::Success;
        if (res == HALOperationResult::Success) {
            //Serial.println("Number exec OK");
            currentValue = valState;
        } else {
            Serial.println("Number exec fail");
        }
        if (sendCurrentValue() == false) {
            return HALOperationResult::ExecutionFailed;
        }
        return res;
        
    }

    bool HA_Number::sendCurrentValue() {
        bool success = HA_DeviceDiscovery::SendState(mqttClient, hass_uid.c_str(), currentValue);
        if (success == false) {
            GlobalLogger.Error(F("could not send number state update to HASS"));
        }
        return success;
    }

}