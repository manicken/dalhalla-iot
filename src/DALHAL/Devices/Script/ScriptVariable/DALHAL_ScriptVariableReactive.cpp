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

#include "DALHAL_ScriptVariableReactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    ScriptVariable_Reactive::ScriptVariable_Reactive(const char* type) : Device(type) {}

    HALOperationResult ScriptVariable_Reactive::Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) {
        return GetSimpleReactiveEventImpl(this, zcFuncName, reactiveEventOut, eventTable);
    }

    const EventDescriptorT<ScriptVariable_Reactive> ScriptVariable_Reactive::eventTable[] = {
#if HAS_REACTIVE_BEGIN(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_BEGIN(ScriptVariable_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_CYCLE_COMPLETE(ScriptVariable_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_VALUE_CHANGE(ScriptVariable_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_STATE_CHANGE(ScriptVariable_Reactive),
#endif
#if HAS_REACTIVE_READ(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_READ(ScriptVariable_Reactive),
#endif
#if HAS_REACTIVE_WRITE(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_WRITE(ScriptVariable_Reactive),
#endif
#if HAS_REACTIVE_EXEC(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_EXEC(ScriptVariable_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_BRACKET_READ(ScriptVariable_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_BRACKET_WRITE(ScriptVariable_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_TIMEOUT(ScriptVariable_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_WRITE_ERROR(ScriptVariable_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_READ_ERROR(ScriptVariable_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(SCRIPT_VARIABLE)
        REACTIVE_ENTRY_EXEC_ERROR(ScriptVariable_Reactive),
#endif
        REACTIVE_TABLE_END
    };
}