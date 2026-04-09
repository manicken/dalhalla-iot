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

#include "DALHAL_CommonSchemas_Time.h"

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Float.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr SchemaUInt    refreshTimeMsField  = { "refreshtimems", FieldPolicy::OneOfGroup, 1}; // here we dont define min/max 
        constexpr SchemaFloat    refreshTimeSecField  = { "refreshtimesec", FieldPolicy::OneOfGroup, 1.0f}; // here we dont define min/max 
        constexpr SchemaFloat    refreshTimeMinField  = { "refreshtimemin", FieldPolicy::OneOfGroup, 1.0f}; // here we dont define min/max 
        constexpr const SchemaTypeBase* refreshGroupItems[] = {&refreshTimeMsField, &refreshTimeSecField, &refreshTimeMinField, nullptr};

        constexpr SchemaOneOfFieldsGroup   refreshTimeGroupFields = {"refreshtime", FieldPolicy::Optional, Gui::UseInline, refreshGroupItems}; // here refreshtimems defines what name to use for the BSON output
        constexpr SchemaOneOfFieldsGroup   refreshTimeGroupFieldsRequired = {"refreshtime", FieldPolicy::Required, Gui::UseInline, refreshGroupItems};
    }

}