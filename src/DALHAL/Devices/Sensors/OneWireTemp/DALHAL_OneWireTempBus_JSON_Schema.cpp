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

#include "DALHAL_OneWireTempBus_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfObjects.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>

#include "DALHAL_OneWireTempDevice_JSON_Schema.h"

#include "DALHAL_OneWireTempBus.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace OneWireTempBus {

            constexpr SchemaArrayOfObjects itemsField = {"items", FieldPolicy::Required, Gui::UseInline, &OneWireTempDevice::Root, EmptyPolicy::Error};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &CommonPins::InputOutputPinField,
                &itemsField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "OneWireTempBus",
                fields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Error,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(DALHAL::DeviceCreateContext& context, DALHAL::OneWireTempBus* out) {
                const JsonVariant& jsonObj = *(context.jsonObjItem);
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(jsonObj)); 
                out->pin = JsonSchema::CommonPins::InputOutputPinField.ExtractFrom(jsonObj);
        

                out->oneWire = new OneWire(out->pin);
                out->dTemp = new DallasTemperature(out->oneWire);
                out->dTemp->setWaitForConversion(false);

                out->deviceCount = 0;
                const JsonArray& items = JsonSchema::OneWireTempBus::itemsField.GetValidatedJsonArray(jsonObj);
                int itemCount = items.size();
                // first pass count valid devices
                for (int i=0;i<itemCount;i++) {
                    const JsonVariant& item = items[i];
                    if (Device::DisabledOrCommentItem(item)) { continue; }
                    out->deviceCount++;
                }
                out->devices = new Device*[static_cast<size_t>(out->deviceCount)]();
                uint32_t index = 0;
                DeviceCreateContext createContext;
                createContext.deviceType = "OneWireTempDevice";

                for (int i=0;i<itemCount;i++) {
                    const JsonVariant& item = items[i];
                    if (Device::DisabledOrCommentItem(item)) { continue; }

                    createContext.jsonObjItem = &item;
                    out->devices[index++] = new DALHAL::OneWireTempDevice(createContext);
                }
            }

        }

        namespace OneWireTempBusAtRoot {

            constexpr const SchemaTypeBase* fieldsAtRoot[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &CommonTime::refreshTimeGroupFieldsRequired, // required for now as this device need to run refresh in background
                                                // if it should not depend on automatic refresh, it do need a cmd that start a conversion
                                                // that then emit a reactive event when the convertion is done
                &CommonPins::InputOutputPinField,
                &OneWireTempBus::itemsField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "OneWireTempBusAtRoot",
                fieldsAtRoot,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(DALHAL::DeviceCreateContext& context, DALHAL::OneWireTempBusAtRoot* out) {
                const JsonVariant& jsonObj = *(context.jsonObjItem);
                // empty function for future use
            }

        }

    }

}