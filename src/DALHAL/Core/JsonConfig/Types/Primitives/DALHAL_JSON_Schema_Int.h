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

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>

#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_NumericBase.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

namespace DALHAL {

    namespace JsonSchema {

        struct SchemaInt : SchemaNumericBase {
            
            static const FieldTypeRegistryDefine RegistryDefine;
        protected:
            static void ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError);
            static ValidatorResult ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError);
            static HALValue GetValue(const SchemaTypeBase& fieldSchema, const JsonVariant& jsonObj);
            static void SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out);
            static const char* GetJavaScriptValidator();
        public:
            int ExtractFrom(const JsonVariant& jsonObj) const;
            
        public:
            int32_t minValue;
            int32_t maxValue;
            int32_t defaultValue;

        protected:
            constexpr SchemaInt(const char* name, FieldType type, FieldPolicy policy, int32_t minValue, int32_t maxValue, int32_t defaultValue, float conversionFactor, size_t structOffset)
                : SchemaNumericBase(name, type, policy, conversionFactor, structOffset), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
        
        public:
            constexpr SchemaInt(const char* name, FieldPolicy policy, int32_t defaultValue) 
                : SchemaNumericBase(name, FieldType::Int, policy, 1.0f), minValue(-2147483648), maxValue(0), defaultValue(defaultValue) {}
            constexpr SchemaInt(const char* name, FieldPolicy policy, int32_t minValue, int32_t maxValue, int32_t defaultValue)
                : SchemaNumericBase(name, FieldType::Int, policy, 1.0f), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}

            constexpr SchemaInt(const char* name, FieldPolicy policy, int32_t defaultValue, size_t structOffset) 
                : SchemaNumericBase(name, FieldType::Int, policy, 1.0f, structOffset), minValue(-2147483648), maxValue(0), defaultValue(defaultValue) {}
            constexpr SchemaInt(const char* name, FieldPolicy policy, int32_t minValue, int32_t maxValue, int32_t defaultValue, size_t structOffset)
                : SchemaNumericBase(name, FieldType::Int, policy, 1.0f, structOffset), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
            
            // using conversionFactor
            constexpr SchemaInt(const char* name, FieldPolicy policy, int32_t defaultValue, float conversionFactor) 
                : SchemaNumericBase(name, FieldType::Int, policy, conversionFactor), minValue(-2147483648), maxValue(0), defaultValue(defaultValue) {}
            constexpr SchemaInt(const char* name, FieldPolicy policy, int32_t minValue, int32_t maxValue, int32_t defaultValue, float conversionFactor)
                : SchemaNumericBase(name, FieldType::Int, policy, conversionFactor), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}

            constexpr SchemaInt(const char* name, FieldPolicy policy, int32_t defaultValue, float conversionFactor, size_t structOffset) 
                : SchemaNumericBase(name, FieldType::Int, policy, conversionFactor, structOffset), minValue(-2147483648), maxValue(0), defaultValue(defaultValue) {}
            constexpr SchemaInt(const char* name, FieldPolicy policy, int32_t minValue, int32_t maxValue, int32_t defaultValue, float conversionFactor, size_t structOffset)
                : SchemaNumericBase(name, FieldType::Int, policy, conversionFactor, structOffset), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
        
        };

    }

}