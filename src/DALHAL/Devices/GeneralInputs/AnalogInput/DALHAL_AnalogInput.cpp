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
        DALHAL_REACTIVE_EVENT_TABLE(ANALOG_INPUT)
    };
    //volatile const void* keep_AnalogInput = &DALHAL::AnalogInput::RegistryDefine;
    
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
        // the following can be used when analog read task is implemented
        // to signal that the value has been read
#if HAS_REACTIVE_CYCLE_COMPLETE(ANALOG_INPUT)
        triggerCycleComplete();
#endif
    }

    HALOperationResult AnalogInput::read(HALValue &val) {
        //val.set((uint32_t)analogRead(pin));
#if defined(ESP32)
        val = (uint32_t)analogRead(pin);
#endif

#if HAS_REACTIVE_READ(ANALOG_INPUT)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }

    String AnalogInput::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",";
        ret += DeviceConstStrings::value;//StartWithComma;
        ret += std::to_string(analogRead(pin)).c_str();
        return ret;
    }

	
}