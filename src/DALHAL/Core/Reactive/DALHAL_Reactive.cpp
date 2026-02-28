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

#include "DALHAL_Reactive.h"
#include "../../Support/DALHAL_Logger.h"

namespace DALHAL {

    ReactiveEvent::SimpleContext::SimpleContext(uint32_t& _current) : current(_current) { }

    bool ReactiveEventDefault(void* context) {
        GlobalLogger.Error(F("ReactiveEventDefault triggered"));
        return false;
    }
    ReactiveEvent::ReactiveEvent(CheckFn _checkFn, Deleter _deleteFn, void* context) : checkFn(checkFn), deleteFn(_deleteFn), context(context) {
        if (_checkFn == nullptr || _deleteFn == nullptr || context == nullptr ) {
            this->checkFn = ReactiveEventDefault;
            GlobalLogger.Error(F("ReactiveEvent using ReactiveEventDefault"));
        }
    }
    ReactiveEvent::~ReactiveEvent() {
        if (deleteFn && context) {
            deleteFn(context);
        }
    }

    bool ReactiveEvent::SimpleReactiveEventCheck(void* context) {
        ReactiveEvent::SimpleContext* ctx = static_cast<ReactiveEvent::SimpleContext*>(context);
        if (ctx->current != ctx->lastSeen) {
            ctx->lastSeen = ctx->current; // update to current
            return true;
        }
        return false;
    }

}