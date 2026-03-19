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

#pragma once

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Reactive/DALHAL_Reactive.h>
#include <DALHAL/Core/Reactive/DALHAL_ReactiveEvent.h>
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    class AnalogInput_Reactive : public Device {
    protected:
#if HAS_REACTIVE_BEGIN(ANALOG_INPUT)
        REACTIVE_DECLARE_FEATURE_BEGIN(AnalogInput_Reactive);
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(ANALOG_INPUT)
        REACTIVE_DECLARE_CYCLE_COMPLETE(AnalogInput_Reactive);
#endif
#if HAS_REACTIVE_VALUE_CHANGE(ANALOG_INPUT)
        REACTIVE_DECLARE_FEATURE_VALUE_CHANGE(AnalogInput_Reactive);
#endif
#if HAS_REACTIVE_STATE_CHANGE(ANALOG_INPUT)
        REACTIVE_DECLARE_FEATURE_STATE_CHANGE(AnalogInput_Reactive);
#endif
#if HAS_REACTIVE_READ(ANALOG_INPUT)
        REACTIVE_DECLARE_FEATURE_READ(AnalogInput_Reactive);
#endif
#if HAS_REACTIVE_WRITE(ANALOG_INPUT)
        REACTIVE_DECLARE_FEATURE_WRITE(AnalogInput_Reactive);
#endif
#if HAS_REACTIVE_EXEC(ANALOG_INPUT)
        REACTIVE_DECLARE_FEATURE_EXEC(AnalogInput_Reactive);
#endif
#if HAS_REACTIVE_BRACKET_READ(ANALOG_INPUT)
        REACTIVE_DECLARE_FEATURE_BRACKET_READ(AnalogInput_Reactive);
#endif
#if HAS_REACTIVE_BRACKET_WRITE(ANALOG_INPUT)
        REACTIVE_DECLARE_FEATURE_BRACKET_WRITE(AnalogInput_Reactive);
#endif
#if HAS_REACTIVE_TIMEOUT(ANALOG_INPUT)
        REACTIVE_DECLARE_FEATURE_TIMEOUT(AnalogInput_Reactive);
#endif
#if HAS_REACTIVE_WRITE_ERROR(ANALOG_INPUT)
        REACTIVE_DECLARE_FEATURE_WRITE_ERROR(AnalogInput_Reactive);
#endif
#if HAS_REACTIVE_READ_ERROR(ANALOG_INPUT)
        REACTIVE_DECLARE_FEATURE_READ_ERROR(AnalogInput_Reactive);
#endif
#if HAS_REACTIVE_EXEC_ERROR(ANALOG_INPUT)
        REACTIVE_DECLARE_FEATURE_EXEC_ERROR(AnalogInput_Reactive);
#endif
    public:
        DALHAL_DECLARE_REACTIVE_TABLE(AnalogInput_Reactive);

        AnalogInput_Reactive(const char* type);

        HALOperationResult Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) override;

    };
    
}