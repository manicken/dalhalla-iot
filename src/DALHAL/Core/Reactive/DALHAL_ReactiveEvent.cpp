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

#include "DALHAL_ReactiveEvent.h"

#include <DALHAL/Support/DALHAL_Logger.h>

namespace DALHAL {
    ReactiveEvent::SimpleContext::SimpleContext(uint32_t& current) : current(current) { }

    bool ReactiveEventDefault(void* context) {
        GlobalLogger.Error(F("ReactiveEventDefault triggered"));
        return false;
    }
    ReactiveEvent::ReactiveEvent(CheckFn checkFn) : checkFn(checkFn), deleteFn(nullptr), context(nullptr) {
        if ( this->checkFn == nullptr ) {
            this->checkFn = ReactiveEventDefault;
            GlobalLogger.Error(F("ReactiveEvent using ReactiveEventDefault"));
        }
    }
    ReactiveEvent::ReactiveEvent(CheckFn checkFn, Deleter deleteFn, void* context) : checkFn(checkFn), deleteFn(deleteFn), context(context) {
        if ( this->checkFn == nullptr || this->deleteFn == nullptr || this->context == nullptr ) {
            this->checkFn = ReactiveEventDefault;
            GlobalLogger.Error(F("ReactiveEvent using ReactiveEventDefault"));
        }
    }
    ReactiveEvent::ReactiveEvent(uint32_t* current) {
        this->checkFn = ReactiveEvent::SimpleReactiveEventCheck;
        this->deleteFn = DeleteAs<ReactiveEvent::SimpleContext>;
        context = new ReactiveEvent::SimpleContext(*current);
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