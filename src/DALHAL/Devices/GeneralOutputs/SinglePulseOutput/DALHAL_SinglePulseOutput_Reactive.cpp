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

#include "DALHAL_SinglePulseOutput_Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    SinglePulseOutput_Reactive::SinglePulseOutput_Reactive(const char* type) : Device(type) {}

    DALHAL_DEFINE_GET_REACTIVE_EVENT_FUNC(SinglePulseOutput_Reactive);

    DALHAL_DEFINE_REACTIVE_TABLE(SinglePulseOutput_Reactive) = {

#if HAS_REACTIVE_CUSTOM(SINGLE_PULSE_OUTPUT)
        DALHAL_REACTIVE_ENTRY(SinglePulseOutput_Reactive, Custom1),
        DALHAL_REACTIVE_ENTRY(SinglePulseOutput_Reactive, Custom2),
        DALHAL_REACTIVE_ENTRY(SinglePulseOutput_Reactive, Custom3),
#endif
#if HAS_REACTIVE_BEGIN(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_BEGIN(SinglePulseOutput_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_CYCLE_COMPLETE(SinglePulseOutput_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_VALUE_CHANGE(SinglePulseOutput_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_STATE_CHANGE(SinglePulseOutput_Reactive),
#endif
#if HAS_REACTIVE_READ(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_READ(SinglePulseOutput_Reactive),
#endif
#if HAS_REACTIVE_WRITE(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_WRITE(SinglePulseOutput_Reactive),
#endif
#if HAS_REACTIVE_EXEC(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_EXEC(SinglePulseOutput_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_BRACKET_READ(SinglePulseOutput_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_BRACKET_WRITE(SinglePulseOutput_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_TIMEOUT(SinglePulseOutput_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_WRITE_ERROR(SinglePulseOutput_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_READ_ERROR(SinglePulseOutput_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(SINGLE_PULSE_OUTPUT)
        REACTIVE_ENTRY_EXEC_ERROR(SinglePulseOutput_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}