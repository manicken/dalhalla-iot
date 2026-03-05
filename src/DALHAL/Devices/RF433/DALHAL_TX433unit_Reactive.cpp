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

#include "DALHAL_TX433unit_Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    TX433unit_Reactive::TX433unit_Reactive(const char* type) : Device(type) {}

    HALOperationResult TX433unit_Reactive::Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) {
        return Reactive::GetSimpleReactiveEventImpl(this, zcFuncName, reactiveEventOut, eventTable);
    }
    DALHAL_DEFINE_REACTIVE_TABLE(TX433unit_Reactive, eventTable) = {

#if HAS_REACTIVE_CUSTOM(TX433_UNIT)
        DALHAL_REACTIVE_ENTRY(TX433unit_Reactive, Custom1),
        DALHAL_REACTIVE_ENTRY(TX433unit_Reactive, Custom2),
        DALHAL_REACTIVE_ENTRY(TX433unit_Reactive, Custom3),
#endif
#if HAS_REACTIVE_BEGIN(TX433_UNIT)
        REACTIVE_ENTRY_BEGIN(TX433unit_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(TX433_UNIT)
        REACTIVE_ENTRY_CYCLE_COMPLETE(TX433unit_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(TX433_UNIT)
        REACTIVE_ENTRY_VALUE_CHANGE(TX433unit_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(TX433_UNIT)
        REACTIVE_ENTRY_STATE_CHANGE(TX433unit_Reactive),
#endif
#if HAS_REACTIVE_READ(TX433_UNIT)
        REACTIVE_ENTRY_READ(TX433unit_Reactive),
#endif
#if HAS_REACTIVE_WRITE(TX433_UNIT)
        REACTIVE_ENTRY_WRITE(TX433unit_Reactive),
#endif
#if HAS_REACTIVE_EXEC(TX433_UNIT)
        REACTIVE_ENTRY_EXEC(TX433unit_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(TX433_UNIT)
        REACTIVE_ENTRY_BRACKET_READ(TX433unit_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(TX433_UNIT)
        REACTIVE_ENTRY_BRACKET_WRITE(TX433unit_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(TX433_UNIT)
        REACTIVE_ENTRY_TIMEOUT(TX433unit_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(TX433_UNIT)
        REACTIVE_ENTRY_WRITE_ERROR(TX433unit_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(TX433_UNIT)
        REACTIVE_ENTRY_READ_ERROR(TX433unit_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(TX433_UNIT)
        REACTIVE_ENTRY_EXEC_ERROR(TX433unit_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}