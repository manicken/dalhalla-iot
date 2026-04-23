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

    constexpr Registry::DefineBase SinglePulseOutput::RegistryDefine = {
        Create,
        &JsonSchema::SinglePulseOutput::Root,
        DALHAL_REACTIVE_EVENT_TABLE(SINGLE_PULSE_OUTPUT)
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

    HALOperationResult SinglePulseOutput::read(HALValue &val) {
        //val.set(value); // read back the latest write value
        val = pulseLength;
#if HAS_REACTIVE(SINGLE_PULSE_OUTPUT, READ)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }

    /*static*/void SinglePulseOutput::pulseTicker_Callback(SinglePulseOutput* context) {
        context->endPulse();
    }

    HALOperationResult SinglePulseOutput::write(const HALValue &val) {
        if (val.getType() == HALValue::Type::TEST) { /*printf("\nSinglePulseOutput::write TEST\n");*/ return HALOperationResult::Success; }// test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;

        uint32_t t = val;
        if (t != 0) {// only change if not zero
            
#if HAS_REACTIVE(SINGLE_PULSE_OUTPUT, VALUE_CHANGE)
            if (pulseLength != t) {
                triggerValueChange(); // increments change counter; consumers compare with their last seen value
            }
#endif
            pulseLength = t;
        }
#if HAS_REACTIVE(SINGLE_PULSE_OUTPUT, WRITE)
        HALOperationResult res = exec();
        if (res == HALOperationResult::Success) {
            triggerWrite();
        }
        return res;
#else
        return exec();
#endif
    }
    Device::Exec_FuncType SinglePulseOutput::GetExec_Function(ZeroCopyString& zcFuncName) {
        return SinglePulseOutput::static_exec;
    } 

    HALOperationResult SinglePulseOutput::static_exec(Device* device) {
        return static_cast<SinglePulseOutput*>(device)->exec(); // direct call no vtable
    }

    HALOperationResult SinglePulseOutput::exec() {
        if (pulseLength == 0) return HALOperationResult::ExecutionFailed; // pulse length not configured
        pulseTicker.detach();
        digitalWrite(pin, activeLevel);
        pulseTicker.once_ms(pulseLength, pulseTicker_Callback, this);
#if HAS_REACTIVE(SINGLE_PULSE_OUTPUT, EXEC)
        triggerExec();
#endif
        return HALOperationResult::Success;
    }

    void SinglePulseOutput::endPulse() {
        digitalWrite(pin, inactiveLevel);
        pulseTicker.detach();
    }

    String SinglePulseOutput::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",\"pulseLength\":";
        ret += std::to_string(pulseLength).c_str();
        return ret;
    }
	
}