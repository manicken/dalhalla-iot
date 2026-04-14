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

#include <DALHAL/Support/DALHAL_Logger.h>

#include "DALHAL_JSON_Schema_TypeBase.h"
#include "DALHAL_JSON_Schema_StringBase.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

namespace DALHAL {

    namespace JsonSchema {

        struct SchemaStringSizeConstrained : SchemaStringBase {

            static const FieldTypeRegistryDefine RegistryDefine;
            static void SchemaValidate(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError);
            static ValidatorResult ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError);
            static void SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out);
            static const char* JavaScriptValidator;
            

            uint16_t minLength;
            uint16_t maxLength;
        protected:
            // defining type, maxLength
            constexpr SchemaStringSizeConstrained(const char* n, FieldType t, FieldPolicy pol, const char* defVal, uint16_t maxLength) 
                : SchemaStringBase(n, t, pol, defVal), minLength(1), maxLength(maxLength) {}

            // defining type, guiFlags, maxLength
            constexpr SchemaStringSizeConstrained(const char* n, FieldType t, FieldPolicy pol, FieldGuiFlags guiFlags, const char* defVal, uint16_t maxLength) 
                : SchemaStringBase(n, t, pol, guiFlags, defVal), minLength(1), maxLength(maxLength) {}

            // defining type, minLength, maxLength
            constexpr SchemaStringSizeConstrained(const char* n, FieldType t, FieldPolicy pol, const char* defVal, uint16_t minLength, uint16_t maxLength) 
                : SchemaStringBase(n, t, pol, defVal), minLength(minLength), maxLength(maxLength) {}

            // defining type, guiFlags, minLength, maxLength
            constexpr SchemaStringSizeConstrained(const char* n, FieldType t, FieldPolicy pol, FieldGuiFlags guiFlags, const char* defVal, uint16_t minLength, uint16_t maxLength) 
                : SchemaStringBase(n, t, pol, guiFlags, defVal), minLength(minLength), maxLength(maxLength) {}

        public:
            // defining maxLength
            constexpr SchemaStringSizeConstrained(const char* n, FieldPolicy pol, const char* defVal, uint16_t maxLength) 
                : SchemaStringBase(n, FieldType::StringSizeConstrained, pol, defVal), minLength(1), maxLength(maxLength) {}

            // defining guiFlags, maxLength
            constexpr SchemaStringSizeConstrained(const char* n, FieldPolicy pol, FieldGuiFlags guiFlags, const char* defVal, uint16_t maxLength) 
                : SchemaStringBase(n, FieldType::StringSizeConstrained, pol, guiFlags, defVal), minLength(1), maxLength(maxLength) {}

            // defining minLength, maxLength
            constexpr SchemaStringSizeConstrained(const char* n, FieldPolicy pol, const char* defVal, uint16_t minLength, uint16_t maxLength) 
                : SchemaStringBase(n, FieldType::StringSizeConstrained, pol, defVal), minLength(minLength), maxLength(maxLength) {}

            // defining guiFlags, minLength, maxLength
            constexpr SchemaStringSizeConstrained(const char* n, FieldPolicy pol, FieldGuiFlags guiFlags, const char* defVal, uint16_t minLength, uint16_t maxLength) 
                : SchemaStringBase(n, FieldType::StringSizeConstrained, pol, guiFlags, defVal), minLength(minLength), maxLength(maxLength) {}

        };

    }

}