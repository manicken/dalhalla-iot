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

#include "DALHAL_DHT_Reactive.h"

#include <DALHAL/Core/Reactive/DALHAL_ReactiveTypes.h>
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    DHT_Reactive::DHT_Reactive(const char* type) : Device(type) {}

    HALOperationResult DHT_Reactive::Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) {
        return Reactive::GetSimpleReactiveEventImpl(this, zcFuncName, reactiveEventOut, eventTable);
    }

    DALHAL_DEFINE_REACTIVE_TABLE(DHT_Reactive, eventTable) = {
#if HAS_REACTIVE_BEGIN(DHT)
        REACTIVE_ENTRY_BEGIN(DHT_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(DHT)
        REACTIVE_ENTRY_CYCLE_COMPLETE(DHT_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(DHT)
        REACTIVE_ENTRY_VALUE_CHANGE(DHT_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(DHT)
        REACTIVE_ENTRY_STATE_CHANGE(DHT_Reactive),
#endif
#if HAS_REACTIVE_READ(DHT)
        REACTIVE_ENTRY_READ(DHT_Reactive),
#endif
#if HAS_REACTIVE_WRITE(DHT)
        REACTIVE_ENTRY_WRITE(DHT_Reactive),
#endif
#if HAS_REACTIVE_EXEC(DHT)
        REACTIVE_ENTRY_EXEC(DHT_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(DHT)
        REACTIVE_ENTRY_BRACKET_READ(DHT_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(DHT)
        REACTIVE_ENTRY_BRACKET_WRITE(DHT_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(DHT)
        REACTIVE_ENTRY_TIMEOUT(DHT_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(DHT)
        REACTIVE_ENTRY_WRITE_ERROR(DHT_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(DHT)
        REACTIVE_ENTRY_READ_ERROR(DHT_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(DHT)
        REACTIVE_ENTRY_EXEC_ERROR(DHT_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}