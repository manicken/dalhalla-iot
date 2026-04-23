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

    constexpr Registry::DefineBase DigitalInput::RegistryDefine = {
        Create,
        &JsonSchema::DigitalInput::Root,
        DALHAL_REACTIVE_EVENT_TABLE(DIGITAL_INPUT)
    };
    
    Device* DigitalInput::Create(DeviceCreateContext& context) {
        return new DigitalInput(context);
    }

    DigitalInput::DigitalInput(DeviceCreateContext& context) : DigitalInput_DeviceBase(context.deviceType) {
        JsonSchema::DigitalInput::Extractors::Apply(context, this);
        pinMode(pin, INPUT);
    }

    HALOperationResult DigitalInput::read(HALValue &val) {
        //val.set((uint32_t)digitalRead(pin));
        val = (uint32_t)digitalRead(pin);
#if HAS_REACTIVE_READ(DIGITAL_INPUT)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }

    String DigitalInput::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",";
        ret += DeviceConstStrings::value;//StartWithComma;
        ret += std::to_string(digitalRead(pin)).c_str();
        return ret;
    }
	
}