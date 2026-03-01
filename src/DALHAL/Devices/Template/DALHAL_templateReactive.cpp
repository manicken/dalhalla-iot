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

#include "DALHAL_templateReactive.h"
#include "../../Config/DALHAL_ReactiveConfig.h"

namespace DALHAL {

    Template_Reactive::Template_Reactive(const char* type) : Device(type) {}

    HALOperationResult Template_Reactive::Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) {
        return GetSimpleReactiveEventImpl(this, zcFuncName, reactiveEventOut, eventTable);
    }

    const EventDescriptorT<Template_Reactive> Template_Reactive::eventTable[] = {
#if HAS_REACTIVE(TEMPLATE, BEGIN)
        REACTIVE_ENTRY_BEGIN(Template_Reactive),
#endif        
#if HAS_REACTIVE(TEMPLATE, VALUE_CHANGE)
        REACTIVE_ENTRY_VALUE_CHANGE(Template_Reactive),
#endif
#if HAS_REACTIVE(TEMPLATE, STATE_CHANGE)
        REACTIVE_ENTRY_STATE_CHANGE(Template_Reactive),
#endif
#if HAS_REACTIVE(TEMPLATE, READ)
        REACTIVE_ENTRY_READ(Template_Reactive),
#endif
#if HAS_REACTIVE(TEMPLATE, WRITE)
        REACTIVE_ENTRY_WRITE(Template_Reactive),
#endif
#if HAS_REACTIVE(TEMPLATE, EXEC)
        REACTIVE_ENTRY_EXEC(Template_Reactive),
#endif
#if HAS_REACTIVE(TEMPLATE, BRACKET_READ)
        REACTIVE_ENTRY_BRACKET_READ(Template_Reactive),
#endif
#if HAS_REACTIVE(TEMPLATE, BRACKET_WRITE)
        REACTIVE_ENTRY_BRACKET_WRITE(Template_Reactive),
#endif
#if HAS_REACTIVE(TEMPLATE, TIMEOUT)
        REACTIVE_ENTRY_TIMEOUT(Template_Reactive),
#endif
#if HAS_REACTIVE(TEMPLATE, WRITE_ERROR)
        REACTIVE_ENTRY_WRITE_ERROR(Template_Reactive),
#endif
#if HAS_REACTIVE(TEMPLATE, READ_ERROR)
        REACTIVE_ENTRY_READ_ERROR(Template_Reactive),
#endif
#if HAS_REACTIVE(TEMPLATE, EXEC_ERROR)
        REACTIVE_ENTRY_EXEC_ERROR(Template_Reactive),
#endif
        REACTIVE_TABLE_END
    };
}