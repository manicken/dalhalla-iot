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

    class WS2812_Reactive : public Device {
    protected:
#if HAS_REACTIVE_CUSTOM(WS2812)
        DALHAL_DECLARE_REACTIVE_FEATURE(WS2812_Reactive, Custom1); // keep for future custom features
#endif
#if HAS_REACTIVE_BEGIN(WS2812)
        REACTIVE_DECLARE_FEATURE_BEGIN(WS2812_Reactive);
#endif
#if HAS_REACTIVE_CYCLE_COMPLETE(WS2812)
        REACTIVE_DECLARE_CYCLE_COMPLETE(WS2812_Reactive);
#endif
#if HAS_REACTIVE_VALUE_CHANGE(WS2812)
        REACTIVE_DECLARE_FEATURE_VALUE_CHANGE(WS2812_Reactive);
#endif
#if HAS_REACTIVE_STATE_CHANGE(WS2812)
        REACTIVE_DECLARE_FEATURE_STATE_CHANGE(WS2812_Reactive);
#endif
#if HAS_REACTIVE_READ(WS2812)
        REACTIVE_DECLARE_FEATURE_READ(WS2812_Reactive);
#endif
#if HAS_REACTIVE_WRITE(WS2812)
        REACTIVE_DECLARE_FEATURE_WRITE(WS2812_Reactive);
#endif
#if HAS_REACTIVE_EXEC(WS2812)
        REACTIVE_DECLARE_FEATURE_EXEC(WS2812_Reactive);
#endif
#if HAS_REACTIVE_BRACKET_READ(WS2812)
        REACTIVE_DECLARE_FEATURE_BRACKET_READ(WS2812_Reactive);
#endif
#if HAS_REACTIVE_BRACKET_WRITE(WS2812)
        REACTIVE_DECLARE_FEATURE_BRACKET_WRITE(WS2812_Reactive);
#endif
#if HAS_REACTIVE_TIMEOUT(WS2812)
        REACTIVE_DECLARE_FEATURE_TIMEOUT(WS2812_Reactive);
#endif
#if HAS_REACTIVE_WRITE_ERROR(WS2812)
        REACTIVE_DECLARE_FEATURE_WRITE_ERROR(WS2812_Reactive);
#endif
#if HAS_REACTIVE_READ_ERROR(WS2812)
        REACTIVE_DECLARE_FEATURE_READ_ERROR(WS2812_Reactive);
#endif
#if HAS_REACTIVE_EXEC_ERROR(WS2812)
        REACTIVE_DECLARE_FEATURE_EXEC_ERROR(WS2812_Reactive);
#endif
    public:
        DALHAL_DECLARE_REACTIVE_TABLE(WS2812_Reactive);

        WS2812_Reactive(const char* type);

        HALOperationResult Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut) override;

    };
    
}