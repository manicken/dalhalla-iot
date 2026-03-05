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


#include <stdlib.h>

namespace DALHAL {

    namespace JsonSchema {

        enum class FieldType {
            UID,
            UID_Path,
            Int,
            UInt,
            Float,
            Array,
            Pin
        };

        enum class FieldFlag {
            Required,
            Optional
        };

        struct FieldBase {
            const char* name;    // flash string
            FieldType type;
            FieldFlag flag;

            constexpr FieldBase(const char* n, FieldType t, FieldFlag f)
                : name(n), type(t), flag(f) {}
        };

        struct FieldInt : FieldBase {
            int32_t minValue;
            int32_t maxValue;
            int32_t defaultValue;
        };

        struct FieldUInt : FieldBase {
            uint32_t minValue;
            uint32_t maxValue;
            uint32_t defaultValue;

             constexpr FieldUInt(const char* n, FieldType t, FieldFlag f, uint32_t minValue, uint32_t maxValue, uint32_t defaultValue)
                : FieldBase(n, t, f), minValue(minValue), maxValue(maxValue), defaultValue(defaultValue) {}
        };

        struct FieldFloat : FieldBase {
            float minValue;
            float maxValue;
            float defaultValue;
        };

        struct FieldBool : FieldBase {
            bool defaultValue;
        };

        struct FieldString : FieldBase {
            const char* defaultValue;  // flash string default
            uint16_t maxLength;

            constexpr FieldString(const char* n, FieldType t, FieldFlag f, const char* defVal, uint16_t maxLen)
                : FieldBase(n, t, f), defaultValue(defVal), maxLength(maxLen) {}
        };

        struct FieldPin : FieldBase {
            uint8_t def;
        };

        struct FieldArray : FieldBase {
            const FieldBase** allowedChildren; // nullptr terminated
            const char* arrayName;             // e.g., "items", "groups"
        };
    }
}