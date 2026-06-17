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

#include "DALHAL_REGO600_Register.h"
#include <DALHAL/Drivers/REGO600.h>

#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include "DALHAL_REGO600_Register_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase REGO600_Register::RegistryDefine = {
        nullptr, // no Create function needed here
        &JsonSchema::REGO600_Register::Root,
        DALHAL_REACTIVE_EVENT_TABLE(REGO600_REGISTRY_ITEM),
        &REGO600_Register::FunctionTable
    };
    
    /* override */
    const Registry::DefineBase* REGO600_Register::GetRegistryDefine() {
        return &RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> REGO600_Register::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "read cached value")
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable REGO600_Register::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };
    
    REGO600_Register::REGO600_Register(DeviceCreateContext& context) : REGO600register_DeviceBase(context.deviceType) {
        JsonSchema::REGO600_Register::Extractors::Apply(context, this);
#if HAS_REACTIVE_VALUE_CHANGE(SCRIPT_VARIABLE)
        value.setCallbacks(this, GenericValueCallback<REGO600register_DeviceBase>, nullptr);
#endif
    }

    HALValue* REGO600_Register::GetValueDirectAccessPtr() {
        return &value;
    }

    /* static */
    HALOperationResult REGO600_Register::HALValue_primary_read(Device* device, HALValue& val) {
        val = static_cast<REGO600_Register&>(*device).value;
        return HALOperationResult::Success;
    }

    void REGO600_Register::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
    }

}