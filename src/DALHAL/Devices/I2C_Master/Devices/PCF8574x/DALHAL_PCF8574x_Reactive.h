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

    class PCF8574x_Reactive : public Device {
    protected:
#if HAS_REACTIVE_CUSTOM(I2C_DEVICE_PCF8574X)
        DALHAL_DECLARE_REACTIVE_FEATURE(Interrupt);
#endif
#if HAS_REACTIVE_BEGIN(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_FEATURE_BEGIN();
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_CYCLE_COMPLETE();
#endif
#if HAS_REACTIVE_VALUE_CHANGE(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_FEATURE_VALUE_CHANGE();
#endif
#if HAS_REACTIVE_STATE_CHANGE(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_FEATURE_STATE_CHANGE();
#endif
#if HAS_REACTIVE_READ(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_FEATURE_READ();
#endif
#if HAS_REACTIVE_WRITE(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_FEATURE_WRITE();
#endif
#if HAS_REACTIVE_EXEC(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_FEATURE_EXEC();
#endif
#if HAS_REACTIVE_BRACKET_READ(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_FEATURE_BRACKET_READ();
#endif
#if HAS_REACTIVE_BRACKET_WRITE(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_FEATURE_BRACKET_WRITE();
#endif
#if HAS_REACTIVE_TIMEOUT(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_FEATURE_TIMEOUT();
#endif
#if HAS_REACTIVE_WRITE_ERROR(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_FEATURE_WRITE_ERROR();
#endif
#if HAS_REACTIVE_READ_ERROR(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_FEATURE_READ_ERROR();
#endif
#if HAS_REACTIVE_EXEC_ERROR(I2C_DEVICE_PCF8574X)
        REACTIVE_DECLARE_FEATURE_EXEC_ERROR();
#endif
    public:
        DALHAL_DECLARE_REACTIVE_TABLE(PCF8574x_Reactive, eventTable);

        PCF8574x_Reactive(const char* type);

        HALOperationResult Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) override;

    };
    
}