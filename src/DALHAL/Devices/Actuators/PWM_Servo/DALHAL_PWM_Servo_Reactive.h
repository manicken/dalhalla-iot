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
#include <DALHAL/Core/Reactive/DALHAL_ReactiveEvent.h>
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    class PWM_Servo_Reactive : public Device {
    protected:

#if HAS_REACTIVE_BEGIN(PWM_SERVO)
        REACTIVE_DECLARE_FEATURE_BEGIN(PWM_Servo_Reactive);
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(PWM_SERVO)
        REACTIVE_DECLARE_CYCLE_COMPLETE(PWM_Servo_Reactive);
#endif
#if HAS_REACTIVE_VALUE_CHANGE(PWM_SERVO)
        REACTIVE_DECLARE_FEATURE_VALUE_CHANGE(PWM_Servo_Reactive);
#endif
#if HAS_REACTIVE_STATE_CHANGE(PWM_SERVO)
        REACTIVE_DECLARE_FEATURE_STATE_CHANGE(PWM_Servo_Reactive);
#endif
#if HAS_REACTIVE_READ(PWM_SERVO)
        REACTIVE_DECLARE_FEATURE_READ(PWM_Servo_Reactive);
#endif
#if HAS_REACTIVE_WRITE(PWM_SERVO)
        REACTIVE_DECLARE_FEATURE_WRITE(PWM_Servo_Reactive);
#endif
#if HAS_REACTIVE_EXEC(PWM_SERVO)
        REACTIVE_DECLARE_FEATURE_EXEC(PWM_Servo_Reactive);
#endif
#if HAS_REACTIVE_BRACKET_READ(PWM_SERVO)
        REACTIVE_DECLARE_FEATURE_BRACKET_READ(PWM_Servo_Reactive);
#endif
#if HAS_REACTIVE_BRACKET_WRITE(PWM_SERVO)
        REACTIVE_DECLARE_FEATURE_BRACKET_WRITE(PWM_Servo_Reactive);
#endif
#if HAS_REACTIVE_TIMEOUT(PWM_SERVO)
        REACTIVE_DECLARE_FEATURE_TIMEOUT(PWM_Servo_Reactive);
#endif
#if HAS_REACTIVE_WRITE_ERROR(PWM_SERVO)
        REACTIVE_DECLARE_FEATURE_WRITE_ERROR(PWM_Servo_Reactive);
#endif
#if HAS_REACTIVE_READ_ERROR(PWM_SERVO)
        REACTIVE_DECLARE_FEATURE_READ_ERROR(PWM_Servo_Reactive);
#endif
#if HAS_REACTIVE_EXEC_ERROR(PWM_SERVO)
        REACTIVE_DECLARE_FEATURE_EXEC_ERROR(PWM_Servo_Reactive);
#endif
    public:
        DALHAL_DECLARE_REACTIVE_TABLE(PWM_Servo_Reactive);

        PWM_Servo_Reactive(const char* type);

        HALOperationResult Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) override;

    };
    
}