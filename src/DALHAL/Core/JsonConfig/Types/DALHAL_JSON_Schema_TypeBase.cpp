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

#include "DALHAL_JSON_Schema_TypeBase.h"

namespace DALHAL {

    namespace JsonSchema {

        /*const char* FieldTypeToString(FieldType type) {
            switch (type)
            {
                case FieldType::FieldsGroup: return  "FieldsGroup";
                case FieldType::AllOfFieldsGroup: return "AllOfFieldsGroup";
                case FieldType::OneOfFieldsGroup: return "OneOfFieldsGroup";
                case FieldType::Array: return "Array";
                case FieldType::ArrayPrimitive: return "ArrayPrimitive";
                case FieldType::Bool: return "Bool";
                case FieldType::Float: return "Float";
                case FieldType::HardwarePin: return "HardwarePin";
                case FieldType::HardwarePinOrVirtualPin: return "HardwarePinOrVirtualPin";
                case FieldType::HexBytes: return "HexBytes";
                case FieldType::Int: return "Int";
                case FieldType::Number: return "Number";
                case FieldType::Object: return "Object";
                case FieldType::RegistryArray: return "RegistryArray";
                case FieldType::StringBase: return "StringBase";
                case FieldType::StringAnyOfByFuncConstrained: return "StringAnyOfByFuncConstrained";
                case FieldType::StringAnyOfArrayConstrained: return "StringAnyOfArrayConstrained";
                case FieldType::StringSizeConstrained: return "StringSizeConstrained";
                case FieldType::StringUID: return "UID";
                case FieldType::StringUID_Path: return "UID_Path";
                case FieldType::UInt: return "UInt";
                
                //case FieldType: return "";
                default: return "Unknown";
            }
        }*/
        const char* FieldPolicyToString(FieldPolicy policy) {
            switch (policy)
            {
                case FieldPolicy::FieldsGroup: return "FieldsGroup";
                case FieldPolicy::AllOfFieldsGroup: return "AllOfGroup";
                case FieldPolicy::OneOfGroup: return "OneOfGroup";
                case FieldPolicy::ModeDefine: return "ModeDefine";
                case FieldPolicy::Optional: return "Optional";
                case FieldPolicy::Required: return "Required";
                //case FieldPolicy: return "";
                default: return "Unknown";
            }
        }
        
        
        const char* EmptyPolicyToString(EmptyPolicy policy) {
            switch (policy)
            {
                case EmptyPolicy::Error: return "Error";
                case EmptyPolicy::Ignore: return "Ignore";
                case EmptyPolicy::Warn: return "Warn";
                //case CanBeEmptyPolicy: return "";
                default: return "Unknown";
            }
        }
    }

}