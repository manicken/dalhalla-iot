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
#include "DALHAL_JSON_Schema_StringAnyOfByFuncConstrained.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

namespace DALHAL {

    namespace JsonSchema {

        struct ByArrayConstraints {
            enum class Policy {
                Strict,
                IgnoreCase
            };
            const char* const* allowedValues;
            ByArrayConstraints::Policy allowedValuesPolicy;
            
            constexpr ByArrayConstraints(const char* const* allowedValues, ByArrayConstraints::Policy allowedValuesPolicy)
                : allowedValues(allowedValues), allowedValuesPolicy(allowedValuesPolicy) {}
        };
      
        struct SchemaStringAnyOfArrayConstrained final : SchemaStringAnyOfByFuncConstrained {
            static constexpr FieldTypeRegistryDefine RegistryDefine {

            };

            constexpr SchemaStringAnyOfArrayConstrained(
                const char* name, 
                FieldPolicy pol, 
                const char* defVal,
                const ByArrayConstraints* byArrayConstraints
            )
                : SchemaStringAnyOfByFuncConstrained(name, pol, defVal, &Validate, &Describe, (void*)byArrayConstraints) {}

            constexpr SchemaStringAnyOfArrayConstrained(
                const char* name, 
                FieldPolicy pol,
                FieldGuiFlags guiFlags,
                const char* defVal,
                const ByArrayConstraints* byArrayConstraints
            )
                : SchemaStringAnyOfByFuncConstrained(name, pol, guiFlags, defVal, &Validate, &Describe, (void*)byArrayConstraints) {}
            
            static inline bool Validate(void* _ctx, const char* value) {
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
            static inline std::string Describe(void* _ctx) {
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
            

        };

    }

}