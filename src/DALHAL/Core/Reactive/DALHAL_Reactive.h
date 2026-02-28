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
#include "../../Support/DALHAL_DeleterTemplate.h"
#include "../Types/DALHAL_OperationResult.h"
#include "../Types/DALHAL_ZeroCopyString.h"

// Terminator for all tables
#define REACTIVE_TABLE_END { nullptr, nullptr }

namespace DALHAL {
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
        ReactiveEvent(CheckFn checkFn, Deleter deleteFn, void* context);

        static bool SimpleReactiveEventCheck(void* context);

        ~ReactiveEvent();

        inline bool CheckForEvent() {
            return checkFn(context);
        }
    };

    template<typename T>
    struct EventDescriptorT {
        const char* name;
        uint32_t T::* counter;
    };

    template<typename TDevice>
    HALOperationResult GetSimpleReactiveEventImpl(TDevice* device, ZeroCopyString& name, ReactiveEvent** out, const EventDescriptorT<TDevice>* table)
    {
        for (size_t i = 0; table[i].name != nullptr; ++i)
        {
            if (name.EqualsIC(table[i].name) == false) {
                continue;
            }
            if (out) {
                uint32_t* counterPtr = &(device->*(table[i].counter));

                *out = new ReactiveEvent(
                    ReactiveEvent::SimpleReactiveEventCheck,
                    DeleteAs<ReactiveEvent::SimpleContext>,
                    new ReactiveEvent::SimpleContext(*counterPtr)
                );
            }
            return HALOperationResult::Success;
        }
        return HALOperationResult::ReactiveEventByNameNotFound;
    }

    template<typename TDevice>
    HALOperationResult GetReactiveEventImpl(TDevice* device, ZeroCopyString& name, ReactiveEvent** out, const EventDescriptorT<TDevice>* table)
    {
        for (size_t i = 0; table[i].name != nullptr; ++i)
        {
            if (name.EqualsIC(table[i].name) == false) {
                continue;
            }
            if (out)
            {
                uint32_t* counterPtr = &(device->*(table[i].counter));

                *out = new ReactiveEvent(
                    TDevice::EventCheck,
                    DeleteAs<typename TDevice::ReactiveContext>,
                    new typename TDevice::ReactiveContext(*counterPtr)
                );
            }
            return HALOperationResult::Success;
        }
        return HALOperationResult::ReactiveEventByNameNotFound;
    }
}