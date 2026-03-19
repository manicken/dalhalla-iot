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

#include "DALHAL_CommonSchemes_Base.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldUID uidFieldRequired{DALHAL_COMMON_CFG_NAME_UID, FieldFlag::Required};
        constexpr FieldUID uidFieldOptional{DALHAL_COMMON_CFG_NAME_UID, FieldFlag::Optional};

        constexpr FieldString typeField = {DALHAL_COMMON_CFG_NAME_TYPE, FieldType::String, FieldFlag::Required, nullptr, 0};

        constexpr FieldString noteField = {DALHAL_COMMON_CFG_NAME_NOTE, FieldType::String, FieldFlag::Optional, nullptr, 0};

        constexpr FieldBool disabledField = {DALHAL_COMMON_CFG_NAME_DISABLED, FieldFlag::Optional, false};

    }

}