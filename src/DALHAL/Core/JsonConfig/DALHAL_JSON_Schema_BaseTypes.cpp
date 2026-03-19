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

#include "DALHAL_JSON_Schema_BaseTypes.h"

namespace DALHAL {

    namespace JsonSchema {
        const char* FieldTypeToString(FieldType type) {
            switch (type)
            {
                case FieldType::AllOfGroup: return "AllOfGroup";
                case FieldType::AnyOfGroup: return "AnyOfGroup";
                case FieldType::Array: return "Array";
                case FieldType::Bool: return "Bool";
                case FieldType::Float: return "Float";
                case FieldType::HardwarePin: return "HardwarePin";
                case FieldType::HardwarePinOrVirtualPin: return "HardwarePinOrVirtualPin";
                case FieldType::HexBytes: return "HexBytes";
                case FieldType::Int: return "Int";
                case FieldType::Object: return "Object";
                case FieldType::RegistryArray: return "RegistryArray";
                case FieldType::String: return "String";
                case FieldType::UID: return "UID";
                case FieldType::UID_Path: return "UID_Path";
                case FieldType::UInt: return "UInt";
                //case FieldType: return "";
                default: return "Unknown";
            }
        }
        const char* FieldFlagToString(FieldFlag flag) {
            switch (flag)
            {
                case FieldFlag::AllOfGroup: return "AllOfGroup";
                case FieldFlag::AnyOfGroup: return "AnyOfGroup";
                case FieldFlag::ModeDefine: return "ModeDefine";
                case FieldFlag::Optional: return "Optional";
                case FieldFlag::Required: return "Required";
                //case FieldFlag: return "";
                default: return "Unknown";
            }
        }
        const char* FieldConstraintTypeToString(FieldConstraint::Type type) {
            switch (type)
            {
                case FieldConstraint::Type::GreaterThan: return "GreaterThan";
                case FieldConstraint::Type::GreaterThanOrEqual: return "GreaterThanOrEqual";
                case FieldConstraint::Type::LessThan: return "LessThan";
                case FieldConstraint::Type::LessThanOrEqual: return "LessThanOrEqual";
                case FieldConstraint::Type::Void: return "Void";
                //case FieldConstraint::Type: return "";
                default: return "Unknown";
            }
        }
    }

}