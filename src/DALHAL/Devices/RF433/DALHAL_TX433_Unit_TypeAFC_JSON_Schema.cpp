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

#include "DALHAL_TX433_Unit_TypeAFC_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_BaseTypes.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>

namespace DALHAL {

    namespace JsonSchema {
    /* could also use the following functions instead of specifying each possible value
        surely faster but will see if it also take less space
        static bool ValidateHex(void* ctx, const char* value) {
            if (!value || !value[0] || value[1] != '\0') return false; // single char only
            char c = value[0];
            return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
        }

        static std::string DescribeHex(void* ctx) {
            return R"(["0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"])";
        }
    */
        constexpr const char* ids[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", nullptr};
        constexpr ByArrayConstraints hexNumbersConstraint = {ids, ByArrayConstraints::Policy::IgnoreCase};
        constexpr FieldStringAnyOfArrayConstrained chField = {"ch", FieldPolicy::Optional, "0", &hexNumbersConstraint};
        constexpr FieldStringAnyOfArrayConstrained btnField = {"btn", FieldPolicy::Optional, "0", &hexNumbersConstraint};
        constexpr FieldUInt stateField = {"state", FieldPolicy::Optional, 0, 1, 0};

        constexpr const FieldBase* fields[] = {
            &disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &chField,
            &btnField,
            &stateField,
            nullptr,
        };

        constexpr JsonObjectSchema TX433_Unit_TypeAFC = {
            "TX433_Unit_TypeAFC",
            fields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}