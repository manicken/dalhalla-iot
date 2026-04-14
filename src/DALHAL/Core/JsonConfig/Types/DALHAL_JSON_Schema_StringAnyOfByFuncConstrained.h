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

#include "DALHAL_JSON_Schema_TypeBase.h"
#include "DALHAL_JSON_Schema_StringBase.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <cassert>

namespace DALHAL {

    namespace JsonSchema {
        
        struct SchemaStringAnyOfByFuncConstrained : SchemaStringBase {

            static const FieldTypeRegistryDefine RegistryDefine;
            static void SchemaValidate(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError);
            static ValidatorResult ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError);
            static void SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out);
            static const char* JavaScriptValidator;

            
            using ValidateFunc  = bool(*)(void* ctx, const char* value);
            using DescribeFunc  = std::string(*)(void* ctx);

            ValidateFunc validate;
            DescribeFunc describe;
            void* ctx;
	
            constexpr SchemaStringAnyOfByFuncConstrained(
                const char* n, FieldPolicy pol, const char* defVal, const ValidateFunc validate, const DescribeFunc describe, void* ctx
            )
                : SchemaStringBase(n, FieldType::StringAnyOfByFuncConstrained, pol, defVal), validate(validate), describe(describe), ctx(ctx) { }

            constexpr SchemaStringAnyOfByFuncConstrained(
                const char* n, FieldPolicy pol, FieldGuiFlags guiFlags, const char* defVal, ValidateFunc validate, DescribeFunc describe, void* ctx
            )
                : SchemaStringBase(n, FieldType::StringAnyOfByFuncConstrained, pol, guiFlags, defVal), validate(validate), describe(describe), ctx(ctx) { }
            
        };

    }

}