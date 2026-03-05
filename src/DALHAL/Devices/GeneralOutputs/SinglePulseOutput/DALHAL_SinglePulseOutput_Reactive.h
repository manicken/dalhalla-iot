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

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Reactive/DALHAL_Reactive.h>
#include <DALHAL/Core/Reactive/DALHAL_ReactiveTypes.h>
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    class SinglePulseOutput_Reactive : public Device {
    protected:
#if HAS_REACTIVE_CUSTOM(SINGLE_PULSE_OUTPUT)
        DALHAL_DECLARE_REACTIVE_FEATURE(Custom1);
        DALHAL_DECLARE_REACTIVE_FEATURE(Custom2);
        DALHAL_DECLARE_REACTIVE_FEATURE(Custom3);
#endif
#if HAS_REACTIVE_BEGIN(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_FEATURE_BEGIN();
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_CYCLE_COMPLETE();
#endif
#if HAS_REACTIVE_VALUE_CHANGE(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_FEATURE_VALUE_CHANGE();
#endif
#if HAS_REACTIVE_STATE_CHANGE(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_FEATURE_STATE_CHANGE();
#endif
#if HAS_REACTIVE_READ(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_FEATURE_READ();
#endif
#if HAS_REACTIVE_WRITE(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_FEATURE_WRITE();
#endif
#if HAS_REACTIVE_EXEC(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_FEATURE_EXEC();
#endif
#if HAS_REACTIVE_BRACKET_READ(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_FEATURE_BRACKET_READ();
#endif
#if HAS_REACTIVE_BRACKET_WRITE(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_FEATURE_BRACKET_WRITE();
#endif
#if HAS_REACTIVE_TIMEOUT(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_FEATURE_TIMEOUT();
#endif
#if HAS_REACTIVE_WRITE_ERROR(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_FEATURE_WRITE_ERROR();
#endif
#if HAS_REACTIVE_READ_ERROR(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_FEATURE_READ_ERROR();
#endif
#if HAS_REACTIVE_EXEC_ERROR(SINGLE_PULSE_OUTPUT)
        REACTIVE_DECLARE_FEATURE_EXEC_ERROR();
#endif
    public:
        DALHAL_DECLARE_REACTIVE_TABLE(SinglePulseOutput_Reactive, eventTable);

        SinglePulseOutput_Reactive(const char* type);

        HALOperationResult Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) override;

    };
    
}