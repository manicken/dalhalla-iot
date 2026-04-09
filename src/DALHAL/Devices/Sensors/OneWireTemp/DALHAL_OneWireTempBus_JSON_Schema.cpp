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

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Array.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>

#include "DALHAL_OneWireTempDevice_JSON_Schema.h"

namespace DALHAL {

    namespace JsonSchema {

        constexpr SchemaArray itemsField = {"items", FieldPolicy::Required, &OneWireTempDevice, EmptyPolicy::Error};

        constexpr const SchemaTypeBase* fields[] = {
            &disabled_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &InputOutputPinField,
            &itemsField,
            nullptr,
        };

        constexpr const SchemaTypeBase* fieldsAtRoot[] = {
            &disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &refreshTimeGroupFieldsRequired, // required for now as this device need to run refresh in background
                                             // if it should not depend on automatic refresh, it do need a cmd that start a conversion
                                             // that then emit a reactive event when the convertion is done
            &InputOutputPinField,
            &itemsField,
            nullptr,
        };

        constexpr JsonObjectSchema OneWireTempBus = {
            "OneWireTempBus",
            fields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Error,
            UnknownFieldPolicy::Warn,
        };

        constexpr JsonObjectSchema OneWireTempBusAtRoot = {
            "OneWireTempBusAtRoot",
            fieldsAtRoot,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}