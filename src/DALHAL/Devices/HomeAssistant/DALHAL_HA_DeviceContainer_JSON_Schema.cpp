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

#include "DALHAL_HA_DeviceContainer_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfRegistryItems.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_DeviceTypeReg.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_CreateFunctionContext.h>

#include "DALHAL_HA_DeviceContainer.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace HA_DeviceContainer {

            constexpr SchemaArrayOfRegistryItems itemsField = {"items", FieldPolicy::Required, HA_DeviceRegistry, "ROOT.HOMEASSISTANT"};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &itemsField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "HA_DeviceContainer",
                fields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(DALHAL::HA_CreateFunctionContext& context, DALHAL::HA_DeviceContainer* out) {
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*(context.jsonObjItem)));
                const JsonArray& jsonArray = JsonSchema::HA_DeviceContainer::itemsField.GetValidatedJsonArray(*(context.jsonObjItem));
                
                uint32_t deviceCountTmp = 0;
                int arraySize = jsonArray.size();

                // First pass: count valid entries
                for (int i=0;i<arraySize;i++) {
                    if (Device::DisabledOrCommentItem(jsonArray[i])) { continue; } // comment or disabled item
                    deviceCountTmp++;
                }
                
                out->deviceCount = deviceCountTmp;
                if (deviceCountTmp == 0) {
                    out->devices = nullptr;
                    GlobalLogger.Error(F("DeviceContainer JSON cfg does not contain any valid devices!\n" 
                                        "Hint: Check that all entries have 'type' and 'uid' fields, and match known types."));
                    return;
                }

                // Allocate space for all devices
                out->devices = new Device*[deviceCountTmp]();

                if (out->devices == nullptr) {
                    out->deviceCount = 0;
                    GlobalLogger.Error(F("Failed to allocate device array"));
                    return;
                }

                // Second pass: actually create and store devices
                uint32_t index = 0;
                
                for (int i=0;i<arraySize;i++) {
                    const JsonVariant& jsonItem = jsonArray[i];
                    if (Device::DisabledOrCommentItem(jsonItem)) { continue; } // comment or disabled item

                    const char* type_cStr = JsonSchema::CommonBase::typeField.ExtractFrom(jsonItem);
                    const Registry::Item& regItem = Registry::GetItem(HA_DeviceRegistry, type_cStr);
                    // reuse the passed context as it also carry other important instances or references
                    context.jsonObjItem = &jsonItem;
                    context.deviceType = regItem.typeName;
                    out->devices[index++] = regItem.def->Create_Function(context);
                }
                std::string devCountStr = std::to_string(deviceCountTmp);
                GlobalLogger.Info(F("Created sub devices: "), devCountStr.c_str());
            }

        }

    }

}