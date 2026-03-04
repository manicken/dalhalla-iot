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

#include "DALHAL_ButtonInput_Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    ButtonInput_Reactive::ButtonInput_Reactive(const char* type) : Device(type) {}

    HALOperationResult ButtonInput_Reactive::Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) {
        return Reactive::GetSimpleReactiveEventImpl(this, zcFuncName, reactiveEventOut, eventTable);
    }
    DALHAL_DEFINE_REACTIVE_TABLE(ButtonInput_Reactive, eventTable) = {

#if HAS_REACTIVE_CUSTOM(BUTTON_INPUT)
        DALHAL_REACTIVE_ENTRY(ButtonInput_Reactive, Press),
        DALHAL_REACTIVE_ENTRY(ButtonInput_Reactive, Release),
#endif
#if HAS_REACTIVE_BEGIN(BUTTON_INPUT)
        REACTIVE_ENTRY_BEGIN(ButtonInput_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(BUTTON_INPUT)
        REACTIVE_ENTRY_CYCLE_COMPLETE(ButtonInput_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(BUTTON_INPUT)
        REACTIVE_ENTRY_VALUE_CHANGE(ButtonInput_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(BUTTON_INPUT)
        REACTIVE_ENTRY_STATE_CHANGE(ButtonInput_Reactive),
#endif
#if HAS_REACTIVE_READ(BUTTON_INPUT)
        REACTIVE_ENTRY_READ(ButtonInput_Reactive),
#endif
#if HAS_REACTIVE_WRITE(BUTTON_INPUT)
        REACTIVE_ENTRY_WRITE(ButtonInput_Reactive),
#endif
#if HAS_REACTIVE_EXEC(BUTTON_INPUT)
        REACTIVE_ENTRY_EXEC(ButtonInput_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(BUTTON_INPUT)
        REACTIVE_ENTRY_BRACKET_READ(ButtonInput_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(BUTTON_INPUT)
        REACTIVE_ENTRY_BRACKET_WRITE(ButtonInput_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(BUTTON_INPUT)
        REACTIVE_ENTRY_TIMEOUT(ButtonInput_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(BUTTON_INPUT)
        REACTIVE_ENTRY_WRITE_ERROR(ButtonInput_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(BUTTON_INPUT)
        REACTIVE_ENTRY_READ_ERROR(ButtonInput_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(BUTTON_INPUT)
        REACTIVE_ENTRY_EXEC_ERROR(ButtonInput_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}