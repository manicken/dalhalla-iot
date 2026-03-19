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

#include "DALHAL_ThingSpeak_Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#include <DALHAL/Core/Reactive/DALHAL_ReactiveEvent.h>

namespace DALHAL {

    ThingSpeak_Reactive::ThingSpeak_Reactive(const char* type) : Device(type) {}

    DALHAL_DEFINE_GET_REACTIVE_EVENT_FUNC(ThingSpeak_Reactive);

    DALHAL_DEFINE_REACTIVE_TABLE(ThingSpeak_Reactive) = {

#if HAS_REACTIVE_BEGIN(THINGSPEAK)
        REACTIVE_ENTRY_BEGIN(ThingSpeak_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(THINGSPEAK)
        REACTIVE_ENTRY_CYCLE_COMPLETE(ThingSpeak_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(THINGSPEAK)
        REACTIVE_ENTRY_VALUE_CHANGE(ThingSpeak_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(THINGSPEAK)
        REACTIVE_ENTRY_STATE_CHANGE(ThingSpeak_Reactive),
#endif
#if HAS_REACTIVE_READ(THINGSPEAK)
        REACTIVE_ENTRY_READ(ThingSpeak_Reactive),
#endif
#if HAS_REACTIVE_WRITE(THINGSPEAK)
        REACTIVE_ENTRY_WRITE(ThingSpeak_Reactive),
#endif
#if HAS_REACTIVE_EXEC(THINGSPEAK)
        REACTIVE_ENTRY_EXEC(ThingSpeak_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(THINGSPEAK)
        REACTIVE_ENTRY_BRACKET_READ(ThingSpeak_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(THINGSPEAK)
        REACTIVE_ENTRY_BRACKET_WRITE(ThingSpeak_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(THINGSPEAK)
        REACTIVE_ENTRY_TIMEOUT(ThingSpeak_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(THINGSPEAK)
        REACTIVE_ENTRY_WRITE_ERROR(ThingSpeak_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(THINGSPEAK)
        REACTIVE_ENTRY_READ_ERROR(ThingSpeak_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(THINGSPEAK)
        REACTIVE_ENTRY_EXEC_ERROR(ThingSpeak_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}