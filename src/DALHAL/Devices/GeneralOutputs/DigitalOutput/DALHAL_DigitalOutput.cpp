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

#include "DALHAL_DigitalOutput.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include "DALHAL_DigitalOutput_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase DigitalOutput::RegistryDefine = {
        Create,
        &JsonSchema::DigitalOutput::Root,
        DALHAL_REACTIVE_EVENT_TABLE(DIGITAL_OUTPUT),
        &DigitalOutput::FunctionTable
    };
    
    /* override */
    const Registry::DefineBase* DigitalOutput::GetRegistryDefine() {
        return &RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> DigitalOutput::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "read back the latest write value")
    };

    constexpr FunctionEntry<FunctionTypes::WriteHALValue> DigitalOutput::writeValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY_WITH_VAL_TYPE(HALValue_primary_write, "write binary state", FunctionValueType::_Bool_),
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable DigitalOutput::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeValueFunctions),
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };
    
    Device* DigitalOutput::Create(DeviceCreateContext& context) {
        return new DigitalOutput(context);
    }

    DigitalOutput::DigitalOutput(DeviceCreateContext& context) : DigitalOutput_DeviceBase(context.deviceType) {
        JsonSchema::DigitalOutput::Extractors::Apply(context, this);
        pinMode(pin, OUTPUT); // output
    }

    DigitalOutput::~DigitalOutput() { pinMode(pin, INPUT); /*input*/ } // release the pin

    HALOperationResult DigitalOutput::HALValue_primary_read(Device* device, HALValue& val) {
        DigitalOutput& self = static_cast<DigitalOutput&>(*device);

        val = self.value;
#if HAS_REACTIVE_READ(DIGITAL_OUTPUT)
        self.triggerRead();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult DigitalOutput::HALValue_primary_write(Device* device, const HALValue& val) {
        DigitalOutput& self = static_cast<DigitalOutput&>(*device);

        if (!val.isBoolCompatible()) return HALOperationResult::WriteValueNaN;
        uint32_t newValue = val.toUInt();
#if HAS_REACTIVE_VALUE_CHANGE(DIGITAL_OUTPUT)
        if (self.value != newValue) {
            self.triggerValueChange();
        }
#endif
        self.value = newValue;
        digitalWrite(self.pin, self.value);
#if HAS_REACTIVE_WRITE(DIGITAL_OUTPUT)
        self.triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    void DigitalOutput::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pin"), pin);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("value"), value);
        
    }
	
}