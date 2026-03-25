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

#include "DALHAL_REGO600_Register_Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    REGO600_Register_Reactive::REGO600_Register_Reactive(const char* type) : Device(type) {}

    DALHAL_DEFINE_GET_REACTIVE_EVENT_FUNC(REGO600_Register_Reactive);

    DALHAL_DEFINE_REACTIVE_TABLE(REGO600_Register_Reactive) = {

#if HAS_REACTIVE_CUSTOM(REGO600_REGISTRY_ITEM)
        DALHAL_REACTIVE_ENTRY(REGO600_Register_Reactive, Custom1),
        DALHAL_REACTIVE_ENTRY(REGO600_Register_Reactive, Custom2),
        DALHAL_REACTIVE_ENTRY(REGO600_Register_Reactive, Custom3),
#endif
#if HAS_REACTIVE_BEGIN(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_BEGIN(REGO600_Register_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_CYCLE_COMPLETE(REGO600_Register_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_VALUE_CHANGE(REGO600_Register_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_STATE_CHANGE(REGO600_Register_Reactive),
#endif
#if HAS_REACTIVE_READ(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_READ(REGO600_Register_Reactive),
#endif
#if HAS_REACTIVE_WRITE(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_WRITE(REGO600_Register_Reactive),
#endif
#if HAS_REACTIVE_EXEC(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_EXEC(REGO600_Register_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_BRACKET_READ(REGO600_Register_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_BRACKET_WRITE(REGO600_Register_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_TIMEOUT(REGO600_Register_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_WRITE_ERROR(REGO600_Register_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_READ_ERROR(REGO600_Register_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(REGO600_REGISTRY_ITEM)
        REACTIVE_ENTRY_EXEC_ERROR(REGO600_Register_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}