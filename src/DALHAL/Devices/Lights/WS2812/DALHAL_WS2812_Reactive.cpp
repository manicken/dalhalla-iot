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

#include "DALHAL_WS2812_Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    WS2812_Reactive::WS2812_Reactive(const char* type) : Device(type) {}

    HALOperationResult WS2812_Reactive::Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) {
        return Reactive::GetSimpleReactiveEventImpl(this, zcFuncName, reactiveEventOut, eventTable);
    }
    DALHAL_DEFINE_REACTIVE_TABLE(WS2812_Reactive, eventTable) = {

#if HAS_REACTIVE_CUSTOM(WS2812)
        DALHAL_REACTIVE_ENTRY(WS2812_Reactive, Custom1), // keep for future custom features
#endif
#if HAS_REACTIVE_BEGIN(WS2812)
        REACTIVE_ENTRY_BEGIN(WS2812_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(WS2812)
        REACTIVE_ENTRY_CYCLE_COMPLETE(WS2812_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(WS2812)
        REACTIVE_ENTRY_VALUE_CHANGE(WS2812_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(WS2812)
        REACTIVE_ENTRY_STATE_CHANGE(WS2812_Reactive),
#endif
#if HAS_REACTIVE_READ(WS2812)
        REACTIVE_ENTRY_READ(WS2812_Reactive),
#endif
#if HAS_REACTIVE_WRITE(WS2812)
        REACTIVE_ENTRY_WRITE(WS2812_Reactive),
#endif
#if HAS_REACTIVE_EXEC(WS2812)
        REACTIVE_ENTRY_EXEC(WS2812_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(WS2812)
        REACTIVE_ENTRY_BRACKET_READ(WS2812_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(WS2812)
        REACTIVE_ENTRY_BRACKET_WRITE(WS2812_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(WS2812)
        REACTIVE_ENTRY_TIMEOUT(WS2812_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(WS2812)
        REACTIVE_ENTRY_WRITE_ERROR(WS2812_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(WS2812)
        REACTIVE_ENTRY_READ_ERROR(WS2812_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(WS2812)
        REACTIVE_ENTRY_EXEC_ERROR(WS2812_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}