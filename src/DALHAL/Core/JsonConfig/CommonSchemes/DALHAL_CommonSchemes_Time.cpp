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

#include "DALHAL_CommonSchemes_Time.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldUInt    refreshTimeMsField  = { "refreshtimems", FieldType::UInt, FieldFlag::AnyOfGroup, 1, 0, 0 }; // zero max mean infinite
        constexpr FieldFloat    refreshTimeSecField  = { "refreshtimesec", FieldType::Float, FieldFlag::AnyOfGroup, 1, 0, 0 }; // zero max mean infinite
        constexpr FieldFloat    refreshTimeMinField  = { "refreshtimemin", FieldType::Float, FieldFlag::AnyOfGroup, 1, 0, 0 }; // zero max mean infinite
        constexpr const FieldBase* refreshGroupItems[] = {&refreshTimeMsField, &refreshTimeSecField, &refreshTimeMinField, nullptr};

        constexpr AnyOfGroup   refreshTimeGroupFields = {"refreshtimems", FieldFlag::Optional, refreshGroupItems}; // here refreshtimems defines what name to use for the BSON output
        
    }

}