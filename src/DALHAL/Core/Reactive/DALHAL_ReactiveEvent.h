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

#include <cstdint>
#include <cstddef>
#include <DALHAL/Support/DALHAL_DeleterTemplate.h>
#include <DALHAL/Config/DALHAL_BuildFlags.h>
#include <DALHAL/Support/DALHAL_Logger.h>

namespace  DALHAL
{
    /** used by consumers */
    class ReactiveEvent {
    public:
        using CheckFn  = bool (*)(void*);
        struct SimpleContext {
            uint32_t lastSeen;
            volatile uint32_t& current;
            SimpleContext(uint32_t& current);
        };
    private:
        CheckFn checkFn;
        Deleter deleteFn;
        void* context;
    public:
        ReactiveEvent() = delete;
        ReactiveEvent(ReactiveEvent&) = delete;
        /** used in special cases where the reactive event either allways trigger and in situations when it never trigger */
        ReactiveEvent(CheckFn _checkFn);
        ReactiveEvent(CheckFn checkFn, Deleter deleteFn, void* context);
        ReactiveEvent(uint32_t* current);

        static bool SimpleReactiveEventCheck(void* context);

        ~ReactiveEvent();

        inline bool CheckForEvent() {
            if (checkFn == nullptr) {
                GlobalLogger.Error(F("checkFn == nullptr"));
                return false;
            }
            return checkFn(context);
        }
    };
}