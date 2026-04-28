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
        DALHAL_REACTIVE_EVENT_TABLE(DIGITAL_OUTPUT)
    };
    //volatile const void* keep_DigitalOutput = &DALHAL::DigitalOutput::RegistryDefine;
    
    Device* DigitalOutput::Create(DeviceCreateContext& context) {
        return new DigitalOutput(context);
    }

    DigitalOutput::DigitalOutput(DeviceCreateContext& context) : DigitalOutput_DeviceBase(context.deviceType) {
        JsonSchema::DigitalOutput::Extractors::Apply(context, this);
        pinMode(pin, OUTPUT); // output
    }

    DigitalOutput::~DigitalOutput() { pinMode(pin, INPUT); /*input*/ } // release the pin

    HALOperationResult DigitalOutput::read(HALValue &val) {
        //val.set(value); // read back the latest write value
        val = value;
#if HAS_REACTIVE(DIGITAL_OUTPUT, READ)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult DigitalOutput::write(const HALValue &val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
#if HAS_REACTIVE_VALUE_CHANGE(DIGITAL_OUTPUT)
        if (value != val.asUInt()) {
            triggerValueChange();
        }
#endif
        value = val;//val.asUInt();
        digitalWrite(pin, value);
#if HAS_REACTIVE_WRITE(DIGITAL_OUTPUT)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    String DigitalOutput::ToString() {
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
        ret += std::to_string(value).c_str();
        return ret;
    }
	
}