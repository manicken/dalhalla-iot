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

#include "DALHAL_ValueReactive.h"

namespace DALHAL {

    ReactiveHALValue::ReactiveHALValue() : HALValue(), callback(nullptr), context(nullptr) {}
    ReactiveHALValue::ReactiveHALValue(int32_t v) : HALValue(v), callback(nullptr), context(nullptr) {}
    ReactiveHALValue::ReactiveHALValue(uint32_t v) : HALValue(v), callback(nullptr), context(nullptr) {}
    ReactiveHALValue::ReactiveHALValue(float v) : HALValue(v), callback(nullptr), context(nullptr) {}

    void ReactiveHALValue::setCallback(OnSetCallback cb, void* ctx) {
        callback = cb;
        context = ctx;
    }

    void ReactiveHALValue::set(int32_t v) { HALValue::set(v); triggerCallback(); }
    void ReactiveHALValue::set(uint32_t v) { HALValue::set(v); triggerCallback(); }
    void ReactiveHALValue::set(float v) { HALValue::set(v); triggerCallback(); }

    ReactiveHALValue& ReactiveHALValue::operator=(int32_t v)   { set(v); return *this; }
    ReactiveHALValue& ReactiveHALValue::operator=(uint32_t v)  { set(v); return *this; }
    ReactiveHALValue& ReactiveHALValue::operator=(float v)     { set(v); return *this; }
    ReactiveHALValue& ReactiveHALValue::operator=(const HALValue& other) {
        HALValue::operator=(other);  // copy the value
        triggerCallback();           // call the reactive callback
        return *this;
    }
}