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

#include "DALHAL_HA_JsonSchema_Common.h"

#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_String.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_FieldsGroup.h>

namespace DALHAL {

    namespace JsonSchema {

        namespace HomeAssistant {

            constexpr SchemaString nameField = {"name", FieldPolicy::Required};
            constexpr SchemaString hass_uidField = {"hass_uid", FieldPolicy::Required};
            constexpr SchemaString hass_prev_uidField = {"hass_prev_uid", FieldPolicy::Optional};

            constexpr const SchemaTypeBase* common_items[] = { 
                &nameField,
                &hass_uidField,
                &hass_prev_uidField,
                nullptr
            };

            constexpr SchemaFieldsGroup common_fields = {
                "common",
                common_items,
                Gui::UseInline,
                /*sizeof(disabled_uidreq_note_group_items)/sizeof(disabled_uidreq_note_group_items[0])*/ // future implementation
            };
            
        }

    }
    
}