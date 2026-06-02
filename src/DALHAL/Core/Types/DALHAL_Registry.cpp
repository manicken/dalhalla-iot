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
#include <DALHAL/Core/Reactive/DALHAL_ReactiveConfig.h>

#include <DALHAL/API/DALHAL_BlockStreamer.h>
#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>

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
            cb("{\"type\":\"start_chunked\"}", CmdCbType::Control);
            cb("{\"regitems\":[", CmdCbType::Data);

            std::string buffer;
            buffer.reserve(1024);

            const auto* items = reg.items;

            for (size_t i = 0; i < reg.count; i++)
            {
                if (buffer.size() > 900) {
                    cb(buffer.c_str(), CmdCbType::Data);
                    buffer.clear();
                }

                if (!buffer.empty()) buffer += ',';

                buffer += '"';
                buffer += items[i].typeName;
                buffer += '"';
            }

            if (!buffer.empty()) {
                cb(buffer.c_str(), CmdCbType::Data);
            }

            cb("]}", CmdCbType::Data);
            cb("{\"type\":\"end_chunked\"}", CmdCbType::Control);
        }

        void PrintTo(const Registry::DeviceRegistry& reg, CommandCallback cb) {
            const Registry::Item* items = reg.items;
            size_t itemCount = reg.count;

            DALHAL::BlockStreamer bs(cb, "registry", DALHAL::BlockStreamer::DataType::Json);
            DALHAL::StringBuilderStreamer& sbs = bs.writer();

            sbs.write(F("{\"regitems\":["));

            for (size_t i=0; i<itemCount; i++)
            {
                const Registry::Item& regItem = items[i];

                if (i > 0) { sbs.write(','); } 

                sbs.write(F("{\"name\":\""));
                sbs.write(regItem.typeName);
                sbs.write(F("\",\"events\":["));

                if (regItem.def->reactiveTable != nullptr) {
                    bool first = true;
                    for (const EventDescriptor* entry = regItem.def->reactiveTable; entry->name; entry++) {
                    
                        if (first == false) { sbs.write(','); }
                        else { first = false; }
                        sbs.write('"'); sbs.write(entry->name); sbs.write('"');
                    }
                }
                sbs.write(']');

                sbs.write(F(",\"functions\":{"));

                if (regItem.def->functionTable != nullptr) {
                    regItem.def->functionTable->PrintTo(sbs);
                }
                sbs.write('}');
                sbs.write('}');

            }
            sbs.write(']');
            sbs.write('}');
            sbs.flush();
        }

    }

}