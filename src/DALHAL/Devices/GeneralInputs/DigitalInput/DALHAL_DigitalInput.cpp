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

#include "DALHAL_DigitalInput.h"

#include <DALHAL/Support/DALHAL_Logger.h>

#include "DALHAL_DigitalInput_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase DigitalInput::RegistryDefine = {
        Create,
        &JsonSchema::DigitalInput::Root,
        DALHAL_REACTIVE_EVENT_TABLE(DIGITAL_INPUT),
        &DigitalInput::FunctionTable
    };
    
    /* override */
    const Registry::DefineBase* DigitalInput::GetRegistryDefine() {
        return &RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> DigitalInput::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "read digital value")
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable DigitalInput::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };
    
    Device* DigitalInput::Create(DeviceCreateContext& context) {
        return new DigitalInput(context);
    }

    DigitalInput::DigitalInput(DeviceCreateContext& context) : DigitalInput_DeviceBase(context.deviceType) {
        JsonSchema::DigitalInput::Extractors::Apply(context, this);
        pinMode(pin, INPUT);
    }

    /* static */
    HALOperationResult DigitalInput::HALValue_primary_read(Device* device, HALValue& val) {
        DigitalInput& self = static_cast<DigitalInput&>(*device);

        val = (uint32_t)digitalRead(self.pin);
#if HAS_REACTIVE_READ(DIGITAL_INPUT)
        self.triggerRead();
#endif
        return HALOperationResult::Success;
    }

    void DigitalInput::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pin"), pin);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("value"), digitalRead(pin));
    }
	
}