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

#include "DALHAL_CommonSchemas_Base.h"

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Types.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldUID uidFieldRequired{DALHAL_COMMON_CFG_NAME_UID, FieldPolicy::Required};
        constexpr FieldUID uidFieldOptional{DALHAL_COMMON_CFG_NAME_UID, FieldPolicy::Optional};

        constexpr FieldStringBase typeField = {DALHAL_COMMON_CFG_NAME_TYPE, FieldPolicy::Required, nullptr};

        constexpr FieldStringBase noteField = {DALHAL_COMMON_CFG_NAME_NOTE, FieldPolicy::Optional, nullptr};

        constexpr FieldBool disabledField = {DALHAL_COMMON_CFG_NAME_DISABLED, FieldPolicy::Optional, false};

        constexpr const FieldBase* disabled_uidreq_note_group_items[] = { 
            &disabledField, 
            &uidFieldRequired, 
            &noteField, 
            nullptr
        };
        constexpr FieldsGroup disabled_uidreq_note_group = {
            disabled_uidreq_note_group_items,
            Gui::UseInline,
            /*sizeof(disabled_uidreq_note_group_items)/sizeof(disabled_uidreq_note_group_items[0])*/ // future implementation
        };

        constexpr const FieldBase* disabled_type_uidreq_note_group_items[] = {
            &disabled_uidreq_note_group,
            &typeField,
            nullptr
        };
        constexpr FieldsGroup disabled_type_uidreq_note_group = {
            disabled_type_uidreq_note_group_items,
            Gui::UseInline,
            /*sizeof(disabled_type_uidreq_note_group_items)/sizeof(disabled_type_uidreq_note_group_items[0])*/ // future implementation
        };
    }

}