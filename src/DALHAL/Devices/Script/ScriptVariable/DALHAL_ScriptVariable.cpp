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

#include "DALHAL_ScriptVariable.h"

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include "DALHAL_ScriptVariable_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase ScriptVariable::RegistryDefine = {
        Create,
        &JsonSchema::ScriptVariable::Root,
        DALHAL_REACTIVE_EVENT_TABLE(SCRIPT_VARIABLE)
    };
    //volatile const void* keep_ScriptVariable = &DALHAL::ScriptVariable::RegistryDefine;

    ScriptVariable::ScriptVariable(DeviceCreateContext& context) : ScriptVariable_DeviceBase(context.deviceType) {
        JsonSchema::ScriptVariable::Extractors::Apply(context, this);
#if HAS_REACTIVE_VALUE_CHANGE(SCRIPT_VARIABLE)
        value.setCallbacks(this, GenericValueCallback<ScriptVariable_DeviceBase>, nullptr);
#endif
    }

    Device* ScriptVariable::Create(DeviceCreateContext& context) {
        return new ScriptVariable(context);
    }

    HALValue* ScriptVariable::GetValueDirectAccessPtr() {
        return &value;
    }

    HALOperationResult ScriptVariable::read(HALValue& val) {
        val = value;
#if HAS_REACTIVE_READ(SCRIPT_VARIABLE)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }
    HALOperationResult ScriptVariable::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        value = val;
#if HAS_REACTIVE_WRITE(SCRIPT_VARIABLE)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    String ScriptVariable::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\",";
        ret += DeviceConstStrings::value;//StartWithComma;
        ret += value.toString().c_str();
       /* if (value.getType() == HALValue::Type::FLOAT) {
            ret += std::to_string(value.asRawFloat()).c_str();
        } else if (value.getType() == HALValue::Type::UINT) {
            ret += std::to_string(value.asRawUInt()).c_str();
        } else if (value.getType() == HALValue::Type::INT) {
            ret += std::to_string(value.asRawInt()).c_str();
        } else if (value.getType() == HALValue::Type::BOOL) {
            ret += std::to_string(value.asRawBool()).c_str();
        } else if (value.getType() == HALValue::Type::CSTRING) {
            ret += value.toConstChar();
        } else {
            ret += "\"not set\"";
        }*/
        return ret;
    }

}