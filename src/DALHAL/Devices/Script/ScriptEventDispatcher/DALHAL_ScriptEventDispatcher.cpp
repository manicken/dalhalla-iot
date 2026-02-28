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

#include "DALHAL_ScriptEventDispatcher.h"

#include "../../../Support/DALHAL_Logger.h"
#include "../../../Core/Device/DALHAL_JSON_Config_Defines.h"
#include "../../../Support/DALHAL_ArduinoJSON_ext.h"

namespace DALHAL {
    
    ScriptEventDispatcher::ScriptEventDispatcher(const JsonVariant &jsonObj, const char* type) : Device(type) {
        uid = encodeUID(GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID));
        eventCounter = 0;
    }

    bool ScriptEventDispatcher::VerifyJSON(const JsonVariant &jsonObj) {
        return true;
    }

    Device* ScriptEventDispatcher::Create(const JsonVariant &jsonObj, const char* type) {
        return new ScriptEventDispatcher(jsonObj, type);
    }

    HALOperationResult ScriptEventDispatcher::exec() {
        eventCounter++;
        return HALOperationResult::Success;
    }
    
    /*static*/HALOperationResult ScriptEventDispatcher::exec(Device* device) {
        static_cast<ScriptEventDispatcher*>(device)->eventCounter++;
        return HALOperationResult::Success;
    }

    Device::Exec_FuncType ScriptEventDispatcher::GetExec_Function(ZeroCopyString& zcFuncName) {
        return ScriptEventDispatcher::exec;
    }


    String ScriptEventDispatcher::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += '"';
        ret += ',';
        ret += DeviceConstStrings::type;
        ret += type;
        ret += '"';
        return ret;
    }

}