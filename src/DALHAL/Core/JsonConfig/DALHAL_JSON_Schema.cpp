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
        constexpr FieldUID uidFieldRequired{FieldFlag::Required};
        constexpr FieldUID uidFieldOptional{FieldFlag::Optional};

        constexpr FieldString typeField = {DALHAL_KEYNAME_TYPE, FieldType::String, FieldFlag::Required, nullptr, 0};
        
        constexpr FieldUInt    refreshTimeMsField  = { "refreshtimems", FieldType::UInt, FieldFlag::AnyOfGroup, 1, 0, 0 }; // zero max mean infinite
        constexpr FieldFloat    refreshTimeSecField  = { "refreshtimesec", FieldType::Float, FieldFlag::AnyOfGroup, 1, 0, 0 }; // zero max mean infinite
        constexpr FieldFloat    refreshTimeMinField  = { "refreshtimemin", FieldType::Float, FieldFlag::AnyOfGroup, 1, 0, 0 }; // zero max mean infinite
        constexpr const FieldBase* refreshGroupItems[] = {&refreshTimeMsField, &refreshTimeSecField, &refreshTimeMinField, nullptr};

        constexpr AnyOfGroup   refreshTimeGroupFields = {"refreshtimems", FieldFlag::Optional, refreshGroupItems}; // here refreshtimems defines what name to use for the BSON output
        
        constexpr FieldString sourceField = { "source", FieldType::UID_Path, FieldFlag::Optional, nullptr, 0 }; // zero lenght mean as long as one wants
        constexpr FieldString eventSourceField = { "event_source", FieldType::UID_Path, FieldFlag::Optional, nullptr, 0 }; // zero lenght mean as long as one wants
        
        constexpr ModeConjunctionDefine refreshModeConjunctions[] = {
            { &refreshTimeGroupFields, true },  // group must exist for this mode
            { &sourceField, true },            // source must exist
            { &eventSourceField, false },      // event_source must NOT exist
            { nullptr, false}
        };
        constexpr ModeConjunctionDefine eventModeConjunctions[] = {
            { &refreshTimeGroupFields, false },  // group must NOT exist for this mode
            { &sourceField, true },            // source must exist
            { &eventSourceField, true },      // event_source must exist
            { nullptr, false}
        };
        constexpr ModeConjunctionDefine scriptModeConjunctions[] = {
            { &refreshTimeGroupFields, false },  // group must NOT exist for this mode
            { &sourceField, false },            // source must NOT exist
            { &eventSourceField, false },      // event_source must NOT exist
            { nullptr, false}
        };
        constexpr const ModeSelector templateDeviceModes[] = {
            {"refresh mode", refreshModeConjunctions},
            {"event mode", eventModeConjunctions},
            {"script mode", scriptModeConjunctions},
            {nullptr, nullptr}
        };

        constexpr const FieldBase* templateFields[] = {
            &typeField,
            &uidFieldRequired,
            &refreshTimeGroupFields,
            &sourceField,
            &eventSourceField,
            nullptr
        };

        constexpr JsonSchema::JsonObjectScheme templateJsonObject = {
            "template",
            templateFields,
            templateDeviceModes
        };

    }
}