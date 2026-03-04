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

#include "DALHAL_LEDC_Servo_Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    LEDC_Servo_Reactive::LEDC_Servo_Reactive(const char* type) : Device(type) {}

    HALOperationResult LEDC_Servo_Reactive::Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) {
        return Reactive::GetSimpleReactiveEventImpl(this, zcFuncName, reactiveEventOut, eventTable);
    }
    DALHAL_DEFINE_REACTIVE_TABLE(LEDC_Servo_Reactive, eventTable) = {

#if HAS_REACTIVE_BEGIN(LEDC_SERVO)
        REACTIVE_ENTRY_BEGIN(LEDC_Servo_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(LEDC_SERVO)
        REACTIVE_ENTRY_CYCLE_COMPLETE(LEDC_Servo_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(LEDC_SERVO)
        REACTIVE_ENTRY_VALUE_CHANGE(LEDC_Servo_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(LEDC_SERVO)
        REACTIVE_ENTRY_STATE_CHANGE(LEDC_Servo_Reactive),
#endif
#if HAS_REACTIVE_READ(LEDC_SERVO)
        REACTIVE_ENTRY_READ(LEDC_Servo_Reactive),
#endif
#if HAS_REACTIVE_WRITE(LEDC_SERVO)
        REACTIVE_ENTRY_WRITE(LEDC_Servo_Reactive),
#endif
#if HAS_REACTIVE_EXEC(LEDC_SERVO)
        REACTIVE_ENTRY_EXEC(LEDC_Servo_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(LEDC_SERVO)
        REACTIVE_ENTRY_BRACKET_READ(LEDC_Servo_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(LEDC_SERVO)
        REACTIVE_ENTRY_BRACKET_WRITE(LEDC_Servo_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(LEDC_SERVO)
        REACTIVE_ENTRY_TIMEOUT(LEDC_Servo_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(LEDC_SERVO)
        REACTIVE_ENTRY_WRITE_ERROR(LEDC_Servo_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(LEDC_SERVO)
        REACTIVE_ENTRY_READ_ERROR(LEDC_Servo_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(LEDC_SERVO)
        REACTIVE_ENTRY_EXEC_ERROR(LEDC_Servo_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}