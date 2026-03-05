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
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

#ifdef DALHAL_REACTIVE_NOT_USE_TYPESAFE_TEMPLATE_BASED_TABLES

    bool CheckOrGetDeviceEvents( 
            const EventDescriptor* table,
            const char* checkName = nullptr, // optional: check if a name exists     
            const char** outNames = nullptr, // caller-provided array
            size_t maxNames                  // size of the array
        )
    {
        if (checkName) {
                // Mode: check if name exists
                for (size_t i = 0; table[i].name != nullptr; ++i) {
                    if (strcmp(table[i].name, checkName) == 0) {
                        return true;
                    }
                }
                return false;
            } else if (outNames && maxNames > 0) {
                // Mode: retrieve names into buffer
                size_t idx = 0;
                for (size_t i = 0; table[i].name != nullptr && idx < maxNames; ++i) {
                    outNames[idx++] = table[i].name;
                }
                if (idx < maxNames) outNames[idx] = nullptr; // null-terminate
                return true; // success
            }
            return false; // invalid usage
    }
    /*std::string Reactive::GetDeviceEventNames(const EventDescriptor* table)
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
    }*/
    
    HALOperationResult Reactive::GetSimpleReactiveEventImpl(DALHAL::Device* device, ZeroCopyString& name, ReactiveEvent** out, const EventDescriptor* table)
    {
       /* for (size_t i = 0; table[i].name != nullptr; ++i)
        {
            if (name.EqualsIC(table[i].name) == false) {
                continue;
            }
            if (out) {
                uint32_t* counterPtr = reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(device) + table[i].offset);

                *out = new ReactiveEvent(
                    ReactiveEvent::SimpleReactiveEventCheck,
                    DeleteAs<ReactiveEvent::SimpleContext>,
                    new ReactiveEvent::SimpleContext(*counterPtr)
                );
            }
            return HALOperationResult::Success;
        }*/
        for (const EventDescriptor* entry = table; entry->name != nullptr; ++entry)
        {
            if (name.EqualsIC(entry->name) == false) {
                continue;
            }
            if (out) {
                //uint32_t* counterPtr = &(device->*(table[i].counter));
                //*out = new ReactiveEvent(*counterPtr);
                *out = new ReactiveEvent(*(reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(device) + entry->offset)));
            }
            return HALOperationResult::Success;
        }
        return HALOperationResult::ReactiveEventByNameNotFound;
    }
#endif

}