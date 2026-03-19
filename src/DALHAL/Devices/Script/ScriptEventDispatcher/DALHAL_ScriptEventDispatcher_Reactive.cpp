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

#include "DALHAL_ScriptEventDispatcher_Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    ScriptEventDispatcher_Reactive::ScriptEventDispatcher_Reactive(const char* type) : Device(type) {}


    DALHAL_DEFINE_GET_REACTIVE_EVENT_FUNC(ScriptEventDispatcher_Reactive);
    
    DALHAL_DEFINE_REACTIVE_TABLE(ScriptEventDispatcher_Reactive) = {

#if HAS_REACTIVE_CUSTOM(SCRIPT_EVENT_DISPATCHER)
        DALHAL_REACTIVE_ENTRY(ScriptEventDispatcher_Reactive, Custom1),
        DALHAL_REACTIVE_ENTRY(ScriptEventDispatcher_Reactive, Custom2),
        DALHAL_REACTIVE_ENTRY(ScriptEventDispatcher_Reactive, Custom3),
#endif
#if HAS_REACTIVE_BEGIN(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_BEGIN(ScriptEventDispatcher_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_CYCLE_COMPLETE(ScriptEventDispatcher_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_VALUE_CHANGE(ScriptEventDispatcher_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_STATE_CHANGE(ScriptEventDispatcher_Reactive),
#endif
#if HAS_REACTIVE_READ(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_READ(ScriptEventDispatcher_Reactive),
#endif
#if HAS_REACTIVE_WRITE(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_WRITE(ScriptEventDispatcher_Reactive),
#endif
#if HAS_REACTIVE_EXEC(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_EXEC(ScriptEventDispatcher_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_BRACKET_READ(ScriptEventDispatcher_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_BRACKET_WRITE(ScriptEventDispatcher_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_TIMEOUT(ScriptEventDispatcher_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_WRITE_ERROR(ScriptEventDispatcher_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_READ_ERROR(ScriptEventDispatcher_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(SCRIPT_EVENT_DISPATCHER)
        REACTIVE_ENTRY_EXEC_ERROR(ScriptEventDispatcher_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}