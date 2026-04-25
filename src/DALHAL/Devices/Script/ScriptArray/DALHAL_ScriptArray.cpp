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

#include "DALHAL_ScriptArray.h"

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>


#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfPrimitives.h>

#include "DALHAL_ScriptArray_JSON_Schema.h"

namespace DALHAL {
    constexpr Registry::DefineBase ScriptArray::RegistryDefine = {
        Create,
        &JsonSchema::ScriptArray,
        DALHAL_REACTIVE_EVENT_TABLE(SCRIPT_ARRAY)
    };
    
    ScriptArray::ScriptArray(DeviceCreateContext& context) : ScriptArray_DeviceBase(context.deviceType) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
        uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*(context.jsonObjItem)));
        readOnly = JsonSchema::readonlyField.ExtractFrom(*(context.jsonObjItem));
        values = nullptr; // allways set it to nullptr
        if (JsonSchema::scriptArrayItems.ExtractValues(jsonObj, &values, valueCount) == false) {
            GlobalLogger.Error(F("Failed to extract script array items"));
        }
    }
    ScriptArray::~ScriptArray() {
        delete values;
    }

    Device* ScriptArray::Create(DeviceCreateContext& context) {
        return new ScriptArray(context);
    }

    String ScriptArray::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\"";
        return ret;
    }
    // init array here
    void ScriptArray::begin() {

    }

    HALOperationResult ScriptArray::read(const HALReadStringRequestValue& val) {

        if (val.cmd == "valuelist") {
            std::string ret;
            ret += '[';
            for (int i=0;i<valueCount;i++) {
                ret += values[i].toString();
                if (i<valueCount-1) ret += ',';
            }
            ret += ']';
            val.out_value = ret;
        } else if (val.cmd.ValidNumber()) {
            int32_t index = 0;
            if (val.cmd.ConvertTo_int32(index) == false) {
                val.out_value = "invalid index not a integer";
                return HALOperationResult::BracketOpSubscriptInvalid;
            }

            if (index < 0 || index >= valueCount) {
                val.out_value = "invalid index out of range";
                return HALOperationResult::BracketOpSubscriptOutOffRange;
            }

            val.out_value = values[index].toString();
        } else {
            val.out_value = "unknown command";
            return HALOperationResult::UnsupportedCommand;
        }
#if HAS_REACTIVE_READ(SCRIPT_ARRAY)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult ScriptArray::read(const HALValue& bracketSubscriptVal, HALValue& val) {
        int index = bracketSubscriptVal.asInt();
        if (index < 0 || index >= valueCount) {
            printf("\nScriptArray::read BracketOpSubscriptOutOffRange:%d\n", index);
            return HALOperationResult::BracketOpSubscriptOutOffRange;
        }
        val = values[index];
#if HAS_REACTIVE_BRACKET_READ(SCRIPT_ARRAY)
        triggerBracketRead();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult ScriptArray::write(const HALValue& bracketSubscriptVal, const HALValue& val) {
        if (readOnly) return HALOperationResult::UnsupportedOperation;

        int index = bracketSubscriptVal.asInt();
        if (index < 0 || index >= valueCount) {
            printf("\nScriptArray::write BracketOpSubscriptOutOffRange:%d\n", index);
            return HALOperationResult::BracketOpSubscriptOutOffRange;
        }
        values[index] = val;
#if HAS_REACTIVE_BRACKET_WRITE(SCRIPT_ARRAY)
        triggerBracketWrite();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult ScriptArray::BracketRead_Func(Device* device, const HALValue& bracketSubscriptVal, HALValue& val) {
        return static_cast<ScriptArray*>(device)->read(bracketSubscriptVal, val);
    }
    HALOperationResult ScriptArray::BracketWrite_Func(Device* device, const HALValue& bracketSubscriptVal, const HALValue& val) {
        return static_cast<ScriptArray*>(device)->write(bracketSubscriptVal, val);
    }
    
    Device::BracketOpRead_FuncType ScriptArray::GetBracketOpRead_Function(ZeroCopyString& zcFuncName) {
        return ScriptArray::BracketRead_Func; // functionname not used at the moment
    }

    Device::BracketOpWrite_FuncType ScriptArray::GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) {
        return ScriptArray::BracketWrite_Func; // functionname not used at the moment
    }


}