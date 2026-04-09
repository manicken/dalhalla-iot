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

#include <cstdint>
#include <cstring>
#include <string>

#define DALHAL_JsonSchema_FIELD_TYPE_LIST \
    X(FieldsGroup) \
    X(AllOfFieldsGroup) \
    X(OneOfFieldsGroup) \
    X(Array) \
    X(ArrayPrimitive) \
    X(Bool) \
    X(Float) \
    X(HardwarePin) \
    X(HardwarePinOrVirtualPin) \
    X(HexBytes) \
    X(Int) \
    X(Number) \
    X(Object) \
    X(RegistryArray) \
    X(StringBase) \
    X(StringSizeConstrained) \
    X(StringAnyOfByFuncConstrained) \
    X(StringAnyOfArrayConstrained) \
    X(UID) \
    X(UID_Path) \
    X(UInt)

namespace DALHAL {

    namespace JsonSchema {

        /*enum class FieldType {
            FieldsGroup,
            AllOfFieldsGroup,
            OneOfFieldsGroup,
            Array,         // homogeneous type
            ArrayPrimitive, // homogeneous type of primitives defined by allowence flags (and can mix different primitive types such as bool, uint, int and float)
            Bool,
            Float,
            HardwarePin,
            HardwarePinOrVirtualPin,
            HexBytes,      // note can be use with or without delimiters but only consistent usage is allowed so for example 2845:56:67 is not allowed, however currently the delimiter type is not fixed so it can be 28:45.56-67
            Int,
            Number, // json number
            Object,        // ordinary JSON object i.e. enclosed by {}
            RegistryArray, // polymorphic (registry-based)
            StringBase,
            StringSizeConstrained,
            StringAnyOfArrayConstrained,
            StringAnyOfByFuncConstrained,
            UID,
            UID_Path,
            UInt,
        };*/
        enum class FieldType : uint8_t {
        #define X(name) name,
            DALHAL_JsonSchema_FIELD_TYPE_LIST
        #undef X
            _Count_
        };
        const char* FieldTypeToString(FieldType type);

        enum class FieldPolicy {
            Required,
            Optional,
            OneOfGroup, // the higher group defines
            AllOfFieldsGroup, // the higher group defines
            FieldsGroup, // the individual items decides the policy as they where defined flat, group defines no policy
            ModeDefine // Mode defines
        };
        const char* FieldPolicyToString(FieldPolicy flag);

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

        enum PrimitiveTypeFlags : uint8_t {
            AllowAll   = 0xFF,
            AllowNumbers = (1 << 0) | (1 << 1) | (1 << 2),
            AllowInt   = 1 << 0,
            AllowUInt  = 1 << 1,
            AllowFloat = 1 << 2,
            AllowBool = 1 << 3,
        };

        struct SchemaTypeBase {
            const char* name;    // flash string
            FieldType type;
            FieldPolicy policy;
            FieldGuiFlags guiFlags;
        protected:
            constexpr SchemaTypeBase(const char* n, FieldType t, FieldPolicy policy)
                : name(n), type(t), policy(policy), guiFlags(Gui::None) {}

            constexpr SchemaTypeBase(const char* n, FieldType t, FieldPolicy policy, FieldGuiFlags guiFlags)
                : name(n), type(t), policy(policy), guiFlags(guiFlags) {}
        };

    }

  }
