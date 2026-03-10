/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

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

#pragma once


#include <stdlib.h>
#include <DALHAL/Core/Types/DALHAL_UID.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

namespace DALHAL {

    namespace JsonSchema {

        enum class FieldType {
            AnyOfGroup,
            UID,
            UID_Path,
            Int,
            UInt,
            Float,
            Array,
            Pin
        };

        enum class FieldFlag {
            Required,
            Optional,
            AnyOfGroup // the higher group defines
        };

        struct FieldBase {
            const char* name;    // flash string
            FieldType type;
            FieldFlag flag;

            constexpr FieldBase(const char* n, FieldType t, FieldFlag f)
                : name(n), type(t), flag(f) {}
        };

        struct FieldInt : FieldBase {
            int32_t minValue;
            int32_t maxValue;
            int32_t defaultValue;
        };

        struct FieldUInt : FieldBase {
            uint32_t minValue;
            uint32_t maxValue;
            uint32_t defaultValue;

             constexpr FieldUInt(const char* n, FieldType t, FieldFlag f, uint32_t minValue, uint32_t maxValue, uint32_t defaultValue)
                : FieldBase(n, t, f), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
        };

        struct FieldFloat : FieldBase {
            float minValue;
            float maxValue;
            float defaultValue;
        };

        struct FieldBool : FieldBase {
            bool defaultValue;
        };

        struct FieldString : FieldBase {
            const char* defaultValue;  // flash string default
            uint16_t maxLength;

            constexpr FieldString(const char* n, FieldType t, FieldFlag f, const char* defVal, uint16_t maxLen)
                : FieldBase(n, t, f), defaultValue(defVal), maxLength(maxLen) {}
        };

        struct FieldUID : FieldString {
            constexpr FieldUID()
                : FieldString(DALHAL_KEYNAME_UID, FieldType::UID, FieldFlag::Required, nullptr, HAL_UID::Size) {}
        };
        

        struct FieldPin : FieldBase {
            uint8_t def;
        };

        struct FieldArray : FieldBase {
            const FieldBase** allowedChildren; // nullptr terminated
            const char* arrayName;             // e.g., "items", "groups"
        };

        // FieldGroup is a logical unit of fields where the group may be optional, 
        // and if present, at least one child field must exist. 
        // The group's presence can be overridden by ModeSelector rules.
        struct AnyOfGroup : FieldBase {
            const FieldBase* const* fields;
            constexpr AnyOfGroup(FieldFlag f, const FieldBase* const* fields)
                : FieldBase(nullptr, FieldType::AnyOfGroup, f), fields(fields) {}
        };

        struct ModeConjunctionDefine {
            const FieldBase* fieldRef;
            bool required; // true = must exist, false = must explicit NOT exist
        };

        struct ModeSelector {
            uint32_t modeId = 0;
            const char* name; // optional
            const ModeConjunctionDefine* conjunctions; // nullptr terminated
            constexpr ModeSelector(uint32_t modeId, const char* name, const ModeConjunctionDefine* conjunctions)
                : modeId(modeId), name(name), conjunctions(conjunctions) {}
        };

        struct Device {
            const FieldBase* const* fields;
            const ModeSelector* const* modes;
        };

    } // namespace JsonSchema
}