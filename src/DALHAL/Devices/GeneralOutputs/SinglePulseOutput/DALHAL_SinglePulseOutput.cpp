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

#include "DALHAL_SinglePulseOutput.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include "DALHAL_SinglePulseOutput_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase SinglePulseOutput::RegistryDefine = {
        Create,
        &JsonSchema::SinglePulseOutput::Root,
        DALHAL_REACTIVE_EVENT_TABLE(SINGLE_PULSE_OUTPUT),
        &SinglePulseOutput::FunctionTable
    };

    /* override */
    const Registry::DefineBase* SinglePulseOutput::GetRegistryDefine() {
        return &RegistryDefine;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::Exec> SinglePulseOutput::execFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(SinglePulseOutput::primary_exec, "execute a pulse")
    };

    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> SinglePulseOutput::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "read back the latest used pulse length")
    };

    constexpr FunctionEntry<FunctionTypes::WriteHALValue> SinglePulseOutput::writeValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY_WITH_VAL_TYPE(HALValue_primary_write, "start a pulse with the given length", FunctionValueType::_UInt_),
    };

    constexpr DeviceFunctionTable SinglePulseOutput::FunctionTable = {
        DALHAL_FUNCTION_TABLE_ENTRY(execFunctions),

        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeValueFunctions),

        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,

        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };
    
    Device* SinglePulseOutput::Create(DeviceCreateContext& context) {
        return new SinglePulseOutput(context);
    }

    SinglePulseOutput::SinglePulseOutput(DeviceCreateContext& context) : SinglePulseOutput_DeviceBase(context.deviceType) {
        JsonSchema::SinglePulseOutput::Extractors::Apply(context, this);
        // use the following to make sure that it's explicit defined states
        if (activeLevel == 1) inactiveLevel = 0;
        else inactiveLevel = 1;
        // pulseLength is optional as it can be given by the write function
        pinMode(pin, OUTPUT); // output
        digitalWrite(pin, inactiveLevel);
    }

    SinglePulseOutput::~SinglePulseOutput() { 
        pinMode(pin, INPUT); // input
        pulseTicker.detach();
    }

    HALOperationResult SinglePulseOutput::HALValue_primary_read(Device* device, HALValue& val) {
        SinglePulseOutput& self = *static_cast<SinglePulseOutput*>(device);
        val = self.pulseLength;
#if HAS_REACTIVE(SINGLE_PULSE_OUTPUT, READ)
        self.triggerRead();
#endif
        return HALOperationResult::Success;
    }

    /*static*/
    void SinglePulseOutput::pulseTicker_Callback(SinglePulseOutput* context) {
        context->endPulse();
    }

    HALOperationResult SinglePulseOutput::HALValue_primary_write(Device* device, const HALValue& val) {
        SinglePulseOutput& self = *static_cast<SinglePulseOutput*>(device);
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;

        uint32_t t = val.toUInt();
        if (t != 0) {// only change if not zero
            
#if HAS_REACTIVE(SINGLE_PULSE_OUTPUT, VALUE_CHANGE)
            if (self.pulseLength != t) {
                self.triggerValueChange(); // increments change counter; consumers compare with their last seen value
            }
#endif
            self.pulseLength = t;
        }
#if HAS_REACTIVE(SINGLE_PULSE_OUTPUT, WRITE)
        HALOperationResult res = SinglePulseOutput::primary_exec(&self);
        if (res == HALOperationResult::Success) {
            self.triggerWrite();
        }
        return res;
#else
        return exec();
#endif
    }

    /* static */
    HALOperationResult SinglePulseOutput::primary_exec(Device* device) {
        SinglePulseOutput& self = *static_cast<SinglePulseOutput*>(device);

        if (self.pulseLength == 0) return HALOperationResult::ExecutionFailed; // pulse length not configured
        self.pulseTicker.detach();
        digitalWrite(self.pin, self.activeLevel);
        self.pulseTicker.once_ms(self.pulseLength, pulseTicker_Callback, &self);
#if HAS_REACTIVE(SINGLE_PULSE_OUTPUT, EXEC)
        self.triggerExec();
#endif
        return HALOperationResult::Success;
    }

    void SinglePulseOutput::endPulse() {
        digitalWrite(pin, inactiveLevel);
        pulseTicker.detach();
    }

    void SinglePulseOutput::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pin"), pin);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pulseLength"), pulseLength);
    }
	
}