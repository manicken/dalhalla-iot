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

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DALHAL/Devices/I2C_Master/_DevicesRegistry/DALHAL_I2C_Master_DevicesRegistry.h>

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

#include "DALHAL_Display_SSD1306.h"
#include "DALHAL_Display_SSD1306_Element.h"

#include "DALHAL_Display_SSD1306_Element_JSON_Schema.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace Display_SSD1306 {

            constexpr SchemaUInt widthField = {"width", FieldPolicy::Required, (unsigned int)8, (unsigned int)128, (unsigned int)128};
            constexpr SchemaUInt heightField = {"height", FieldPolicy::Required, (unsigned int)8, (unsigned int)64, (unsigned int)64};
            constexpr SchemaUInt textsizeField = {"textsize", FieldPolicy::Optional, (unsigned int)1, (unsigned int)64, (unsigned int)1};
            constexpr SchemaStringHexBytes addrField = {"addr", FieldPolicy::Required, "3C", 1};

            constexpr SchemaArrayOfObjects itemsField = {"items", FieldPolicy::Required, &JsonSchema::Display_SSD1306_Element::Root};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &widthField,
                &heightField,
                &textsizeField,
                &addrField,
                &itemsField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "Display_SSD1306",
                fields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(const DALHAL::I2C_Master_CreateFunctionContext& context, DALHAL::Display_SSD1306* out) {
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*(context.jsonObjItem)));

                uint32_t width = JsonSchema::Display_SSD1306::widthField.ExtractFrom(*(context.jsonObjItem));
                uint32_t height = JsonSchema::Display_SSD1306::heightField.ExtractFrom(*(context.jsonObjItem));
                const char* addrStr = JsonSchema::Display_SSD1306::addrField.ExtractFrom(*(context.jsonObjItem));
                uint8_t addr = static_cast<uint8_t>(std::strtoul(addrStr, nullptr, 16));
                uint8_t textSize = JsonSchema::Display_SSD1306::textsizeField.ExtractFrom(*(context.jsonObjItem));

                out->display = new Adafruit_SSD1306(width, height, &(context.wire), -1); // -1 = no reset pin

                delay(200);
                if (out->display->begin(SSD1306_SWITCHCAPVCC, addr))
                {
                    out->display->clearDisplay();
                    out->display->setTextSize(textSize);
                    out->display->setTextColor(SSD1306_WHITE);
                    out->display->setCursor(0,0);
                    //display.println(F("Hello ESP32!"));
                    out->display->display(); // <--- push buffer to screen
                }

                const JsonArray& items = JsonSchema::Display_SSD1306::itemsField.GetValidatedJsonArray(*(context.jsonObjItem));

                int itemCount = items.size();
                // first pass count valid items
                size_t validItemCount = 0;
                for (int i=0;i<itemCount;i++) {
                    if (Device::DisabledOrCommentItem(items[i])) { continue; }
                    validItemCount++;
                }
                // second pass actually create the devices
                out->elementCount = validItemCount;
                out->elements = new Device*[validItemCount]();
                int index = 0;
                DeviceCreateContext createContext;
                createContext.deviceType = "Display_SSD1306_Element";
                for (int i=0;i<itemCount;i++) {
                    const JsonVariant& item = items[i];
                    if (Device::DisabledOrCommentItem(item)) { continue; }
                    createContext.jsonObjItem = &item;
                    out->elements[index++] = new DALHAL::Display_SSD1306_Element(createContext);
                }
            }

        }

    }

}