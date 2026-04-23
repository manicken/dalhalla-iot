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
#include <ArduinoJson.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

namespace DALHAL {

    namespace JsonSchema {

        struct SchemaString : SchemaTypeBase {
            
            static const FieldTypeRegistryDefine RegistryDefine;
            static void ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError);
            static ValidatorResult ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError);
            static HALValue GetValue(const SchemaTypeBase& fieldSchema, const JsonVariant& jsonObj);
            static void SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out);
            static const char* GetJavaScriptValidator();
            

            const char* defaultValue;  // flash string default, or more like what to present at GUI

        protected:
            constexpr SchemaString(const char* n, FieldType t, FieldPolicy pol, const char* defVal) 
                : SchemaTypeBase(n, t, pol), defaultValue(defVal) {}

            constexpr SchemaString(const char* n, FieldType t, FieldPolicy pol, FieldGuiFlagsType guiFlags, const char* defVal) 
                : SchemaTypeBase(n, t, pol, guiFlags), defaultValue(defVal) {}
            
            constexpr SchemaString(const char* n, FieldType t, FieldPolicy pol) 
                : SchemaTypeBase(n, t, pol), defaultValue("") {}

            constexpr SchemaString(const char* n, FieldType t, FieldPolicy pol, FieldGuiFlagsType guiFlags) 
                : SchemaTypeBase(n, t, pol, guiFlags), defaultValue("") {}
            
        public:
            constexpr SchemaString(const char* n, FieldPolicy pol, const char* defVal) 
                : SchemaTypeBase(n, FieldType::String, pol), defaultValue(defVal) {}

            constexpr SchemaString(const char* n, FieldPolicy pol, FieldGuiFlagsType guiFlags, const char* defVal) 
                : SchemaTypeBase(n, FieldType::String, pol, guiFlags), defaultValue(defVal) {}

            constexpr SchemaString(const char* n, FieldPolicy pol) 
                : SchemaTypeBase(n, FieldType::String, pol), defaultValue("") {}

            constexpr SchemaString(const char* n, FieldPolicy pol, FieldGuiFlagsType guiFlags) 
                : SchemaTypeBase(n, FieldType::String, pol, guiFlags), defaultValue("") {}
        };

    }

}