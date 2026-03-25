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

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_BaseTypes.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Devices/_Registry/DALHAL_DevicesRegistry.h>

namespace DALHAL {

    namespace JsonSchema {

        // SSD1306_Element "device"
        constexpr FieldUInt xField = {"x", FieldPolicy::Required, 0, 128, 0};
        constexpr FieldUInt yField = {"y", FieldPolicy::Required, 0, 64, 0};
        constexpr FieldString labelField = {"label", FieldPolicy::Optional, nullptr, 0};
        constexpr FieldString sourceField = {"source", FieldType::UID_Path, FieldPolicy::Optional, nullptr, 0};

        constexpr const FieldBase* elementFields[] = {&typeField, &uidFieldRequired, &xField, &yField, &labelField, &sourceField, nullptr};

        constexpr JsonObjectSchema elementObject = {"SSD1306 element", elementFields, nullptr, nullptr, EmptyPolicy::Warn, UnknownFieldPolicy::Warn};

        // SSD1306 display
        constexpr FieldUInt widthField = {"width", FieldPolicy::Required, 8, 128, 128};
        constexpr FieldUInt heightField = {"height", FieldPolicy::Required, 8, 64, 64};
        constexpr FieldHexBytes addrField = {"addr", FieldPolicy::Required, "3C", 1};

        constexpr FieldArray itemsField = {"items", FieldPolicy::Required, &elementObject};
        

        constexpr const FieldBase* fields[] = {
            &typeField,         // DALHAL_CommonSchemas_Base
            &uidFieldRequired,  // DALHAL_CommonSchemas_Base
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