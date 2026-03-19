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

    class OneWireTempBus_Reactive : public Device {
    protected:
#if HAS_REACTIVE_CUSTOM(ONE_WIRE_TEMP_BUS)
        DALHAL_DECLARE_REACTIVE_FEATURE(OneWireTempBus_Reactive, Custom1);
        DALHAL_DECLARE_REACTIVE_FEATURE(OneWireTempBus_Reactive, Custom2);
        DALHAL_DECLARE_REACTIVE_FEATURE(OneWireTempBus_Reactive, Custom3);
#endif
#if HAS_REACTIVE_BEGIN(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_FEATURE_BEGIN(OneWireTempBus_Reactive);
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_CYCLE_COMPLETE(OneWireTempBus_Reactive);
#endif
#if HAS_REACTIVE_VALUE_CHANGE(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_FEATURE_VALUE_CHANGE(OneWireTempBus_Reactive);
#endif
#if HAS_REACTIVE_STATE_CHANGE(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_FEATURE_STATE_CHANGE(OneWireTempBus_Reactive);
#endif
#if HAS_REACTIVE_READ(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_FEATURE_READ(OneWireTempBus_Reactive);
#endif
#if HAS_REACTIVE_WRITE(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_FEATURE_WRITE(OneWireTempBus_Reactive);
#endif
#if HAS_REACTIVE_EXEC(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_FEATURE_EXEC(OneWireTempBus_Reactive);
#endif
#if HAS_REACTIVE_BRACKET_READ(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_FEATURE_BRACKET_READ(OneWireTempBus_Reactive);
#endif
#if HAS_REACTIVE_BRACKET_WRITE(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_FEATURE_BRACKET_WRITE(OneWireTempBus_Reactive);
#endif
#if HAS_REACTIVE_TIMEOUT(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_FEATURE_TIMEOUT(OneWireTempBus_Reactive);
#endif
#if HAS_REACTIVE_WRITE_ERROR(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_FEATURE_WRITE_ERROR(OneWireTempBus_Reactive);
#endif
#if HAS_REACTIVE_READ_ERROR(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_FEATURE_READ_ERROR(OneWireTempBus_Reactive);
#endif
#if HAS_REACTIVE_EXEC_ERROR(ONE_WIRE_TEMP_BUS)
        REACTIVE_DECLARE_FEATURE_EXEC_ERROR(OneWireTempBus_Reactive);
#endif
    public:
        DALHAL_DECLARE_REACTIVE_TABLE(OneWireTempBus_Reactive);

        OneWireTempBus_Reactive(const char* type);

        HALOperationResult Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) override;

    };
    
}