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

#include "DALHAL_TX433_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfRegistryItems.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>
#include "DALHAL_TX433_UnitTypeRegistry.h"

#include "DALHAL_TX433.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace TX433 {

            constexpr SchemaArrayOfRegistryItems unitsField = {"units", FieldPolicy::Optional, TX433_UnitTypeRegistry, "ROOT.TX433"};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group,
                &CommonPins::OutputPinField,
                &unitsField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "TX433",
                fields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(DALHAL::DeviceCreateContext& context, DALHAL::TX433* out) {
                const JsonVariant& jsonObj = *(context.jsonObjItem);
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(jsonObj));
                out->pin = JsonSchema::CommonPins::OutputPinField.ExtractFrom(jsonObj);

                // as this whole field is optional we first see if it exists
                if (jsonObj.containsKey(JsonSchema::TX433::unitsField.name) == false) {
                    return;
                }
                // if it exists it's validated that it's a JsonArray so we can safely just do:
                JsonArray jsonItems = JsonSchema::TX433::unitsField.GetValidatedJsonArray(jsonObj);
                int jsonItems_count = jsonItems.size();

                int unitCount = 0;
                // first pass count enabled and non comment items
                for (int i=0;i<jsonItems_count;i++) {
                    if (Device::DisabledOrCommentItem(jsonItems[i]) == true) { continue; }
                    unitCount++;
                }
                // second pass create units(devices)
                out->units = new Device*[unitCount]();
                uint32_t index = 0;
                TX433_Unit_CreateFunctionContext createContext(out->pin);
                for (int i=0;i<jsonItems_count;i++) {
                    if (Device::DisabledOrCommentItem(jsonItems[i]) == true) { continue; }
                    const JsonVariant& item = jsonItems[i];
                    createContext.jsonObjItem = &item;
                    const char* type_cStr = JsonSchema::CommonBase::typeField.ExtractFrom(item);
                    const Registry::Item& regItem = Registry::GetItem(TX433_UnitTypeRegistry, type_cStr);
                    // here it's safe to use regItem as JSON validation ensure that device type exists
                    createContext.deviceType = regItem.typeName;
                    createContext.ApplyFunction = static_cast<const TX433_UNIT_RegistryDefine*>(regItem.def)->Apply;

                    out->units[index++] = regItem.def->Create_Function(createContext);
                }

            }

        }

    }

}