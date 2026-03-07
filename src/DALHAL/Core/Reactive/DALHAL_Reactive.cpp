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

    bool Reactive::CheckDeviceEvent(
            const EventDescriptor* table,
            const char* checkName)
    {
        if (!table || !checkName) {
            GlobalLogger.Error(F("Reactive::CheckDeviceEvent")); // no additional error needed as this is the only one generated here
            return false;
        }
        // check if name exists
        for (size_t i = 0; table[i].name != nullptr; ++i) {
            if (strcasecmp(table[i].name, checkName) == 0) {
                return true;
            }
        }
        return false;
    }

    size_t Reactive::GetDeviceEvents( 
            const EventDescriptor* table, 
            const char** outNames, // caller-provided array
            size_t maxNames)                  // size of the array
    {
        if (!table || !outNames || maxNames == 0) {
            GlobalLogger.Error(F("Reactive::GetDeviceEvents")); // no additional error needed as this is the only one generated here
            return 0;
        }
        // retrieve names into buffer
        size_t idx = 0;
        for (size_t i = 0; table[i].name != nullptr && idx < maxNames; ++i) {
            outNames[idx++] = table[i].name;
        }
        if (idx < maxNames) outNames[idx] = nullptr; // null-terminate
        return idx; // success if idx != 0
    }
        
    HALOperationResult Reactive::GetSimpleReactiveEventImpl(
        DALHAL::Device* device, 
        ZeroCopyString& name, 
        ReactiveEvent** out, 
        const EventDescriptor* table)
    {
        for (const EventDescriptor* entry = table; entry->name != nullptr; ++entry)
        {
            if (name.EqualsIC(entry->name) == false) {
                continue;
            }
            if (out) {
                *out = new ReactiveEvent(entry->getter(device));
            }
            return HALOperationResult::Success;
        }
        return HALOperationResult::ReactiveEventByNameNotFound;
    }

}