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

#include "DALHAL_JSON_Schema.h"

namespace DALHAL {
    
    namespace JsonSchema {
        constexpr FieldUID uid{};
        
        constexpr FieldUInt    refreshTimeMs  = { "refreshtimems", FieldType::UInt, FieldFlag::AnyOfGroup, 1, 0, 0 }; // zero max mean infinite
        constexpr FieldFloat    refreshTimeSec  = { "refreshtimesec", FieldType::Float, FieldFlag::AnyOfGroup, 1, 0, 0 }; // zero max mean infinite
        constexpr FieldFloat    refreshTimeMin  = { "refreshtimemin", FieldType::Float, FieldFlag::AnyOfGroup, 1, 0, 0 }; // zero max mean infinite
        constexpr const FieldBase* refreshGroupItems[] = {&refreshTimeMs, &refreshTimeSec, &refreshTimeMin, nullptr};

        constexpr AnyOfGroup   refreshTimeGroup = {"refreshtimems", FieldFlag::Optional, refreshGroupItems}; // here refreshtimems defines what name to use for the BSON output
        
        constexpr FieldString source = { "source", FieldType::UID_Path, FieldFlag::Optional, nullptr, 0 }; // zero lenght mean as long as one wants
        constexpr FieldString eventSource = { "event_source", FieldType::UID_Path, FieldFlag::Optional, nullptr, 0 }; // zero lenght mean as long as one wants
        
        constexpr ModeConjunctionDefine refreshModeConjunctions[] = {
            { &refreshTimeGroup, true },  // group must exist for this mode
            { &source, true },            // source must exist
            { &eventSource, false },      // event_source must NOT exist
            { nullptr, false}
        };
        constexpr ModeConjunctionDefine eventModeConjunctions[] = {
            { &refreshTimeGroup, false },  // group must NOT exist for this mode
            { &source, true },            // source must exist
            { &eventSource, true },      // event_source must exist
            { nullptr, false}
        };
        constexpr ModeConjunctionDefine scriptModeConjunctions[] = {
            { &refreshTimeGroup, false },  // group must NOT exist for this mode
            { &source, false },            // source must NOT exist
            { &eventSource, false },      // event_source must NOT exist
            { nullptr, false}
        };
        constexpr const ModeSelector templateDeviceModes[] = {
            {"refresh mode", refreshModeConjunctions},
            {"event mode", eventModeConjunctions},
            {"script mode", scriptModeConjunctions},
            {nullptr, nullptr}
        };

        constexpr const FieldBase* templateFields[] = {
            &uid,
            &refreshTimeGroup,
            &source,
            &eventSource,
            nullptr
        };

        constexpr JsonSchema::Device templateDevice = {
            templateFields,
            templateDeviceModes
        };

    }
}