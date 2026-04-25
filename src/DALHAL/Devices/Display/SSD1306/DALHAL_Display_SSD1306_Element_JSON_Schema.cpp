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

#include "DALHAL_Display_SSD1306_Element_JSON_Schema.h"


#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfObjects.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_String.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringHexBytes.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringSizeConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringUID_Path.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Consumer.h>
#include <DALHAL/Devices/_Registry/DALHAL_DevicesRegistry.h>

#include "DALHAL_Display_SSD1306_Element.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace Display_SSD1306_Element {

            // SSD1306_Element "device"
            constexpr SchemaUInt xField = {"x", FieldPolicy::Required, (unsigned int)0, (unsigned int)128, (unsigned int)0};
            constexpr SchemaUInt yField = {"y", FieldPolicy::Required, (unsigned int)0, (unsigned int)64, (unsigned int)0};
            constexpr SchemaString labelField = {"label", FieldPolicy::Optional};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabledField, 
                &CommonBase::uidFieldRequired, 
                &xField, 
                &yField, 
                &labelField, 
                &CommonConsumer::sourceField, 
                nullptr
            };

            constexpr JsonObjectSchema Root = {
                "SSD1306 element",
                fields,
                nullptr,
                nullptr,
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn
            };

            void Extractors::Apply(const DALHAL::DeviceCreateContext& context, DALHAL::Display_SSD1306_Element* out) {
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*(context.jsonObjItem)));
                out->xPos = JsonSchema::Display_SSD1306_Element::xField.ExtractFrom(*(context.jsonObjItem));
                out->yPos = JsonSchema::Display_SSD1306_Element::yField.ExtractFrom(*(context.jsonObjItem));
                out->label = std::string(JsonSchema::Display_SSD1306_Element::labelField.ExtractFrom(*(context.jsonObjItem)));

                const char* sourceStr = JsonSchema::CommonConsumer::sourceField.ExtractFrom(*(context.jsonObjItem));
                if (sourceStr != nullptr) {
                    out->cdaSource = new CachedDeviceAccess();
                    if (out->cdaSource->Set(sourceStr) == false) {
                        delete out->cdaSource;
                        out->cdaSource = nullptr;
                    }
                }
                else
                    out->cdaSource = nullptr;
            }

        }

    }

}