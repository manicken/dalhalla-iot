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

#include "DALHAL__Template__Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    _Template__Reactive::_Template__Reactive(const char* type) : Device(type) {}

    HALOperationResult _Template__Reactive::Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) {
        return GetSimpleReactiveEventImpl(this, zcFuncName, reactiveEventOut, eventTable);
    }

    const EventDescriptorT<_Template__Reactive> _Template__Reactive::eventTable[] = {
#if HAS_REACTIVE_CUSTOM(_TEMPLATE_)
        DALHAL_REACTIVE_ENTRY(_Template__Reactive, Custom1),
        DALHAL_REACTIVE_ENTRY(_Template__Reactive, Custom2),
        DALHAL_REACTIVE_ENTRY(_Template__Reactive, Custom3),
#endif
#if HAS_REACTIVE_BEGIN(_TEMPLATE_)
        REACTIVE_ENTRY_BEGIN(_Template__Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(_TEMPLATE_)
        REACTIVE_ENTRY_CYCLE_COMPLETE(_Template__Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(_TEMPLATE_)
        REACTIVE_ENTRY_VALUE_CHANGE(_Template__Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(_TEMPLATE_)
        REACTIVE_ENTRY_STATE_CHANGE(_Template__Reactive),
#endif
#if HAS_REACTIVE_READ(_TEMPLATE_)
        REACTIVE_ENTRY_READ(_Template__Reactive),
#endif
#if HAS_REACTIVE_WRITE(_TEMPLATE_)
        REACTIVE_ENTRY_WRITE(_Template__Reactive),
#endif
#if HAS_REACTIVE_EXEC(_TEMPLATE_)
        REACTIVE_ENTRY_EXEC(_Template__Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(_TEMPLATE_)
        REACTIVE_ENTRY_BRACKET_READ(_Template__Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(_TEMPLATE_)
        REACTIVE_ENTRY_BRACKET_WRITE(_Template__Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(_TEMPLATE_)
        REACTIVE_ENTRY_TIMEOUT(_Template__Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(_TEMPLATE_)
        REACTIVE_ENTRY_WRITE_ERROR(_Template__Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(_TEMPLATE_)
        REACTIVE_ENTRY_READ_ERROR(_Template__Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(_TEMPLATE_)
        REACTIVE_ENTRY_EXEC_ERROR(_Template__Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}