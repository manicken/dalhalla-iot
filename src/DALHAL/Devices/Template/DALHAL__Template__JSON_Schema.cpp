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

#include "DALHAL__Template__JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_ComplexTypes.h>

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_ArrayPrimitive.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Bool.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Float.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_HardwarePinOrVirtualPIN.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_HexBytes.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Int.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Number.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringAnyOfArrayConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringAnyOfByFuncConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringBase.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringSizeConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringUID.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringUID_Path.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr SchemaHardwarePin pinField = { DALHAL_COMMON_CFG_NAME_PIN, FieldPolicy::Required, (GPIO_manager::PinFunc::IN | GPIO_manager::PinFunc::OUT) };

        constexpr const SchemaTypeBase* fields[] = {
            &disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &pinField,
            nullptr,
        };

        constexpr JsonObjectSchema _Template_ = {
            "_Template_",
            fields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}