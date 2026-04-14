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
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Array.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>

#include "DALHAL_REGO600_Register_JSON_Schema.h"

namespace DALHAL {

    namespace JsonSchema {

        constexpr SchemaHardwarePin rxpinField = {"rxpin", FieldPolicy::Required, (GPIO_manager::PinFunc::IN)};
        constexpr SchemaHardwarePin txpinField = {"txpin", FieldPolicy::Required, (GPIO_manager::PinFunc::OUT)};

        constexpr SchemaUInt requestDelayMsField = {"requestDelayMs", FieldPolicy::Optional, 0, 0, 10};

        constexpr SchemaArray itemsField = {"items", FieldPolicy::Required, Gui::RenderAllAllowedValues, "", &REGO600_Register, EmptyPolicy::Error};

        constexpr const SchemaTypeBase* fields[] = {
            &disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &rxpinField,
            &txpinField,
            &requestDelayMsField,
            &refreshTimeGroupFieldsRequired,
            &itemsField,
            nullptr,
        };

        constexpr JsonObjectSchema REGO600 = {
            "REGO600",
            fields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}