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

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

namespace DALHAL {

    namespace JsonSchema {

        //************************************* */
        //*******  Native JSON Types ********** */
        //************************************* */

        struct FieldInt : FieldBase {
            int32_t minValue;
            int32_t maxValue;
            int32_t defaultValue;
            // used to define when minValue/maxValue are not defined
            constexpr FieldInt(const char* name, FieldPolicy policy, int32_t defaultValue) 
                : FieldBase(name, FieldType::Float, policy), minValue(-2147483648), maxValue(0), defaultValue(defaultValue) {}
            // can be used when inherited and used as a subtupe
            constexpr FieldInt(const char* name, FieldType type, FieldPolicy policy, int32_t minValue, int32_t maxValue, int32_t defaultValue)
                : FieldBase(name, type, policy), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
            // explicit select type to int
            constexpr FieldInt(const char* name, FieldPolicy policy, int32_t minValue, int32_t maxValue, int32_t defaultValue)
                : FieldBase(name, FieldType::Int, policy), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
        };

        struct FieldUInt : FieldBase {
            uint32_t minValue;
            uint32_t maxValue;
            uint32_t defaultValue;
            // used to define when minValue/maxValue are not defined
            constexpr FieldUInt(const char* name, FieldPolicy policy, uint32_t defaultValue) 
                : FieldBase(name, FieldType::Float, policy), minValue(0), maxValue(0), defaultValue(defaultValue) {}
            // can be used when inherited and used as a subtupe
            constexpr FieldUInt(const char* name, FieldType type, FieldPolicy policy, uint32_t minValue, uint32_t maxValue, uint32_t defaultValue)
                : FieldBase(name, type, policy), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
            // explicit select type to uint
            constexpr FieldUInt(const char* name, FieldPolicy policy, uint32_t minValue, uint32_t maxValue, uint32_t defaultValue)
                : FieldBase(name, FieldType::UInt, policy), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
        };

        struct FieldFloat : FieldBase {
            float minValue;
            float maxValue;
            float defaultValue;
            // used to define when minValue/maxValue are not defined
            constexpr FieldFloat(const char* name, FieldPolicy policy, float defaultValue) 
                : FieldBase(name, FieldType::Float, policy), minValue(NAN), maxValue(NAN), defaultValue(defaultValue) {}
            // can be used when inherited and used as a subtupe
            constexpr FieldFloat(const char* name, FieldType type, FieldPolicy policy, float minValue, float maxValue, float defaultValue)
                : FieldBase(name, type, policy), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
            // explicit select type to float
            constexpr FieldFloat(const char* name, FieldPolicy policy, float minValue, float maxValue, float defaultValue)
                : FieldBase(name, FieldType::Float, policy), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
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

            constexpr FieldNumber(const char* name, FieldPolicy policy, uint8_t primitiveTypeFlags = PrimitiveTypeFlags::AllowNumbers) 
                : FieldBase(name, FieldType::Number, policy), primitiveTypeFlags(primitiveTypeFlags) {}

        };

        struct FieldBool : FieldBase {
            bool defaultValue;
            // can be used when inherited and used as a subtupe
            constexpr FieldBool(const char* name, FieldType type, FieldPolicy policy, bool defaultValue)
                : FieldBase(name, type, policy), defaultValue(defaultValue) {}
            // explicit select type to bool
            constexpr FieldBool(const char* name, FieldPolicy policy, bool defaultValue)
                : FieldBase(name, FieldType::Bool, policy), defaultValue(defaultValue) {}
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
            constexpr FieldString(const char* name, FieldType type, FieldPolicy policy, const char* defVal, uint16_t maxLen)
                : FieldBase(name, type, policy), defaultValue(defVal), minLength(1), maxLength(maxLen), allowedValues(nullptr), allowedValuesPolicy(AllowedValuesPolicy::Void) {}
            // explicit select type to string
            constexpr FieldString(const char* name, FieldPolicy policy, const char* defVal, uint16_t maxLen)
                : FieldBase(name, FieldType::String, policy), defaultValue(defVal), minLength(1), maxLength(maxLen), allowedValues(nullptr), allowedValuesPolicy(AllowedValuesPolicy::Void) {}

            // can be used when inherited and used as a subtupe
            constexpr FieldString(const char* name, FieldType type, FieldPolicy policy, const char* defVal, uint16_t minLen, uint16_t maxLen)
                : FieldBase(name, type, policy), defaultValue(defVal), minLength(minLen), maxLength(maxLen), allowedValues(nullptr), allowedValuesPolicy(AllowedValuesPolicy::Void) {}
            // explicit select type to string
            constexpr FieldString(const char* name, FieldPolicy policy, const char* defVal, uint16_t minLen, uint16_t maxLen)
                : FieldBase(name, FieldType::String, policy), defaultValue(defVal), minLength(minLen), maxLength(maxLen), allowedValues(nullptr), allowedValuesPolicy(AllowedValuesPolicy::Void) {}

            constexpr FieldString(const char* name, FieldPolicy policy, const char* defVal, const char* const* allowedValues, AllowedValuesPolicy allowedValuesPolicy)
                : FieldBase(name, FieldType::String, policy), defaultValue(defVal), minLength(1), maxLength(0), allowedValues(allowedValues), allowedValuesPolicy(allowedValuesPolicy) {}
            
        };

        struct FieldStringConstraint : FieldString {
            bool (*validate)(const char*);
            std::string (*describe)();

            constexpr FieldStringConstraint(const char* name, FieldPolicy policy, bool (*validate)(const char*), std::string (*describe)()) 
            : FieldString(name, FieldType::StringConstraint, policy, nullptr, 0), validate(validate), describe(describe) {}
        };

