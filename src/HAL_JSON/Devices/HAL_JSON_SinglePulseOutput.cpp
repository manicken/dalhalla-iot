/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

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

#include "HAL_JSON_SinglePulseOutput.h"

namespace HAL_JSON {
    
    Device* SinglePulseOutput::Create(const JsonVariant &jsonObj, const char* type) {
        return new SinglePulseOutput(jsonObj, type);
    }

    bool SinglePulseOutput::VerifyJSON(const JsonVariant &jsonObj) {
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, static_cast<uint8_t>(GPIO_manager::PinMode::OUT));
    }

    SinglePulseOutput::SinglePulseOutput(const JsonVariant &jsonObj, const char* type) : Device(type) {
        pin = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PIN);// jsonObj[HAL_JSON_KEYNAME_PIN];// | 0;//.as<uint8_t>();
        uid = encodeUID(GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID));
        inactiveState = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_INACTIVE_STATE, 0);
        pulseLength = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT, 0);
        pinMode(pin, OUTPUT); // output
        digitalWrite(pin, inactiveState);
    }

    SinglePulseOutput::~SinglePulseOutput() { 
        pinMode(pin, INPUT); // input
        pulseTicker.detach();
    }

    HALOperationResult SinglePulseOutput::read(HALValue &val) {
        //val.set(value); // read back the latest write value
        val = pulseLength;
        return HALOperationResult::Success;
    }

    void SinglePulseOutput::pulseTicker_Callback(SinglePulseOutput* context) {
        context->endPulse();
    }

    HALOperationResult SinglePulseOutput::write(const HALValue &val) {
        if (val.getType() == HALValue::Type::TEST) { printf("\nSinglePulseOutput::write TEST\n"); return HALOperationResult::Success; }// test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;

        uint32_t t = val;
        if (t != 0) // only change if not zero
            pulseLength = t;//val.asUInt();
        if (pulseLength == 0) return HALOperationResult::ExecutionFailed; // no pulse

        return exec();
    }
    Device::Exec_FuncType SinglePulseOutput::GetExec_Function(ZeroCopyString& zcFuncName) {
        return SinglePulseOutput::exec;
    } 

    HALOperationResult SinglePulseOutput::exec(Device* device) {
        return static_cast<SinglePulseOutput*>(device)->exec(); // direct call no vtable
    }

    HALOperationResult SinglePulseOutput::exec() {
        digitalWrite(pin, !inactiveState);
        pulseTicker.detach();
        pulseTicker.once_ms(pulseLength, pulseTicker_Callback, this);
        return HALOperationResult::Success;
    }

    void SinglePulseOutput::endPulse() {
        digitalWrite(pin, inactiveState);
    }

    String SinglePulseOutput::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",\"pulseLength\":";
        ret += std::to_string(pulseLength).c_str();
        return ret;
    }
	
}