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

#include "DALHAL_DeviceContainer_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfRegistryItems.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Devices/_Registry/DALHAL_DevicesRegistry.h>

#include "DALHAL_DeviceContainer.h"
#include <DALHAL/Core/Manager/DALHAL_DeviceManager.h>

namespace DALHAL {

    namespace JsonSchema {

        namespace DeviceContainer {

            constexpr SchemaArrayOfRegistryItems itemsField = {"items", FieldPolicy::Required, RootDevicesRegistry, "ROOT"};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &itemsField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "DeviceContainer",
                fields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(const DALHAL::DeviceCreateContext& context, DALHAL::DeviceContainer* out) {
                const JsonVariant& jsonObj = *(context.jsonObjItem);
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*(context.jsonObjItem)));

                const JsonArray& jsonArray = JsonSchema::DeviceContainer::itemsField.GetValidatedJsonArray(jsonObj);
                
                uint32_t deviceCountTmp = 0;
                int arraySize = jsonArray.size();

                // First pass: count valid entries
                for (int i=0;i<arraySize;i++) {
                    if (Device::DisabledOrCommentItem(jsonArray[i]) == true) { continue; }
                    deviceCountTmp++;
                }
                
                out->deviceCount = deviceCountTmp;
                if (out->deviceCount == 0) {
                    out->devices = nullptr;
                    GlobalLogger.Error(F("DeviceContainer JSON cfg does not contain any valid devices!\n" 
                                        "Hint: Check that all entries have 'type' and 'uid' fields, and match known types."));
                    return;
                }

                // Allocate space for all devices
                out->devices = new Device*[out->deviceCount]();

                if (out->devices == nullptr) {
                    out->deviceCount = 0;
                    GlobalLogger.Error(F("Failed to allocate device array"));
                    return;
                }

                // Second pass: actually create and store devices
                uint32_t index = 0;
                for (int i=0;i<arraySize;i++) {
                    const JsonVariant& jsonItem = jsonArray[i];
                    if (Device::DisabledOrCommentItem(jsonItem) == true) { continue; }
                    out->devices[index++] = DeviceManager::CreateDeviceFromJSON(jsonItem);
                }
                std::string devCountStr = std::to_string(out->deviceCount);
                GlobalLogger.Info(F("Created sub devices: "), devCountStr.c_str());
            }

        }

    }

}