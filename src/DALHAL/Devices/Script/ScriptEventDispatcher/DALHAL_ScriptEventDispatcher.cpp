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

#include "DALHAL_ScriptEventDispatcher.h"

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include "DALHAL_ScriptEventDispatcher_JSON_Schema.h"

namespace DALHAL {
    constexpr Registry::DefineBase ScriptEventDispatcher::RegistryDefine = {
        Create,
        &JsonSchema::ScriptEventDispatcher,
        DALHAL_REACTIVE_EVENT_TABLE(SCRIPT_EVENT_DISPATCHER)
    };
    
    ScriptEventDispatcher::ScriptEventDispatcher(DeviceCreateContext& context) : ScriptEventDispatcher_DeviceBase(context.deviceType) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
        uid = encodeUID(GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID));
    }

    Device* ScriptEventDispatcher::Create(DeviceCreateContext& context) {
        return new ScriptEventDispatcher(context);
    }

    HALOperationResult ScriptEventDispatcher::exec() {
#if HAS_REACTIVE_EXEC(SCRIPT_EVENT_DISPATCHER)
        triggerExec();
#endif
        return HALOperationResult::Success;
    }
    
    /*static*/HALOperationResult ScriptEventDispatcher::exec(Device* device) {
#if HAS_REACTIVE_EXEC(SCRIPT_EVENT_DISPATCHER)
        static_cast<ScriptEventDispatcher*>(device)->triggerExec();
#endif
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
        ret += this->Type;
        ret += '"';
        return ret;
    }

}