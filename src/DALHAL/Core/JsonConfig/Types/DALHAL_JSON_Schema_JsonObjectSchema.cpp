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

#include "DALHAL_JSON_Schema_JsonObjectSchema.h"

#include <stdlib.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include "DALHAL_JSON_Schema_TypeBase.h"

namespace DALHAL {

    namespace JsonSchema {

        const char* FieldConstraintTypeToString(FieldConstraint::Type type) {
            switch (type)
            {
                case FieldConstraint::Type::GreaterThan: return ">";
                case FieldConstraint::Type::GreaterThanOrEqual: return ">=";
                case FieldConstraint::Type::LessThan: return "<";
                case FieldConstraint::Type::LessThanOrEqual: return "<=";
                case FieldConstraint::Type::Void: return "Void";
                //case FieldConstraint::Type: return "";
                default: return "Unknown";
            }
        }

        const char* UnknownFieldPolicyToString(UnknownFieldPolicy policy) {
            switch (policy)
            {
                case UnknownFieldPolicy::Error: return "Error";
                case UnknownFieldPolicy::Ignore: return "Ignore";
                case UnknownFieldPolicy::Warn: return "Warn";
                //case UnknownFieldPolicy: return "";
                default: return "Unknown";
            }
        }

    }

}