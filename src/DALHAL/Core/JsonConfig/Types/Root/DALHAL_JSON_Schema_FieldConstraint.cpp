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

#include "DALHAL_JSON_Schema_FieldConstraint.h"

#include <stdlib.h>

#include <DALHAL/Core/Types/DALHAL_Value.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Int.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Float.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        const char* FieldConstraint::TypeToString(FieldConstraint::Type type) {
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

        bool evaluateConstraints_PrevalidateFields(const JsonVariant& jsonObj, const char* sourceObjectTypeName, const FieldConstraint& fcItem) {
            bool tempAnyError = false;
            JsonSchema::ValidateJson(*fcItem.fieldA, sourceObjectTypeName, jsonObj, tempAnyError);
            JsonSchema::ValidateJson(*fcItem.fieldB, sourceObjectTypeName, jsonObj, tempAnyError);
            if (tempAnyError) {
                GlobalLogger.Warn(F("both FieldConstraint fields must be valid"));

                return false;
            }
            return true;
        }

        void FieldConstraint::evaluate(const JsonVariant& jsonObj, const char* sourceObjectTypeName, const FieldConstraint* constraints, bool& anyError) {
            if (constraints == nullptr) return;

            for (int i=0; constraints[i].type != FieldConstraint::Type::Void; ++i) {
                const FieldConstraint& fcItem = constraints[i];
                if (fcItem.fieldA->type != fcItem.fieldB->type) {
                    GlobalLogger.Warn(F("SchemaError - both FieldConstraint fields must be of the same type"), sourceObjectTypeName);

                    continue;
                }
                HALValue valA;
                HALValue valB;
                // now both are the same type so it wont matter which one we check the type against
                if (fcItem.fieldA->type == FieldType::UInt) {
                    // first validate so that the basic values are in range and that it's the correct type
                    if (evaluateConstraints_PrevalidateFields(jsonObj, sourceObjectTypeName, fcItem) == false) {
                        continue;
                    }
                    auto fA = static_cast<const SchemaUInt*>(fcItem.fieldA);
                    auto fB = static_cast<const SchemaUInt*>(fcItem.fieldB);
                    valA = jsonObj.containsKey(fA->name)?jsonObj[fA->name].as<uint32_t>():fA->defaultValue;
                    valB = jsonObj.containsKey(fB->name)?jsonObj[fB->name].as<uint32_t>():fB->defaultValue;
                } else if (fcItem.fieldA->type == FieldType::Int) {
                    // first validate so that the basic values are in range and that it's the correct type
                    if (evaluateConstraints_PrevalidateFields(jsonObj, sourceObjectTypeName, fcItem) == false) {
                        continue;
                    }
                    auto fA = static_cast<const SchemaInt*>(fcItem.fieldA);
                    auto fB = static_cast<const SchemaInt*>(fcItem.fieldB);
                    valA = jsonObj.containsKey(fA->name)?jsonObj[fA->name].as<int32_t>():fA->defaultValue;
                    valB = jsonObj.containsKey(fB->name)?jsonObj[fB->name].as<int32_t>():fB->defaultValue;

                } else if (fcItem.fieldA->type == FieldType::Float) {
                    // first validate so that the basic values are in range and that it's the correct type
                    if (evaluateConstraints_PrevalidateFields(jsonObj, sourceObjectTypeName, fcItem) == false) {
                        continue;
                    }
                    auto fA = static_cast<const SchemaFloat*>(fcItem.fieldA);
                    auto fB = static_cast<const SchemaFloat*>(fcItem.fieldB);
                    valA = jsonObj.containsKey(fA->name)?jsonObj[fA->name].as<float>():fA->defaultValue;
                    valB = jsonObj.containsKey(fB->name)?jsonObj[fB->name].as<float>():fB->defaultValue;

                }/* else if (fcItem.fieldA->type == FieldType::Bool) { // could make sense in some situations
                    // keep it unimplemented for now
                } */else {
                    GlobalLogger.Warn(F("SchemaError - FieldConstraint fieldtype unsupported: "), FieldTypeToString(fcItem.fieldA->type));

                    continue;
                }

                if (fcItem.type == FieldConstraint::Type::GreaterThan) {
                    if ((valA > valB) == false) {
                        anyError = true;
                        std::string err = fcItem.fieldA->name + std::string(" > ") + fcItem.fieldB->name;
                        GlobalLogger.Error(F("Constraint failed: "), err.c_str());
                    }
                } else if (fcItem.type == FieldConstraint::Type::GreaterThanOrEqual) {
                    if ((valA >= valB) == false) {
                        anyError = true;
                        std::string err = fcItem.fieldA->name + std::string(" >= ") + fcItem.fieldB->name;
                        GlobalLogger.Error(F("Constraint failed: "), err.c_str());
                    }
                } else if (fcItem.type == FieldConstraint::Type::LessThan) {
                    if ((valA < valB) == false) {
                        anyError = true;
                        std::string err = fcItem.fieldA->name + std::string(" < ") + fcItem.fieldB->name;
                        GlobalLogger.Error(F("Constraint failed: "), err.c_str());
                    }
                } else if (fcItem.type == FieldConstraint::Type::LessThanOrEqual) {
                    if ((valA <= valB) == false) {
                        anyError = true;
                        std::string err = fcItem.fieldA->name + std::string(" <= ") + fcItem.fieldB->name;
                        GlobalLogger.Error(F("Constraint failed: "), err.c_str());
                    }
                } else {
                    GlobalLogger.Error(F("SchemaError - Constraint type not found: "), FieldConstraint::TypeToString(fcItem.type));
                }

            }
        }

        void FieldConstraint::ToJson(const FieldConstraint* constraints, std::string& out)
        {
            ToJsonString::appendKey(out, "constraints");
            out += '[';
            for (int i=0;constraints[i].type != FieldConstraint::Type::Void; ++i) {
                if (i>0) out += ',';
                out += '{';
                ToJsonString::appendString(out, "fieldA", constraints[i].fieldA->name);
                out += ',';
                ToJsonString::appendString(out, "type", FieldConstraint::TypeToString(constraints[i].type));
                out += ',';
                ToJsonString::appendString(out, "fieldB", constraints[i].fieldB->name);
                out += '}';
            }
            out += ']';
        }

    }

}