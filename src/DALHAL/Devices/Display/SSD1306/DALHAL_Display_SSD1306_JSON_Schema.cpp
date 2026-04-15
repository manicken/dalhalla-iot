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

#include "DALHAL_Display_SSD1306_JSON_Schema.h"


#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfObjects.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_String.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringHexBytes.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringSizeConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringUID_Path.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Devices/_Registry/DALHAL_DevicesRegistry.h>

namespace DALHAL {

    namespace JsonSchema {

        // SSD1306_Element "device"
        constexpr SchemaUInt xField = {"x", FieldPolicy::Required, 0, 128, 0};
        constexpr SchemaUInt yField = {"y", FieldPolicy::Required, 0, 64, 0};
        constexpr SchemaString labelField = {"label", FieldPolicy::Optional};
        constexpr SchemaStringUID_Path sourceField = {"source", FieldPolicy::Optional};

        constexpr const SchemaTypeBase* elementFields[] = {&disabledField, &uidFieldRequired, &xField, &yField, &labelField, &sourceField, nullptr};

        constexpr JsonObjectSchema elementObject = {"SSD1306 element", elementFields, nullptr, nullptr, EmptyPolicy::Warn, UnknownFieldPolicy::Warn};

        // SSD1306 display
        constexpr SchemaUInt widthField = {"width", FieldPolicy::Required, 8, 128, 128};
        constexpr SchemaUInt heightField = {"height", FieldPolicy::Required, 8, 64, 64};
        constexpr SchemaStringHexBytes addrField = {"addr", FieldPolicy::Required, "3C", 1};

        constexpr SchemaArrayOfObjects itemsField = {"items", FieldPolicy::Required, &elementObject};
        

        constexpr const SchemaTypeBase* fields[] = {
            &disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &widthField,
            &heightField,
            &addrField,
            &itemsField,
            nullptr,
        };

        constexpr JsonObjectSchema Display_SSD1306 = {
            "Display_SSD1306",
            fields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}