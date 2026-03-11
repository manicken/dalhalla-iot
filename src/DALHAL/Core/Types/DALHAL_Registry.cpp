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

#include "DALHAL_Registry.h"

namespace DALHAL {

    namespace Registry {
        
        const Registry::Item& GetItem(const Registry::Item* reg, const char* type) {
            int i=0;
            while (true) {
                const Registry::Item& regItem = reg[i++];
                if (regItem.typeName == nullptr) break; // break on terminator entry
                if (strcasecmp(regItem.typeName, type) == 0) return regItem;
            }
            return Registry::TerminatorItem;
        }

        std::string ToString(const Registry::Item* reg) {
            std::string ret = "{\"regitems\":[";
            int i=0;
            bool first = true;
            for (const Registry::Item* regItem = reg; regItem->typeName != nullptr; ++regItem)
            {
                if (first == false) { ret += ","; }
                else { first = false; }
                ret += ' ';
                ret += '{';
                ret += "\"name\":\""; ret += regItem->typeName; ret += '"';
                ret += ",\"events\":[";
                if (regItem->def.reactiveTable != nullptr) {
                    bool first2 = true;
                    for (const EventDescriptor* entry = regItem->def.reactiveTable; entry->name; entry++) {
                    
                        if (first2 == false) { ret += ','; }
                        else { first2 = false; }
                        ret += '"'; ret += entry->name; ret += '"';
                    }
                }
                ret += "]}";
            }
            ret += "]}";
            return ret;
        }

    }

}