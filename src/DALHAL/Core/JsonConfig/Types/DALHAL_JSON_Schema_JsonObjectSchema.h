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

namespace DALHAL {

    namespace JsonSchema {

        enum class UnknownFieldPolicy {
            Ignore,
            Warn,
            Error
        };
        const char* UnknownFieldPolicyToString(UnknownFieldPolicy policy);

        struct ModeConjunctionDefine {
            const SchemaTypeBase* fieldRef;
            bool required; // true = must exist, false = must explicit NOT exist
        };

        struct ModeSelector {
            const char* name; // optional
            const ModeConjunctionDefine* conjunctions; // nullptr terminated
            constexpr ModeSelector(const char* name, const ModeConjunctionDefine* conjunctions)
                : name(name), conjunctions(conjunctions) {}
        };

        struct FieldConstraint {
            enum class Type {
                Void,
                LessThan,
                GreaterThan,
                LessThanOrEqual,
                GreaterThanOrEqual                
            };

            const SchemaTypeBase* fieldA;
            Type type;
            const SchemaTypeBase* fieldB;
            constexpr FieldConstraint(const SchemaTypeBase* fieldA, Type type, const SchemaTypeBase* fieldB):
                fieldA(fieldA),
                type(type),
                fieldB(fieldB)
                {}
        };
        const char* FieldConstraintTypeToString(FieldConstraint::Type type);

        struct JsonObjectSchema {
            const char* typeName;
            const SchemaTypeBase* const* fields;
            const ModeSelector* modes;
            const FieldConstraint* constraints;
            EmptyPolicy emptyPolicy;
            UnknownFieldPolicy unknownFieldPolicy;

            constexpr JsonObjectSchema(const char* typeName, const SchemaTypeBase* const* fields, const ModeSelector* modes, const FieldConstraint* constraints):
                typeName(typeName), fields(fields), modes(modes), constraints(constraints), emptyPolicy(EmptyPolicy::Warn), unknownFieldPolicy(UnknownFieldPolicy::Warn) {}
            
            constexpr JsonObjectSchema(const char* typeName, const SchemaTypeBase* const* fields, const ModeSelector* modes, const FieldConstraint* constraints, EmptyPolicy emptyPolicy, UnknownFieldPolicy unknownFieldPolicy):
                typeName(typeName), fields(fields), modes(modes), constraints(constraints), emptyPolicy(emptyPolicy), unknownFieldPolicy(unknownFieldPolicy) {}
        };

    }

}