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

#pragma once


#include <stdlib.h>
#include <DALHAL/Core/Types/DALHAL_UID.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>
#include "DALHAL_JSON_Schema_BaseTypes.h"

namespace DALHAL {

    namespace JsonSchema {

        //************************************* */
        //*******  Native JSON Types ********** */
        //************************************* */

        struct FieldInt : FieldBase {
            int32_t minValue;
            int32_t maxValue;
            int32_t defaultValue;
            // can be used when inherited and used as a subtupe
            constexpr FieldInt(const char* n, FieldType t, FieldFlag f, int32_t minValue, int32_t maxValue, int32_t defaultValue)
                : FieldBase(n, t, f), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
            // explicit select type to int
            constexpr FieldInt(const char* n, FieldFlag f, int32_t minValue, int32_t maxValue, int32_t defaultValue)
                : FieldBase(n, FieldType::Int, f), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
        };

        struct FieldUInt : FieldBase {
            uint32_t minValue;
            uint32_t maxValue;
            uint32_t defaultValue;
            // can be used when inherited and used as a subtupe
            constexpr FieldUInt(const char* n, FieldType t, FieldFlag f, uint32_t minValue, uint32_t maxValue, uint32_t defaultValue)
                : FieldBase(n, t, f), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
            // explicit select type to uint
            constexpr FieldUInt(const char* n, FieldFlag f, uint32_t minValue, uint32_t maxValue, uint32_t defaultValue)
                : FieldBase(n, FieldType::UInt, f), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
        };

        struct FieldFloat : FieldBase {
            float minValue;
            float maxValue;
            float defaultValue;
            // used to define when minValue/maxValue are not defined
            constexpr FieldFloat(const char* n, FieldFlag f, float defaultValue) : FieldBase(n, FieldType::Float, f), minValue(NAN), maxValue(NAN), defaultValue(defaultValue) {}
            // can be used when inherited and used as a subtupe
            constexpr FieldFloat(const char* n, FieldType t, FieldFlag f, float minValue, float maxValue, float defaultValue)
                : FieldBase(n, t, f), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
            // explicit select type to float
            constexpr FieldFloat(const char* n, FieldFlag f, float minValue, float maxValue, float defaultValue)
                : FieldBase(n, FieldType::Float, f), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
        };

        enum PrimitiveTypeFlags : uint8_t {
            AllowAll   = 0xFF,
            AllowNumbers = (1 << 0) | (1 << 1) | (1 << 2),
            AllowInt   = 1 << 0,
            AllowUInt  = 1 << 1,
            AllowFloat = 1 << 2,
            AllowBool = 1 << 3,
        };

        struct FieldNumber : FieldBase {
            uint8_t primitiveTypeFlags;

            //constexpr FieldNumber(const char* n, FieldFlag f) : FieldBase(n, FieldType::Number, f), primitiveTypeFlags(PrimitiveTypeFlags::AllowNumbers) {}

            constexpr FieldNumber(const char* n, FieldFlag f, uint8_t primitiveTypeFlags = PrimitiveTypeFlags::AllowNumbers) : FieldBase(n, FieldType::Number, f), primitiveTypeFlags(primitiveTypeFlags) {}

        };

        struct FieldBool : FieldBase {
            bool defaultValue;
            // can be used when inherited and used as a subtupe
            constexpr FieldBool(const char* n, FieldType t, FieldFlag f, bool defaultValue)
                : FieldBase(n, t, f), defaultValue(defaultValue) {}
            // explicit select type to bool
            constexpr FieldBool(const char* n, FieldFlag f, bool defaultValue)
                : FieldBase(n, FieldType::Bool, f), defaultValue(defaultValue) {}
        };

        struct FieldString : FieldBase {
            enum class AllowedValuesPolicy {
                Strict,
                IgnoreCase,
                Void
            };

            const char* defaultValue;  // flash string default, or more like what to present at GUI
            uint16_t minLength;
            uint16_t maxLength;

            const char* const* allowedValues; // nullptr = no restriction
            AllowedValuesPolicy allowedValuesPolicy;

            // can be used when inherited and used as a subtupe
            constexpr FieldString(const char* n, FieldType t, FieldFlag f, const char* defVal, uint16_t maxLen)
                : FieldBase(n, t, f), defaultValue(defVal), minLength(1), maxLength(maxLen), allowedValues(nullptr), allowedValuesPolicy(AllowedValuesPolicy::Void) {}
            // explicit select type to string
            constexpr FieldString(const char* n, FieldFlag f, const char* defVal, uint16_t maxLen)
                : FieldBase(n, FieldType::String, f), defaultValue(defVal), minLength(1), maxLength(maxLen), allowedValues(nullptr), allowedValuesPolicy(AllowedValuesPolicy::Void) {}

