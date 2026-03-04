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

#include "DALHAL_DigitalOutput_Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    DigitalOutput_Reactive::DigitalOutput_Reactive(const char* type) : Device(type) {}

    HALOperationResult DigitalOutput_Reactive::Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) {
        return Reactive::GetSimpleReactiveEventImpl(this, zcFuncName, reactiveEventOut, eventTable);
    }
    DALHAL_DEFINE_REACTIVE_TABLE(DigitalOutput_Reactive, eventTable) = {

#if HAS_REACTIVE_BEGIN(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_BEGIN(DigitalOutput_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_CYCLE_COMPLETE(DigitalOutput_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_VALUE_CHANGE(DigitalOutput_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_STATE_CHANGE(DigitalOutput_Reactive),
#endif
#if HAS_REACTIVE_READ(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_READ(DigitalOutput_Reactive),
#endif
#if HAS_REACTIVE_WRITE(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_WRITE(DigitalOutput_Reactive),
#endif
#if HAS_REACTIVE_EXEC(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_EXEC(DigitalOutput_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_BRACKET_READ(DigitalOutput_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_BRACKET_WRITE(DigitalOutput_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_TIMEOUT(DigitalOutput_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_WRITE_ERROR(DigitalOutput_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_READ_ERROR(DigitalOutput_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(DIGITAL_OUTPUT)
        REACTIVE_ENTRY_EXEC_ERROR(DigitalOutput_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}