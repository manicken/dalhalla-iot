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

#include "DALHAL_Registry.h"

#include <DALHAL/Core/Types/DALHAL_DeviceFunctionTable.h>
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

namespace DALHAL {

    namespace Registry {
        
        const Registry::Item& GetItem(const Registry::DeviceRegistry& reg, const char* type) {
            int i=0;
            const Registry::Item* items = reg.items;
            size_t itemCount = reg.count;
            for (size_t i=0;i<itemCount;i++) {
                const Registry::Item& regItem = items[i];
                if (regItem.typeName == nullptr) continue;
                if (strcasecmp(regItem.typeName, type) == 0) return regItem;
            }
            return Registry::TerminatorItem;
        }

        void GetTypeNames(const Registry::DeviceRegistry& reg, CommandCallback cb)
        {
            cb("{\"type\":\"start_chunked\"}", CmdCbType::Text);
            cb("{\"regitems\":[", CmdCbType::Binary);

            std::string buffer;
            buffer.reserve(1024);

            const auto* items = reg.items;

            for (size_t i = 0; i < reg.count; i++)
            {
                if (buffer.size() > 900) {
                    cb(buffer.c_str(), CmdCbType::Binary);
                    buffer.clear();
                }

                if (!buffer.empty()) buffer += ',';

                buffer += '"';
                buffer += items[i].typeName;
                buffer += '"';
            }

            if (!buffer.empty()) {
                cb(buffer.c_str(), CmdCbType::Binary);
            }

            cb("]}", CmdCbType::Binary);
            cb("{\"type\":\"end_chunked\"}", CmdCbType::Text);
        }

        void ToString(const Registry::DeviceRegistry& reg, CommandCallback cb) {
            const Registry::Item* items = reg.items;
            size_t itemCount = reg.count;

            cb("{\"type\":\"start_chunked\"}", CmdCbType::Text);

            cb("{\"regitems\":[", CmdCbType::Binary);
            std::string eventList;
            eventList.reserve(REACTIVE_ALL_ENTRY_NAMES_SIZE+2);
            bool first = true;
            for (size_t i=0; i<itemCount; i++)
            {
                const Registry::Item& regItem = items[i];

                if (first == false) { cb(",", CmdCbType::Binary); }
                else { first = false; }
                //ret += ' ';
                //ret += '{';
                cb("{\"name\":\"", CmdCbType::Binary);
                cb(regItem.typeName, CmdCbType::Binary);
                eventList.clear();
                eventList = "\",\"events\":[";
                //cb("\",\"events\":[", CmdCbType::Binary);
                if (regItem.def->reactiveTable != nullptr) {
                    bool first2 = true;
                    for (const EventDescriptor* entry = regItem.def->reactiveTable; entry->name; entry++) {
                    
                        if (first2 == false) { eventList += ','; }
                        else { first2 = false; }
                        
                        
                        eventList += '"'; eventList += entry->name; eventList += '"';
                        //cb(eventEntryName.c_str(), CmdCbType::Binary);
                        //cb("\"", CmdCbType::Binary); cb(entry->name, CmdCbType::Binary); cb("\"", CmdCbType::Binary);
                    }
                }
                eventList += ']';
                cb(eventList.c_str(), CmdCbType::Binary);
                //ret += ']';
                cb(",\"functions\":{", CmdCbType::Binary);
                if (regItem.def->functionTable != nullptr) {
                    cb(regItem.def->functionTable->ToString().c_str(), CmdCbType::Binary);
                }
                cb("}}", CmdCbType::Binary);
                //ret += '}';
                //ret += '}';
            }
            cb("]}", CmdCbType::Binary);
            //ret += ']';
            //ret += '}';
            //return ret;
            cb("{\"type\":\"end_chunked\"}", CmdCbType::Text);
        }

    }

}