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

#include "DALHAL_JSON_Schema_StringAnyOfArrayConstrained.h"

#include <stdlib.h>
#include <ArduinoJson.h>

#include "DALHAL_JSON_Schema_TypeBase.h"
#include "DALHAL_JSON_Schema_StringBase.h"
#include "DALHAL_JSON_Schema_StringAnyOfByFuncConstrained.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

namespace DALHAL {

    namespace JsonSchema {
    
        constexpr FieldTypeRegistryDefine SchemaStringAnyOfArrayConstrained::RegistryDefine {
            &SchemaValidate,
            &ValidateJson,
            &SchemaToJson,
            JavaScriptValidator
        };

        void SchemaStringAnyOfArrayConstrained::SchemaValidate(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            SchemaStringAnyOfByFuncConstrained::SchemaValidate(fieldSchema, sourceObjTypeName, anyError);
        }

        ValidatorResult SchemaStringAnyOfArrayConstrained::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            return SchemaStringAnyOfByFuncConstrained::ValidateJson(fieldSchema, sourceObjTypeName, jsonObj, anyError);
        }

        void SchemaStringAnyOfArrayConstrained::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaStringAnyOfByFuncConstrained::SchemaToJson(fieldSchema, out);

            // actually this is only needed on objects that are not final but keep it here if that change in the future
            if (fieldSchema.type == FieldType::StringAnyOfArrayConstrained) { 
                out += '}'; // this is complete object
            }
        }

        // private functions
                    
        bool SchemaStringAnyOfArrayConstrained::ValidateByArray(void* _ctx, const char* value) {
            const ByArrayConstraints* ctx = static_cast<const ByArrayConstraints*>(_ctx);
        
            for (int i=0;ctx->allowedValues[i] != nullptr;++i) {
                
                if (ctx->allowedValuesPolicy == ByArrayConstraints::Policy::IgnoreCase) {
                    if ((strcasecmp(ctx->allowedValues[i], value) == 0)) {
                        return true;
                    }
                }
                else if (strcmp(ctx->allowedValues[i], value) == 0) {
                    return true;
                }
            }
            
            return false;
        }

        std::string SchemaStringAnyOfArrayConstrained::DescribeByArray(void* _ctx) {
            const ByArrayConstraints* ctx = static_cast<const ByArrayConstraints*>(_ctx);
            std::string ret;
            ret += '[';
            for (int i=0;ctx->allowedValues[i] != nullptr;++i) {
                if (i>0) ret += ',';
                ret += '"';
                ret += ctx->allowedValues[i];
                ret += '"';
            }
            ret += ']';
            return ret;
        }

    }

}