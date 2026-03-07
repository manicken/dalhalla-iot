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

#include "DALHAL_PCF8574x_Reactive.h"
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    PCF8574x_Reactive::PCF8574x_Reactive(const char* type) : Device(type) {}

    DALHAL_DEFINE_GET_REACTIVE_EVENT_FUNC(PCF8574x_Reactive);

    DALHAL_DEFINE_REACTIVE_TABLE(PCF8574x_Reactive) = {

#if HAS_REACTIVE_CUSTOM(I2C_DEVICE_PCF8574X)
        DALHAL_REACTIVE_ENTRY(PCF8574x_Reactive, Interrupt),
#endif
#if HAS_REACTIVE_BEGIN(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_BEGIN(PCF8574x_Reactive),
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_CYCLE_COMPLETE(PCF8574x_Reactive),
#endif
#if HAS_REACTIVE_VALUE_CHANGE(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_VALUE_CHANGE(PCF8574x_Reactive),
#endif
#if HAS_REACTIVE_STATE_CHANGE(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_STATE_CHANGE(PCF8574x_Reactive),
#endif
#if HAS_REACTIVE_READ(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_READ(PCF8574x_Reactive),
#endif
#if HAS_REACTIVE_WRITE(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_WRITE(PCF8574x_Reactive),
#endif
#if HAS_REACTIVE_EXEC(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_EXEC(PCF8574x_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_READ(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_BRACKET_READ(PCF8574x_Reactive),
#endif
#if HAS_REACTIVE_BRACKET_WRITE(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_BRACKET_WRITE(PCF8574x_Reactive),
#endif
#if HAS_REACTIVE_TIMEOUT(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_TIMEOUT(PCF8574x_Reactive),
#endif
#if HAS_REACTIVE_WRITE_ERROR(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_WRITE_ERROR(PCF8574x_Reactive),
#endif
#if HAS_REACTIVE_READ_ERROR(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_READ_ERROR(PCF8574x_Reactive),
#endif
#if HAS_REACTIVE_EXEC_ERROR(I2C_DEVICE_PCF8574X)
        REACTIVE_ENTRY_EXEC_ERROR(PCF8574x_Reactive),
#endif
        REACTIVE_ENTRY__TERMINATOR_()
    };
}