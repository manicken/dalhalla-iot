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

#include "DALHAL_ValueReactive.h"

namespace DALHAL {

    ReactiveHALValue::ReactiveHALValue() : HALValue(), context(nullptr), setCallback(nullptr) /*getCallback(nullptr),*/ {}
    ReactiveHALValue::ReactiveHALValue(int32_t v) : HALValue(v), context(nullptr), setCallback(nullptr) /*getCallback(nullptr),*/ {}
    ReactiveHALValue::ReactiveHALValue(uint32_t v) : HALValue(v), context(nullptr), setCallback(nullptr) /*getCallback(nullptr),*/ {}
    ReactiveHALValue::ReactiveHALValue(float v) : HALValue(v), context(nullptr), setCallback(nullptr) /*getCallback(nullptr),*/ {}

    void ReactiveHALValue::setCallbacks(void* ctx, OnSetCallback setcb, OnGetCallback getcb) {
        if (ctx == nullptr) {
            // cannot execute without context
            setCallback = nullptr;
           // getCallback = nullptr;
            return;
        }
        this->context = ctx;
        this->setCallback = setcb;
        //this->getCallback = getcb;
    }

    void ReactiveHALValue::set(int32_t v) { HALValue::set(v); triggerSetCallback(); }
    void ReactiveHALValue::set(uint32_t v) { HALValue::set(v); triggerSetCallback(); }
    void ReactiveHALValue::set(float v) { HALValue::set(v); triggerSetCallback(); }
/*
    int32_t ReactiveHALValue::asInt() const {
        triggerGetCallback();
        return HALValue::asInt();
    }
    uint32_t ReactiveHALValue::asUInt() const {
        triggerGetCallback();
        return HALValue::asUInt();
    }
    float ReactiveHALValue::asFloat() const {
        triggerGetCallback();
        return HALValue::asFloat();
    }
*/
    ReactiveHALValue::operator int32_t() const {
        return asInt();
    }

    ReactiveHALValue::operator uint32_t() const {
        return asUInt();
    }
    ReactiveHALValue::operator uint8_t() const {
        return asUInt();
    }
    ReactiveHALValue::operator uint16_t() const {
        return asUInt();
    }

    ReactiveHALValue::operator float() const {
        return asFloat();
    }

    ReactiveHALValue& ReactiveHALValue::operator=(int32_t v)   { set(v); return *this; }
    ReactiveHALValue& ReactiveHALValue::operator=(uint32_t v)  { set(v); return *this; }
    ReactiveHALValue& ReactiveHALValue::operator=(float v)     { set(v); return *this; }
    ReactiveHALValue& ReactiveHALValue::operator=(const HALValue& other) {
        HALValue::operator=(other);  // copy the value
        triggerSetCallback();           // call the reactive callback
        return *this;
    }
}