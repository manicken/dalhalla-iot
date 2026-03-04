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

#include <cstdint>
#include <string>

#include <DALHAL/Core/Types/DALHAL_Value.h>

namespace DALHAL {

    class ReactiveHALValue : public HALValue {
    public:
        using OnSetCallback = void(*)(void* context);
        using OnGetCallback = void(*)(void* context);

    private:
        void* context = nullptr; // usually a Device*, can be anything
        OnSetCallback setCallback = nullptr;
        OnGetCallback getCallback = nullptr;
        
    public:
        ReactiveHALValue();
        ReactiveHALValue(int32_t v);
        ReactiveHALValue(uint32_t v);
        ReactiveHALValue(float v);

        /** 
         * sets the context and callbacks, 
         * if any of the callbacks are not used it should be nullptr, 
         * if ctx is not set then the two callbacks will automatically be set to nullptr
         */
        void setCallbacks(void* ctx, OnSetCallback setcb, OnGetCallback getcb);

        inline void triggerSetCallback() {
            if (setCallback)
                setCallback(context);
        }
        inline void triggerGetCallback() const {
            if (getCallback)
                getCallback(context);
        }

        void set(int32_t v);
        void set(uint32_t v);
        void set(float v);

        int32_t asInt() const;
        uint32_t asUInt() const;
        float asFloat() const;

        operator uint8_t() const;
        operator uint16_t() const;
        operator uint32_t() const;
        operator int32_t() const;
        operator float() const;

        // Override operator=
        ReactiveHALValue& operator=(int32_t v);
        ReactiveHALValue& operator=(uint32_t v);
        ReactiveHALValue& operator=(float v);
        ReactiveHALValue& operator=(const HALValue& other);
    };
}