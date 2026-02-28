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

#pragma once

#include "../Core/Device/DALHAL_Device.h"
#include "../Core/Reactive/DALHAL_Reactive.h"
#include "../Config/DALHAL_ReactiveConfig.h"

namespace DALHAL {

    class Template_Reactive : public Device {
    private:
#if HAS_REACTIVE(TEMPLATE, VALUE_CHANGE)
        REACTIVE_DECLARE_FEATURE_VALUE_CHANGE();
#endif
#if HAS_REACTIVE(TEMPLATE, STATE_CHANGE)
        REACTIVE_DECLARE_FEATURE_STATE_CHANGE();
#endif
#if HAS_REACTIVE(TEMPLATE, READ)
        REACTIVE_DECLARE_FEATURE_READ();
#endif
#if HAS_REACTIVE(TEMPLATE, WRITE)
        REACTIVE_DECLARE_FEATURE_WRITE();
#endif
#if HAS_REACTIVE(TEMPLATE, EXEC)
        REACTIVE_DECLARE_FEATURE_EXEC();
#endif
#if HAS_REACTIVE(TEMPLATE, BRACKET_READ)
        REACTIVE_DECLARE_FEATURE_BRACKET_READ();
#endif
#if HAS_REACTIVE(TEMPLATE, BRACKET_WRITE)
        REACTIVE_DECLARE_FEATURE_BRACKET_WRITE();
#endif
#if HAS_REACTIVE(TEMPLATE, TIMEOUT)
        REACTIVE_DECLARE_FEATURE_TIMEOUT();
#endif
#if HAS_REACTIVE(TEMPLATE, WRITE_ERROR)
        REACTIVE_DECLARE_FEATURE_WRITE_ERROR();
#endif
#if HAS_REACTIVE(TEMPLATE, READ_ERROR)
        REACTIVE_DECLARE_FEATURE_READ_ERROR();
#endif
#if HAS_REACTIVE(TEMPLATE, EXEC_ERROR)
        REACTIVE_DECLARE_FEATURE_EXEC_ERROR();
#endif
    public:
        static const EventDescriptorT<Template_Reactive> eventTable[];

        Template_Reactive(const char* type);
        // If you need helper getters
        HALOperationResult Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) override;

    };
    
}