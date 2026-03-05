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

#include "DALHAL_ScriptArray_Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    ScriptArray_Reactive::ScriptArray_Reactive(const char* type) : Device(type) {}

    HALOperationResult ScriptArray_Reactive::Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) {
        return Reactive::GetSimpleReactiveEventImpl(this, zcFuncName, reactiveEventOut, eventTable);
    }
    DALHAL_DEFINE_REACTIVE_TABLE(ScriptArray_Reactive, eventTable) = {

#if HAS_REACTIVE_CUSTOM(SCRIPT_ARRAY)
        DALHAL_REACTIVE_ENTRY(ScriptArray_Reactive, Custom1),
        DALHAL_REACTIVE_ENTRY(ScriptArray_Reactive, Custom2),
        DALHAL_REACTIVE_ENTRY(ScriptArray_Reactive, Custom3),
#endif
#if HAS_REACTIVE_BEGIN(SCRIPT_ARRAY)
        REACTIVE_ENTRY_BEGIN(ScriptArray_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(SCRIPT_ARRAY)
        REACTIVE_ENTRY_CYCLE_COMPLETE(ScriptArray_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(SCRIPT_ARRAY)
        REACTIVE_ENTRY_VALUE_CHANGE(ScriptArray_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(SCRIPT_ARRAY)
        REACTIVE_ENTRY_STATE_CHANGE(ScriptArray_Reactive),
#endif
#if HAS_REACTIVE_READ(SCRIPT_ARRAY)
        REACTIVE_ENTRY_READ(ScriptArray_Reactive),
#endif
#if HAS_REACTIVE_WRITE(SCRIPT_ARRAY)
        REACTIVE_ENTRY_WRITE(ScriptArray_Reactive),
#endif
#if HAS_REACTIVE_EXEC(SCRIPT_ARRAY)
        REACTIVE_ENTRY_EXEC(ScriptArray_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(SCRIPT_ARRAY)
        REACTIVE_ENTRY_BRACKET_READ(ScriptArray_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(SCRIPT_ARRAY)
        REACTIVE_ENTRY_BRACKET_WRITE(ScriptArray_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(SCRIPT_ARRAY)
        REACTIVE_ENTRY_TIMEOUT(ScriptArray_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(SCRIPT_ARRAY)
        REACTIVE_ENTRY_WRITE_ERROR(ScriptArray_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(SCRIPT_ARRAY)
        REACTIVE_ENTRY_READ_ERROR(ScriptArray_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(SCRIPT_ARRAY)
        REACTIVE_ENTRY_EXEC_ERROR(ScriptArray_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}