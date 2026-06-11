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

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase ScriptEventDispatcher::RegistryDefine = {
        Create,
        &JsonSchema::ScriptEventDispatcher::Root,
        DALHAL_REACTIVE_EVENT_TABLE(SCRIPT_EVENT_DISPATCHER)
    };
    
    /* override */
    const Registry::DefineBase* ScriptEventDispatcher::GetRegistryDefine() {
        return &RegistryDefine;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::Exec> ScriptEventDispatcher::execFunctions[] = {
        {"", &ScriptEventDispatcher::static_exec, "execute event"}
    };

    constexpr DeviceFunctionTable ScriptEventDispatcher::FunctionTable = {
        {execFunctions, sizeof(execFunctions) / sizeof(execFunctions[0])},

        EmptyFunctionTable<FunctionTypes::ReadToHALValue>,
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,

        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,

        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };
    
    ScriptEventDispatcher::ScriptEventDispatcher(DeviceCreateContext& context) : ScriptEventDispatcher_DeviceBase(context.deviceType) {
        JsonSchema::ScriptEventDispatcher::Extractors::Apply(context, this);
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
    
    /*static*/HALOperationResult ScriptEventDispatcher::static_exec(Device* device) {
#if HAS_REACTIVE_EXEC(SCRIPT_EVENT_DISPATCHER)
        static_cast<ScriptEventDispatcher*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    void ScriptEventDispatcher::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
    }

}