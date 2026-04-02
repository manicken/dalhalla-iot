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

namespace DALHAL {

    namespace JsonSchema {
        enum class FieldType {
            AllOfGroup,
            OneOfGroup,
            Array,         // homogeneous type
            ArrayPrimitive, // homogeneous type of primitives defined by allowence flags (and can mix different primitive types such as bool, uint, int and float)
            Bool,
            Float,
            FieldsGroup,
            HardwarePin,
            HardwarePinOrVirtualPin,
            HexBytes,      // note can be use with or without delimiters but only consistent usage is allowed so for example 2845:56:67 is not allowed, however currently the delimiter type is not fixed so it can be 28:45.56-67
            Int,
            Number, /** json number */
            Object,        // ordinary JSON object i.e. enclosed by {}
            RegistryArray, // polymorphic (registry-based)
            StringBase,
            StringSizeConstrained,
            StringAnyOfArrayConstrained,
            StringAnyOfByFuncConstrained,
            UID,
            UID_Path,
            UInt,
        };
        const char* FieldTypeToString(FieldType type);

        enum class FieldPolicy {
            Required,
            Optional,
            OneOfGroup, // the higher group defines
            AllOfGroup, // the higher group defines
            FieldsGroup, // the individual items decides the policy as they where defined flat, group defines no policy
            ModeDefine // Mode defines
        };
        const char* FieldPolicyToString(FieldPolicy flag);

        enum class UnknownFieldPolicy {
            Ignore,
            Warn,
            Error
        };
        const char* UnknownFieldPolicyToString(UnknownFieldPolicy policy);

        enum class EmptyPolicy {
            Ignore,
            Warn,
            Error
        };
        const char* EmptyPolicyToString(EmptyPolicy policy);
        using FieldGuiFlags = uint8_t;

        namespace Gui {
            constexpr FieldGuiFlags None                   = 0;
            constexpr FieldGuiFlags UseInline              = 1 << 0;
            constexpr FieldGuiFlags RenderAllAllowedValues = 1 << 1;
            constexpr FieldGuiFlags DisableByDefault       = 1 << 2;
            constexpr FieldGuiFlags ReadOnly               = 1 << 3;
            constexpr FieldGuiFlags HideLabel              = 1 << 4;

            constexpr bool hasFlag(FieldGuiFlags flags, FieldGuiFlags f) {
                return (flags & f) != 0;
            }
        }

        struct FieldBase {
            const char* name;    // flash string
            FieldType type;
            FieldPolicy policy;
            FieldGuiFlags guiFlags;

            constexpr FieldBase(const char* n, FieldType t, FieldPolicy policy)
                : name(n), type(t), policy(policy), guiFlags(Gui::None) {}

            constexpr FieldBase(const char* n, FieldType t, FieldPolicy policy, FieldGuiFlags guiFlags)
                : name(n), type(t), policy(policy), guiFlags(guiFlags) {}
        };

        struct ModeConjunctionDefine {
            const FieldBase* fieldRef;
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

            const FieldBase* fieldA;
            Type type;
            const FieldBase* fieldB;
            constexpr FieldConstraint(const FieldBase* fieldA, Type type, const FieldBase* fieldB):
                fieldA(fieldA),
                type(type),
                fieldB(fieldB)
                {}
        };
        const char* FieldConstraintTypeToString(FieldConstraint::Type type);

        struct JsonObjectSchema {
            const char* typeName;
            const FieldBase* const* fields;
            const ModeSelector* modes;
            const FieldConstraint* constraints;
            EmptyPolicy emptyPolicy;
            UnknownFieldPolicy unknownFieldPolicy;

            constexpr JsonObjectSchema(const char* typeName, const FieldBase* const* fields, const ModeSelector* modes, const FieldConstraint* constraints):
                typeName(typeName), fields(fields), modes(modes), constraints(constraints), emptyPolicy(EmptyPolicy::Warn), unknownFieldPolicy(UnknownFieldPolicy::Warn) {}
            
            constexpr JsonObjectSchema(const char* typeName, const FieldBase* const* fields, const ModeSelector* modes, const FieldConstraint* constraints, EmptyPolicy emptyPolicy, UnknownFieldPolicy unknownFieldPolicy):
                typeName(typeName), fields(fields), modes(modes), constraints(constraints), emptyPolicy(emptyPolicy), unknownFieldPolicy(unknownFieldPolicy) {}
        };
    }

  }
