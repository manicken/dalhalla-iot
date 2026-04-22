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
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include "DALHAL_AnalogInput_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>

namespace DALHAL {
    constexpr Registry::DefineBase AnalogInput::RegistryDefine = {
        Create,
        &JsonSchema::AnalogInput::Root,
        DALHAL_REACTIVE_EVENT_TABLE(ANALOG_INPUT)
    };
    
#if defined(ESP32) || defined(_WIN32)
    Device* AnalogInput::Create(DeviceCreateContext& context) {
        return new AnalogInput(context);
    }

    AnalogInput::AnalogInput(DeviceCreateContext& context) : AnalogInput_DeviceBase(context.deviceType) {
        uid = encodeUID(JsonSchema::GetValue(JsonSchema::CommonBase::uidFieldRequired, context).asConstChar());
        pin = JsonSchema::GetValue(JsonSchema::AnalogInput::pinField, context);
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
        val = (uint32_t)analogRead(pin);
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
#endif
	
}