        /**
         * used for ordinary JSON objects, i.e. enclosed by {}
         */
        struct FieldObject : FieldBase {
            const JsonSchema::JsonObjectSchema* subtype;
            
            constexpr FieldObject(const char* name, FieldPolicy policy, const JsonSchema::JsonObjectSchema* subtype)
                : FieldBase(name, FieldType::Object, policy), subtype(subtype) {}
            
        };

        /** 
         * FieldArray represents a homogeneous array where every element must conform 
         * to the same JsonSchema::Device definition. This is used for structured data 
         * with a fixed schema (no type selection per element).
         */
        struct FieldArray : FieldBase {
            const JsonSchema::JsonObjectSchema* subtype;
            EmptyPolicy emptyPolicy;

            constexpr FieldArray(const char* name, FieldPolicy policy, const JsonSchema::JsonObjectSchema* subtype)
                : FieldBase(name, FieldType::Array, policy), subtype(subtype), emptyPolicy(EmptyPolicy::Warn) {}
            constexpr FieldArray(const char* name, FieldPolicy policy, const JsonSchema::JsonObjectSchema* subtype, EmptyPolicy emptyPolicy)
                : FieldBase(name, FieldType::Array, policy), subtype(subtype), emptyPolicy(emptyPolicy) {}
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
            constexpr FieldArrayPrimitive(const char* name, FieldPolicy policy, uint8_t primitiveTypeFlags = PrimitiveTypeFlags::AllowAll, EmptyPolicy emptyPolicy = EmptyPolicy::Warn)
                : FieldBase(name, FieldType::ArrayPrimitive, policy), primitiveTypeFlags(primitiveTypeFlags), emptyPolicy(emptyPolicy)
            {}
        };

        //************************************* */
        //*********  Logical Types ************ */
        //************************************* */

        struct FieldUID : FieldString {
            constexpr FieldUID(const char* name, FieldPolicy policy)
                : FieldString(name, FieldType::UID, policy, nullptr, HAL_UID::Size) {}
        };
        
        struct FieldHardwarePin : FieldBase {
            DALHAL_GPIO_MGR_PINFUNC_TYPE mode;
            constexpr FieldHardwarePin(const char* name, FieldPolicy policy, DALHAL_GPIO_MGR_PINFUNC_TYPE mode)
                : FieldBase(name, FieldType::HardwarePin, policy), mode(mode) {} // here pin could use Int but that is not how pins are validated they instead use GPIO_manager for validity
        };

        struct FieldHardwarePinOrVirtualPIN : FieldBase {
            DALHAL_GPIO_MGR_PINFUNC_TYPE mode;
            constexpr FieldHardwarePinOrVirtualPIN(const char* name, FieldPolicy policy, DALHAL_GPIO_MGR_PINFUNC_TYPE mode)
                : FieldBase(name, FieldType::HardwarePinOrVirtualPin, policy), mode(mode) {} // here pin could use Int but that is not how pins are validated they instead use GPIO_manager for validity
        };

        /**
         * OneOfGroup is a logical unit of fields where the group may be optional, 
         * and if present, at least one child field must exist. 
         * The group's presence can be overridden by ModeSelector rules.
         */
        struct OneOfGroup : FieldBase {
            const FieldBase* const* fields;
            constexpr OneOfGroup(const char* outputName, FieldPolicy policy, const FieldBase* const* fields)
                : FieldBase(outputName, FieldType::OneOfGroup, policy), fields(fields) {}
        };

        /**
         * AllOfGroup is a logical unit of fields where the group may be optional, 
         * and if present, all child field must exist. 
         * The group's presence can be overridden by ModeSelector rules.
         */
        struct AllOfGroup : FieldBase {
            const FieldBase* const* fields;
            constexpr AllOfGroup(const char* outputName, FieldPolicy policy, const FieldBase* const* fields)
                : FieldBase(outputName, FieldType::AllOfGroup, policy), fields(fields) {}
        };
        /**
         * Group is a logical unit of fields where the policy is handled individually
         * , just as if the fields where defined standalone flat
         */
        struct FieldsGroup : FieldBase {
            const FieldBase* const* fields;
            /*size_t fieldsCount;*/
            constexpr FieldsGroup(const FieldBase* const* fields/*, size_t fieldsCount*/)
                : FieldBase(nullptr, FieldType::FieldsGroup, FieldPolicy::FieldsGroup), fields(fields)/*, fieldsCount(fieldsCount)*/ {}
        };

        /**
         * FieldRegistryArray represents a polymorphic array where each element is a device 
         * selected from a Registry::Item table. Each entry resolves its type at runtime 
         * (typically via a type field) and is validated/created using the corresponding 
         * registry definition.
         */
        struct FieldRegistryArray : FieldBase {
            const Registry::Item* subtypes;
            const char* regPath;

            constexpr FieldRegistryArray(const char* name, FieldPolicy policy, const Registry::Item* subtypes, const char* regPath)
                : FieldBase(name, FieldType::RegistryArray, policy), subtypes(subtypes), regPath(regPath) {}

        };
        
        struct FieldHexBytes : FieldString {
            uint8_t byteCount;

            constexpr FieldHexBytes(const char* name,
                                FieldPolicy policy,
                                const char* defaultValue,
                                uint8_t byteCount)
                : FieldString(name, FieldType::HexBytes, policy, defaultValue, byteCount*2, 0),
                byteCount(byteCount) {}
        };

    } // namespace JsonSchema

}