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

#include "DALHAL_ScriptVariable.h"

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include "DALHAL_ScriptVariable_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase ScriptVariable::RegistryDefine = {
        Create,
        &JsonSchema::ScriptVariable::Root,
        DALHAL_REACTIVE_EVENT_TABLE(SCRIPT_VARIABLE),
        &ScriptVariable::FunctionTable
    };

    /* override */
    const Registry::DefineBase* ScriptVariable::GetRegistryDefine() {
        return &RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> ScriptVariable::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "read value")
    };

    constexpr FunctionEntry<FunctionTypes::WriteHALValue> ScriptVariable::writeValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_write, "write value"),
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable ScriptVariable::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeValueFunctions),
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };

    ScriptVariable::ScriptVariable(DeviceCreateContext& context) : ScriptVariable_DeviceBase(context.deviceType) {
        JsonSchema::ScriptVariable::Extractors::Apply(context, this);
#if HAS_REACTIVE_VALUE_CHANGE(SCRIPT_VARIABLE)
        value.setCallbacks(this, GenericValueCallback<ScriptVariable_DeviceBase>, nullptr);
#endif
    }

    Device* ScriptVariable::Create(DeviceCreateContext& context) {
        return new ScriptVariable(context);
    }

    HALValue* ScriptVariable::GetValueDirectAccessPtr() {
        return &value;
    }
    /* static */
    HALOperationResult ScriptVariable::HALValue_primary_read(Device* device, HALValue& val) {
        ScriptVariable& self = static_cast<ScriptVariable&>(*device);
        val = self.value;
#if HAS_REACTIVE_READ(SCRIPT_VARIABLE)
        self.triggerRead();
#endif
        return HALOperationResult::Success;
    }
    /* static */
    HALOperationResult ScriptVariable::HALValue_primary_write(Device* device, const HALValue& val) {
        ScriptVariable& self = static_cast<ScriptVariable&>(*device);

        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        self.value = val;
#if HAS_REACTIVE_WRITE(SCRIPT_VARIABLE)
        self.triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    void ScriptVariable::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("value"));
        sbs.write(value);
       // value.toString(sbs);

    }

}