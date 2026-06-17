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

#include "DALHAL_AnalogInput.h"

#include <DALHAL/Support/DALHAL_Logger.h>

#include "DALHAL_AnalogInput_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase AnalogInput::RegistryDefine = {
        Create,
        &JsonSchema::AnalogInput::Root,
        DALHAL_REACTIVE_EVENT_TABLE(ANALOG_INPUT),
        &AnalogInput::FunctionTable
    };

    /* override */
    const Registry::DefineBase* AnalogInput::GetRegistryDefine() {
        return &RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> AnalogInput::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "read analog value")
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable AnalogInput::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };
    
    Device* AnalogInput::Create(DeviceCreateContext& context) {
        return new AnalogInput(context);
    }

    AnalogInput::AnalogInput(DeviceCreateContext& context) : AnalogInput_DeviceBase(context.deviceType) {
        JsonSchema::AnalogInput::Extractors::Apply(context, this);
        pinMode(pin, INPUT);
    }

    AnalogInput::~AnalogInput() { pinMode(pin, INPUT); } // input

    void AnalogInput::loop() {
        return;

        // TODO implement analog read task by refreshtime

        // the following can be used when analog read task is implemented
        // to signal that the value has been read
#if HAS_REACTIVE_CYCLE_COMPLETE(ANALOG_INPUT)
        triggerCycleComplete();
#endif
    }

    HALOperationResult AnalogInput::HALValue_primary_read(Device* device, HALValue& val) {
#if defined(ESP32) // and devices supporting analog read, normally analogInput are not available for devices not supporting it
        AnalogInput& self = static_cast<AnalogInput&>(*device);
        val = (uint32_t)analogRead(self.pin);
#if HAS_REACTIVE_READ(ANALOG_INPUT)
        self.triggerRead();
#endif
        return HALOperationResult::Success;
#else
        return HALOperationResult::ExecutionFailed;
#endif
    }

    void AnalogInput::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
        
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pin"), pin);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("value"), analogRead(pin));
    }

	
}