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

#include "DALHAL_ReactiveTypes.h"

#include <cstdint>


#include <DALHAL/Core/Types/DALHAL_OperationResult.h>
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>
#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Support/DALHAL_DeleterTemplate.h>
#include <DALHAL/Core/Reactive/DALHAL_ReactiveTypes.h>
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {
    
    class Reactive {
    public:
#ifndef DALHAL_REACTIVE_NOT_USE_TYPESAFE_TEMPLATE_BASED_TABLES
        template<typename TDevice>
        static std::string GetDeviceEventNames(const EventDescriptorT<TDevice>* table)
        {
            std::string names;
            names += '[';
            bool first = true;
            for (size_t i = 0; table[i].name != nullptr; ++i)
            {
                if (first == false) {
                    names += ',';
                } else {
                    first = false;
                }
                names += '"' + table[i].name + '"';
            }
            names += ']';
            return names;
        }
#else
        static std::string GetDeviceEventNames(const EventDescriptor* table);
#endif
    

#ifndef DALHAL_REACTIVE_NOT_USE_TYPESAFE_TEMPLATE_BASED_TABLES
        template<typename TDevice>
        static HALOperationResult GetSimpleReactiveEventImpl(TDevice* device, ZeroCopyString& name, ReactiveEvent** out, const EventDescriptorT<TDevice>* table)
        {
            /*for (size_t i = 0; table[i].name != nullptr; ++i)
            {
                if (name.EqualsIC(table[i].name) == false) {
                    continue;
                }
                if (out) {
                    //uint32_t* counterPtr = &(device->*(table[i].counter));
                    //*out = new ReactiveEvent(*counterPtr);
                    *out = new ReactiveEvent(device->*(table[i].counter));
                }
                return HALOperationResult::Success;
            }*/
            for (const EventDescriptorT<TDevice>* entry = table; entry->name != nullptr; ++entry)
            {
                if (name.EqualsIC(entry->name) == false) {
                    continue;
                }
                if (out) {
                    //uint32_t* counterPtr = &(device->*(table[i].counter));
                    //*out = new ReactiveEvent(*counterPtr);
                    *out = new ReactiveEvent(device->*(entry->counter));
                }
                return HALOperationResult::Success;
            }
            return HALOperationResult::ReactiveEventByNameNotFound;
        }
#else
        static HALOperationResult GetSimpleReactiveEventImpl(Device* device, ZeroCopyString& name, ReactiveEvent** out, const EventDescriptor* table);
#endif

#ifndef DALHAL_REACTIVE_NOT_USE_TYPESAFE_TEMPLATE_BASED_TABLES
        /** !!!WARNING!!! while using this function there is not any non type safe variant */
        template<typename TDevice>
        static HALOperationResult GetReactiveEventImpl(TDevice* device, ZeroCopyString& name, ReactiveEvent** out, const EventDescriptorT<TDevice>* table)
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
#else
        // there is no implementation of a non simple variant here
#endif
    };
}