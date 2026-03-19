/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

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


namespace DALHAL {

    namespace JsonSchema {
        enum class FieldType {
            AnyOfGroup,
            AllOfGroup,
            UID,
            UID_Path,
            Bool,
            Int,
            UInt,
            Float,
            String,
            HexBytes,      // note can be use with or without delimiters but only consistent usage is allowed so for example 2845:56:67 is not allowed, however currently the delimiter type is not fixed so it can be 28:45.56-67
            Array,         // homogeneous type
            RegistryArray, // polymorphic (registry-based)
            Object,        // ordinary JSON object i.e. enclosed by {}
            HardwarePin,
            HardwarePinOrVirtualPin
        };
        const char* FieldTypeToString(FieldType type);

        enum class FieldFlag {
            Required,
            Optional,
            AnyOfGroup, // the higher group defines
            AllOfGroup, // the higher group defines
            ModeDefine // Mode defines
        };
        const char* FieldFlagToString(FieldFlag flag);

        struct FieldBase {
            const char* name;    // flash string
            FieldType type;
            FieldFlag flag;

            constexpr FieldBase(const char* n, FieldType t, FieldFlag f)
                : name(n), type(t), flag(f) {}
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

        struct JsonObjectScheme {
            const char* typeName;
            const FieldBase* const* fields;
            const ModeSelector* modes;
            const FieldConstraint* constraints;
        };
    }

  }
