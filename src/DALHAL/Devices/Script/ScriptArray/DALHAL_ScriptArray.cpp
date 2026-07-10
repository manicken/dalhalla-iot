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

#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>

#include "DALHAL_ScriptArray_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase ScriptArray::RegistryDefine = {
        Create,
        &JsonSchema::ScriptArray::Root,
        DALHAL_REACTIVE_EVENT_TABLE(SCRIPT_ARRAY),
        &ScriptArray::FunctionTable
    };
    
    /* override */
    const Registry::DefineBase* ScriptArray::GetRegistryDefine() {
        return &RegistryDefine;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::BracketOpRead> ScriptArray::bracketOpReadFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(ScriptArray::BracketRead_Func, "primary")
    };

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::BracketOpWrite> ScriptArray::bracketOpWriteFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(ScriptArray::BracketWrite_Func, "primary")
    };

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::ReadString> ScriptArray::readStringFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(ScriptArray::readString_primary_Function, "get a item given by the first parameter as the index"),
        DALHAL_FUNCTION_ENTRY("valuelist", ScriptArray::readString_valuelist_Function, "get the whole list of values")
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable ScriptArray::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        EmptyFunctionTable<FunctionTypes::ReadToHALValue>,
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,
        DALHAL_FUNCTION_TABLE_ENTRY(bracketOpReadFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(bracketOpWriteFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(readStringFunctions),
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };
    
    ScriptArray::ScriptArray(DeviceCreateContext& context) : ScriptArray_DeviceBase(context.deviceType) {
        JsonSchema::ScriptArray::Extractors::Apply(context, this);
    }
    ScriptArray::~ScriptArray() {
        delete values;
    }

    Device* ScriptArray::Create(DeviceCreateContext& context) {
        return new ScriptArray(context);
    }

    void ScriptArray::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
    }
    // init array here
    void ScriptArray::begin() {

    }

    HALOperationResult ScriptArray::readString_valuelist_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        ScriptArray& self = static_cast<ScriptArray&>(*device);
        
        sbs.write_jsonMemberStart(F("items"));
        sbs.write_json_array_begin();
        for (int i=0;i<self.valueCount;i++) {
            if (i>0) {
                sbs.write_json_value_separator();
            }
            sbs.write(self.values[i]);
            //self.values[i].toString(sbs);
        }
        sbs.write_json_array_end();

        return HALOperationResult::Success;
    }
    /* static */
    HALOperationResult ScriptArray::readString_primary_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        ZeroCopyString zcIndex = zcStrParameters.SplitOffHead('/');
        if (zcIndex.ValidNumber() == false) { return HALOperationResult::InvalidArgument; }
        ScriptArray& self = *static_cast<ScriptArray*>(device);

        int32_t index = 0;
        if (zcIndex.ConvertTo_int32(index) == false) {
            sbs.write_jsonString(F("error"), F("invalid index not a integer"));
            return HALOperationResult::BracketOpSubscriptInvalid;
        }

        if (index < 0 || index >= self.valueCount) {
            sbs.write_jsonString(F("error"), F("invalid index out of range"));
            return HALOperationResult::BracketOpSubscriptOutOffRange;
        }
        sbs.write(self.values[index]);
        //self.values[index].toString(sbs);
        //val.out_value = values[index].toString();
        return HALOperationResult::Success;
    }
    /* static */
    HALOperationResult ScriptArray::BracketRead_Func(Device* device, const HALValue& bracketSubscriptVal, HALValue& val) {
        ScriptArray& self = static_cast<ScriptArray&>(*device);
        int index = bracketSubscriptVal.toInt();
        if (index < 0 || index >= self.valueCount) {
            printf("\nScriptArray::read BracketOpSubscriptOutOffRange:%d\n", index);
            return HALOperationResult::BracketOpSubscriptOutOffRange;
        }
        val = self.values[index];
#if HAS_REACTIVE_BRACKET_READ(SCRIPT_ARRAY)
        self.triggerBracketRead();
#endif
        return HALOperationResult::Success;
    }
    /* static */
    HALOperationResult ScriptArray::BracketWrite_Func(Device* device, const HALValue& bracketSubscriptVal, const HALValue& val) {
        ScriptArray& self = static_cast<ScriptArray&>(*device);
        if (self.readOnly) return HALOperationResult::UnsupportedOperation;

        int index = bracketSubscriptVal.toInt();
        if (index < 0 || index >= self.valueCount) {
            printf("\nScriptArray::write BracketOpSubscriptOutOffRange:%d\n", index);
            return HALOperationResult::BracketOpSubscriptOutOffRange;
        }
        self.values[index] = val;
#if HAS_REACTIVE_BRACKET_WRITE(SCRIPT_ARRAY)
        self.triggerBracketWrite();
#endif
        return HALOperationResult::Success;
    }

}