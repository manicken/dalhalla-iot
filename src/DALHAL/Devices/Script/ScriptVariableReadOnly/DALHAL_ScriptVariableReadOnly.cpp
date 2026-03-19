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

#include "DALHAL_ScriptVariableReadOnly.h"

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

namespace DALHAL {
    constexpr Registry::DefineBase ScriptVariableReadOnly::RegistryDefine = {
        
        Create,
        VerifyJSON,
        nullptr /* no events available */
    };
    
    ScriptVariableReadOnly::ScriptVariableReadOnly(DeviceCreateContext& context) : Device(context.deviceType) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
        uid = encodeUID(GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID));
        value = GetAsUINT32(jsonObj, "val",0);
    }

    bool ScriptVariableReadOnly::VerifyJSON(const JsonVariant &jsonObj) {
        // could add type def later if wanted
        // also nonvolatile storage could be a mode
        return true;
    }

    Device* ScriptVariableReadOnly::Create(DeviceCreateContext& context) {
        return new ScriptVariableReadOnly(context);
    }

    HALOperationResult ScriptVariableReadOnly::read(HALValue& val) {
        val = value;
        return HALOperationResult::Success;
    }

    String ScriptVariableReadOnly::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\",";
        ret += DeviceConstStrings::value;//StartWithComma;
        if (value.getType() == HALValue::Type::FLOAT)
            ret += std::to_string(value.asFloat()).c_str();
        else if (value.getType() == HALValue::Type::UINT)
            ret += std::to_string(value.asUInt()).c_str();
        else if (value.getType() == HALValue::Type::INT)
            ret += std::to_string(value.asInt()).c_str();
        else
            ret += "\"not set\"";
        return ret;
    }

}