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

#include "DALHAL_ScriptVariableWriteOnlyTest_Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    ScriptVariableWriteOnlyTest_Reactive::ScriptVariableWriteOnlyTest_Reactive(const char* type) : Device(type) {}

    DALHAL_DEFINE_GET_REACTIVE_EVENT_FUNC(ScriptVariableWriteOnlyTest_Reactive);

    DALHAL_DEFINE_REACTIVE_TABLE(ScriptVariableWriteOnlyTest_Reactive) = {

#if HAS_REACTIVE_CUSTOM(SCRIPT_WRITEVAR)
        DALHAL_REACTIVE_ENTRY(ScriptVariableWriteOnlyTest_Reactive, Custom1),
        DALHAL_REACTIVE_ENTRY(ScriptVariableWriteOnlyTest_Reactive, Custom2),
        DALHAL_REACTIVE_ENTRY(ScriptVariableWriteOnlyTest_Reactive, Custom3),
#endif
#if HAS_REACTIVE_BEGIN(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_BEGIN(ScriptVariableWriteOnlyTest_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_CYCLE_COMPLETE(ScriptVariableWriteOnlyTest_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_VALUE_CHANGE(ScriptVariableWriteOnlyTest_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_STATE_CHANGE(ScriptVariableWriteOnlyTest_Reactive),
#endif
#if HAS_REACTIVE_READ(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_READ(ScriptVariableWriteOnlyTest_Reactive),
#endif
#if HAS_REACTIVE_WRITE(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_WRITE(ScriptVariableWriteOnlyTest_Reactive),
#endif
#if HAS_REACTIVE_EXEC(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_EXEC(ScriptVariableWriteOnlyTest_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_BRACKET_READ(ScriptVariableWriteOnlyTest_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_BRACKET_WRITE(ScriptVariableWriteOnlyTest_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_TIMEOUT(ScriptVariableWriteOnlyTest_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_WRITE_ERROR(ScriptVariableWriteOnlyTest_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_READ_ERROR(ScriptVariableWriteOnlyTest_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(SCRIPT_WRITEVAR)
        REACTIVE_ENTRY_EXEC_ERROR(ScriptVariableWriteOnlyTest_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}