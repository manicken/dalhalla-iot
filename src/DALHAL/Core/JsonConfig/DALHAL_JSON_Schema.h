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
#include <DALHAL/Core/Types/DALHAL_Registry.h>
#include "DALHAL_JSON_SchemaFieldBase.h"

namespace DALHAL {

    namespace JsonSchema {

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
            constexpr FieldUID(FieldFlag f)
                : FieldString(DALHAL_KEYNAME_UID, FieldType::UID, f, nullptr, HAL_UID::Size) {}
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

        /**
         * AnyOfGroup is a logical unit of fields where the group may be optional, 
         * and if present, at least one child field must exist. 
         * The group's presence can be overridden by ModeSelector rules.
         */
        struct AnyOfGroup : FieldBase {
            const FieldBase* const* fields;
            constexpr AnyOfGroup(const char* outputName, FieldFlag f, const FieldBase* const* fields)
                : FieldBase(outputName, FieldType::AnyOfGroup, f), fields(fields) {}
        };

        

        /** 
         * FieldArray represents a homogeneous array where every element must conform 
         * to the same JsonSchema::Device definition. This is used for structured data 
         * with a fixed schema (no type selection per element).
         */
        struct FieldArray : FieldBase {
            const JsonSchema::Device* subtype;
            constexpr FieldArray(const char* n, FieldFlag f, const JsonSchema::Device* subtype)
                : FieldBase(n, FieldType::Array, f), subtype(subtype) {}
        };

        /**
         * FieldRegistryArray represents a polymorphic array where each element is a device 
         * selected from a Registry::Item table. Each entry resolves its type at runtime 
         * (typically via a type field) and is validated/created using the corresponding 
         * registry definition.
         */
        struct FieldRegistryArray : FieldBase {
            const Registry::Item* subtypes;
            constexpr FieldRegistryArray(const char* n, FieldFlag f, const Registry::Item* subtypes)
                : FieldBase(n, FieldType::Array, f), subtypes(subtypes) {}
        };
        
        /**
         * used for ordinary JSON objects, i.e. enclosed by {}
         */
        struct FieldObject : FieldBase {
            const JsonSchema::Device* subtype;
            constexpr FieldObject(const char* n, FieldFlag f, const JsonSchema::Device* subtype)
                : FieldBase(n, FieldType::Object, f), subtype(subtype) {}
        };

        struct FieldHexBytes : FieldString {
            uint8_t byteCount;

            constexpr FieldHexBytes(const char* name,
                                FieldFlag flag,
                                const char* defaultValue,
                                uint8_t byteCount)
                : FieldString(name, FieldType::HexBytes, flag, defaultValue, 0),
                byteCount(byteCount) {}
        };


        extern const FieldString typeField;
        extern const FieldUID uidFieldRequired;
        extern const FieldUID uidFieldOptional;

    } // namespace JsonSchema
}