            // can be used when inherited and used as a subtupe
            constexpr FieldString(const char* n, FieldType t, FieldFlag f, const char* defVal, uint16_t minLen, uint16_t maxLen)
                : FieldBase(n, t, f), defaultValue(defVal), minLength(minLen), maxLength(maxLen), allowedValues(nullptr), allowedValuesPolicy(AllowedValuesPolicy::Void) {}
            // explicit select type to string
            constexpr FieldString(const char* n, FieldFlag f, const char* defVal, uint16_t minLen, uint16_t maxLen)
                : FieldBase(n, FieldType::String, f), defaultValue(defVal), minLength(minLen), maxLength(maxLen), allowedValues(nullptr), allowedValuesPolicy(AllowedValuesPolicy::Void) {}

            constexpr FieldString(const char* n, FieldFlag f, const char* defVal, const char* const* allowedValues, AllowedValuesPolicy allowedValuesPolicy)
                : FieldBase(n, FieldType::String, f), defaultValue(defVal), minLength(1), maxLength(0), allowedValues(allowedValues), allowedValuesPolicy(allowedValuesPolicy) {}
            
        };

        /**
         * used for ordinary JSON objects, i.e. enclosed by {}
         */
        struct FieldObject : FieldBase {
            const JsonSchema::JsonObjectScheme* subtype;
            
            constexpr FieldObject(const char* n, FieldFlag f, const JsonSchema::JsonObjectScheme* subtype)
                : FieldBase(n, FieldType::Object, f), subtype(subtype) {}
            
        };

        /** 
         * FieldArray represents a homogeneous array where every element must conform 
         * to the same JsonSchema::Device definition. This is used for structured data 
         * with a fixed schema (no type selection per element).
         */
        struct FieldArray : FieldBase {
            const JsonSchema::JsonObjectScheme* subtype;
            EmptyPolicy emptyPolicy;

            constexpr FieldArray(const char* n, FieldFlag f, const JsonSchema::JsonObjectScheme* subtype)
                : FieldBase(n, FieldType::Array, f), subtype(subtype), emptyPolicy(EmptyPolicy::Warn) {}
            constexpr FieldArray(const char* n, FieldFlag f, const JsonSchema::JsonObjectScheme* subtype, EmptyPolicy emptyPolicy)
                : FieldBase(n, FieldType::Array, f), subtype(subtype), emptyPolicy(emptyPolicy) {}
        };
        
        /** 
         * FieldArrayPrimitive represents a JSON array where each element is a native JSON type:
         * Number (int, float), Bool, or String.
         * Used for unstructured arrays like your "arr1" example.
         */
        struct FieldArrayPrimitive : FieldBase {
            uint8_t primitiveTypeFlags;
            EmptyPolicy emptyPolicy;

            // Number-only array
            constexpr FieldArrayPrimitive(const char* n, FieldFlag f, uint8_t primitiveTypeFlags = PrimitiveTypeFlags::AllowAll, EmptyPolicy emptyPolicy = EmptyPolicy::Warn)
                : FieldBase(n, FieldType::Array, f), primitiveTypeFlags(primitiveTypeFlags), emptyPolicy(emptyPolicy)
            {}
        };

        //************************************* */
        //*********  Logical Types ************ */
        //************************************* */

        struct FieldUID : FieldString {
            constexpr FieldUID(const char* n, FieldFlag f)
                : FieldString(n, FieldType::UID, f, nullptr, HAL_UID::Size) {}
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
         * AllOfGroup is a logical unit of fields where the group may be optional, 
         * and if present, all child field must exist. 
         * The group's presence can be overridden by ModeSelector rules.
         */
        struct AllOfGroup : FieldBase {
            const FieldBase* const* fields;
            constexpr AllOfGroup(const char* outputName, FieldFlag f, const FieldBase* const* fields)
                : FieldBase(outputName, FieldType::AllOfGroup, f), fields(fields) {}
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
        
        struct FieldHexBytes : FieldString {
            uint8_t byteCount;

            constexpr FieldHexBytes(const char* name,
                                FieldFlag flag,
                                const char* defaultValue,
                                uint8_t byteCount)
                : FieldString(name, FieldType::HexBytes, flag, defaultValue, byteCount*2, 0),
                byteCount(byteCount) {}
        };

    } // namespace JsonSchema

}