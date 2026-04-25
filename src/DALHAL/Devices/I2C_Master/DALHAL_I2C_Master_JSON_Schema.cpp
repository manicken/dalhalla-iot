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

#include "DALHAL_I2C_Master_JSON_Schema.h"

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfRegistryItems.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>

#include "_DevicesRegistry/DALHAL_I2C_Master_DevicesRegistry.h"

#include "DALHAL_I2C_Master.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace I2C_Master {

            constexpr SchemaHardwarePin sckpinField = {"sckpin", FieldPolicy::Required, (GPIO_manager::PinFunc::OUT)};
            constexpr SchemaHardwarePin sdapinField = {"sdapin", FieldPolicy::Required, (GPIO_manager::PinFunc::OUT | GPIO_manager::PinFunc::IN)};

            constexpr SchemaUInt freqField = {"freq", FieldPolicy::Optional, (unsigned int)(100*1000)};
            constexpr SchemaUInt busindexField = {"busindex", FieldPolicy::Optional, (unsigned int)0, (unsigned int)0, (unsigned int)1};

            constexpr SchemaArrayOfRegistryItems itemsField = {"items", FieldPolicy::Required, I2C_DeviceRegistry, "ROOT.I2C_Master"};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &sckpinField,
                &sdapinField,
                &itemsField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "I2C_Master",
                fields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(DALHAL::DeviceCreateContext& context, DALHAL::I2C_Master* out) {
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*(context.jsonObjItem)));

                out->sckpin = JsonSchema::I2C_Master::sckpinField.ExtractFrom(*(context.jsonObjItem));
                out->sdapin = JsonSchema::I2C_Master::sdapinField.ExtractFrom(*(context.jsonObjItem));
                out->freq = JsonSchema::I2C_Master::freqField.ExtractFrom(*(context.jsonObjItem));
                if (out->freq < 100000) out->freq = 100000; // defaults to 100khz

#if defined(ESP32)
                int busIndex = JsonSchema::I2C_Master::busindexField.ExtractFrom(*(context.jsonObjItem));
                if (busIndex == 1) {
                    out->wire = &Wire1;
                }
                else
#endif
                    out->wire = &Wire;

                const JsonArray items = JsonSchema::I2C_Master::itemsField.GetValidatedJsonArray(*(context.jsonObjItem));

                int itemCount = items.size();
                // first pass count valid items
                size_t validItemCount = 0;
                for (int i=0;i<itemCount;i++) {
                    const JsonVariant& item = items[i];
                    if (Device::DisabledOrCommentItem(item)) { continue; }
                    validItemCount++;
                }
                // second pass actually create the devices
                out->deviceCount = validItemCount;
                out->devices = new Device*[validItemCount]();
                int index = 0;
                I2C_Master_CreateFunctionContext createContext(*out->wire);
                for (int i=0;i<itemCount;i++) {
                    const JsonVariant& item = items[i];
                    if (Device::DisabledOrCommentItem(item)) { continue; }
                    
                    const char* type_cStr = JsonSchema::CommonBase::typeField.ExtractFrom(item);
                    const Registry::Item& regItem = Registry::GetItem(I2C_DeviceRegistry, type_cStr);
                    // no nullcheck is needed as ValidateJSON ensures that all types are correct
                    createContext.jsonObjItem = &item;
                    createContext.deviceType = regItem.typeName; // use static/flash string
                    
                    out->devices[index++] = regItem.def->Create_Function(createContext); // regItem.typeName is a flash const so it's safe to use
                }
            }

        }

    }

}