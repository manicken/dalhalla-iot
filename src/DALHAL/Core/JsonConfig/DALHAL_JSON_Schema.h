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
            Object,
            HardwarePin,
            HardwarePinOrVirtualPin
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

            constexpr FieldInt(const char* n, FieldType t, FieldFlag f, int32_t minValue, int32_t maxValue, int32_t defaultValue)
                : FieldBase(n, t, f), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
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

            constexpr FieldFloat(const char* n, FieldType t, FieldFlag f, float minValue, float maxValue, float defaultValue)
                : FieldBase(n, t, f), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
        };

        struct FieldBool : FieldBase {
            bool defaultValue;

            constexpr FieldBool(const char* n, FieldType t, FieldFlag f, bool defaultValue)
                : FieldBase(n, t, f), defaultValue(defaultValue) {}
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
        

        struct FieldHardwarePin : FieldBase {
            uint16_t mode;
            constexpr FieldHardwarePin(const char* n, FieldFlag f, uint16_t mode)
                : FieldBase(n, FieldType::HardwarePin, f), mode(mode) {} // here pin could use Int but that is not how pins are validated they instead use GPIO_manager for validity
        };

        struct FieldHardwarePinOrVirtualPIN : FieldBase {
            uint16_t mode;
            constexpr FieldHardwarePinOrVirtualPIN(const char* n, FieldFlag f, uint16_t mode)
                : FieldBase(n, FieldType::HardwarePinOrVirtualPin, f), mode(mode) {} // here pin could use Int but that is not how pins are validated they instead use GPIO_manager for validity
        };

        

        // FieldGroup is a logical unit of fields where the group may be optional, 
        // and if present, at least one child field must exist. 
        // The group's presence can be overridden by ModeSelector rules.
        struct AnyOfGroup : FieldBase {
            const FieldBase* const* fields;
            constexpr AnyOfGroup(const char* outputName, FieldFlag f, const FieldBase* const* fields)
                : FieldBase(outputName, FieldType::AnyOfGroup, f), fields(fields) {}
        };

        struct ModeConjunctionDefine {
            const FieldBase* fieldRef;
            bool required; // true = must exist, false = must explicit NOT exist
        };

        struct ModeSelector {
            const char* name; // optional
            const ModeConjunctionDefine* conjunctions; // nullptr terminated
            constexpr ModeSelector(const char* name, const ModeConjunctionDefine* conjunctions)
                : name(name), conjunctions(conjunctions) {}
        };

        struct Device {
            const FieldBase* const* fields;
            const ModeSelector* modes;
        };

        struct FieldArray : FieldBase {
            const Device* subtype;
            constexpr FieldArray(const char* n, FieldFlag f, const Device* subtype)
                : FieldBase(n, FieldType::Array, f), subtype(subtype) {}
        };

        struct FieldObject : FieldBase {
            const Device* subtype;
            constexpr FieldObject(const char* n, FieldFlag f, const Device* subtype)
                : FieldBase(n, FieldType::Object, f), subtype(subtype) {}
        };

    } // namespace JsonSchema
}