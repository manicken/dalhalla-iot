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

#include "DALHAL_Actuator_Reactive.h"
#include <DALHAL/Core/Reactive/DALHAL_ReactiveEvent.h>
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    Actuator_Reactive::Actuator_Reactive(const char* type) : Device(type) {}

    DALHAL_DEFINE_GET_REACTIVE_EVENT_FUNC(Actuator_Reactive);

    DALHAL_DEFINE_REACTIVE_TABLE(Actuator_Reactive) = {
#if HAS_REACTIVE_CUSTOM(ACTUATOR)
        DALHAL_REACTIVE_ENTRY(Actuator_Reactive, ReachedMin),
        DALHAL_REACTIVE_ENTRY(Actuator_Reactive, ReachedMax),
#endif
#if HAS_REACTIVE_BEGIN(ACTUATOR)
        REACTIVE_ENTRY_BEGIN(Actuator_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(ACTUATOR)
        REACTIVE_ENTRY_CYCLE_COMPLETE(Actuator_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(ACTUATOR)
        REACTIVE_ENTRY_VALUE_CHANGE(Actuator_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(ACTUATOR)
        REACTIVE_ENTRY_STATE_CHANGE(Actuator_Reactive),
#endif
#if HAS_REACTIVE_READ(ACTUATOR)
        REACTIVE_ENTRY_READ(Actuator_Reactive),
#endif
#if HAS_REACTIVE_WRITE(ACTUATOR)
        REACTIVE_ENTRY_WRITE(Actuator_Reactive),
#endif
#if HAS_REACTIVE_EXEC(ACTUATOR)
        REACTIVE_ENTRY_EXEC(Actuator_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(ACTUATOR)
        REACTIVE_ENTRY_BRACKET_READ(Actuator_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(ACTUATOR)
        REACTIVE_ENTRY_BRACKET_WRITE(Actuator_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(ACTUATOR)
        REACTIVE_ENTRY_TIMEOUT(Actuator_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(ACTUATOR)
        REACTIVE_ENTRY_WRITE_ERROR(Actuator_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(ACTUATOR)
        REACTIVE_ENTRY_READ_ERROR(Actuator_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(ACTUATOR)
        REACTIVE_ENTRY_EXEC_ERROR(Actuator_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}