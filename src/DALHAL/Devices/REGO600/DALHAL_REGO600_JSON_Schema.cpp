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

#include "DALHAL_REGO600_JSON_Schema.h"

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfObjects.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringAnyOfByFuncConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>

#include "DALHAL_REGO600_Register_JSON_Schema.h"

#include "DALHAL_REGO600.h"
#include "DALHAL_REGO600_Register.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace REGO600 {

            constexpr SchemaHardwarePin rxpinField = {"rxpin", FieldPolicy::Required, (GPIO_manager::PinFunc::IN)};
            constexpr SchemaHardwarePin txpinField = {"txpin", FieldPolicy::Required, (GPIO_manager::PinFunc::OUT)};

            constexpr SchemaUInt requestDelayMsField = {"requestDelayMs", FieldPolicy::Optional, (unsigned int)0, (unsigned int)0, (unsigned int)10};

            constexpr SchemaArrayOfObjects itemsField = {
                "items", 
                FieldPolicy::Required,
                Gui::RenderAllAllowedValues,
                &REGO600_Register::regnameField,
                &REGO600_Register::Root,
                EmptyPolicy::Error
            };

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &rxpinField,
                &txpinField,
                &requestDelayMsField,
                &CommonTime::refreshTimeGroupFieldsRequired,
                &itemsField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "REGO600",
                fields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(DALHAL::DeviceCreateContext& context, DALHAL::REGO600* out) {
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*(context.jsonObjItem)));
                out->rxPin = JsonSchema::REGO600::rxpinField.ExtractFrom(*(context.jsonObjItem));
                out->txPin = JsonSchema::REGO600::txpinField.ExtractFrom(*(context.jsonObjItem));

                const JsonArray items = JsonSchema::REGO600::itemsField.GetValidatedJsonArray(*(context.jsonObjItem));
                int itemCount = items.size();

                out->registerItemCount = 0;
                // first pass count valid items
                for (int i=0;i<itemCount;i++) {
                    if (Device::DisabledOrCommentItem(items[i])) { continue; }
                    out->registerItemCount++;
                }
                // second pass
                out->requestList = new Drivers::REGO600::Request*[out->registerItemCount]();
                out->registerItems = new Device*[out->registerItemCount]();
                int index = 0;
                DeviceCreateContext createContext;
                createContext.deviceType = "REGO600reg";
                for (int i=0;i<itemCount;i++) {
                    const JsonVariant& item = items[i];
                    if (Device::DisabledOrCommentItem(item)) { continue; }

                    createContext.jsonObjItem = &item;
                    DALHAL::REGO600_Register* itemReg = new DALHAL::REGO600_Register(createContext);

                    const char* regName = JsonSchema::REGO600_Register::regnameField.ExtractFrom(item);
                    // as the cfg is now fully validated this will not return a nullptr
                    const Drivers::REGO600::RegoLookupEntry* entry = Drivers::REGO600::SystemRegisterTableLockup(regName);
                    
                    // here value is passed by ref so that REGO600 driver can access and change the value,
                    // that makes REGO600register read function can then get the correct value
                    const Drivers::REGO600::OpCodeInfo& info = Drivers::REGO600::getCmdInfo(0x02); // system registers only
                    out->requestList[index] = new Drivers::REGO600::Request(
                        info, 
                        *entry,
                        itemReg->value
                    );
                    out->registerItems[index] = itemReg;
                    index++;
                }
                out->refreshTimeMs = JsonSchema::CommonTime::refreshTimeGroupFieldsRequired.ExtractFrom(*(context.jsonObjItem)).toUInt();

                unsigned long requestDelayMs = JsonSchema::REGO600::requestDelayMsField.ExtractFrom(*(context.jsonObjItem));
                
                out->rego600 = new Drivers::REGO600(out->rxPin, out->txPin, out->requestList, out->registerItemCount, out->refreshTimeMs, requestDelayMs);
            }

        }

    }

}