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

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

namespace DALHAL {

    namespace JsonSchema {

        /**
         * FieldsGroup is a logical unit of fields where the policy is handled individually
         * , just as if the fields where defined standalone flat
         * this is also used as the base structure for AllOfFieldsGroup and OneOfFieldsGroup
         */
        struct SchemaFieldsGroup : SchemaTypeBase {
            
            static const FieldTypeRegistryDefine RegistryDefine;
            static void ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError);
            static ValidatorResult ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError);
            static void SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out);
            static void BuildFieldsArray(const SchemaFieldsGroup& group, std::string& out);
            /** this should only be used on final object */
            static void CheckAndAddAsInline(const SchemaTypeBase& fieldSchema, std::string& out);
            static const char* GetJavaScriptValidator();
            
            const SchemaTypeBase* const* fields;          
            /*size_t fieldsCount;*/
            
        protected: // used when inherited
            constexpr SchemaFieldsGroup(const char* n, FieldType t, FieldPolicy policy, const SchemaTypeBase* const* fields/*, size_t fieldsCount*/)
                : SchemaTypeBase(n, t, policy), fields(fields)/*, fieldsCount(fieldsCount)*/ {}

            constexpr SchemaFieldsGroup(const char* n, FieldType t, FieldPolicy policy, const SchemaTypeBase* const* fields, FieldGuiFlagsType guiFlags/*, size_t fieldsCount*/)
                : SchemaTypeBase(n, t, policy, guiFlags), fields(fields)/*, fieldsCount(fieldsCount)*/ {}

        public:
            constexpr SchemaFieldsGroup(const char* n, const SchemaTypeBase* const* fields/*, size_t fieldsCount*/)
                : SchemaTypeBase(n, FieldType::FieldsGroup, FieldPolicy::FieldsGroup), fields(fields)/*, fieldsCount(fieldsCount)*/ {}

            constexpr SchemaFieldsGroup(const char* n, const SchemaTypeBase* const* fields, FieldGuiFlagsType guiFlags/*, size_t fieldsCount*/)
                : SchemaTypeBase(n, FieldType::FieldsGroup, FieldPolicy::FieldsGroup, guiFlags), fields(fields)/*, fieldsCount(fieldsCount)*/ {}
        };

    } // namespace JsonSchema

}