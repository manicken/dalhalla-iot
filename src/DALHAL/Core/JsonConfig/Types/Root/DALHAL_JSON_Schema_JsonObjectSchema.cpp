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

#include <DALHAL/Core/Types/DALHAL_Value.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_ModeSelector.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_FieldConstraint.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_AllOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_FieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_OneOfFieldsGroup.h>

#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

      // this is only a helper/support function and do not use anyError
        // as this could be just defined as a warning, depending on strict level requirements
        bool isUnknownField2(const char* key, const SchemaTypeBase* const* fields)
        {
            for (int i = 0; fields[i] != nullptr; i++) {
                const SchemaTypeBase* f = fields[i];
                
                // to make this clearer find field functionality should been inside each group type for nicer code here
                if (f->type == FieldType::OneOfFieldsGroup) {
                    const SchemaOneOfFieldsGroup* group = static_cast<const SchemaOneOfFieldsGroup*>(f);

                    for (int g = 0; group->fields[g] != nullptr; g++) {
                        if (group->fields[g]->name == nullptr) {
                            GlobalLogger.Warn(F("OneOfGroup - group->fields[g]->name == nullptr @ key: "), key);
                            continue;
                        }
                        if (strcmp(key, group->fields[g]->name) == 0)
                            return false; // isUnknownField
                    }
                } else if (f->type == FieldType::AllOfFieldsGroup) {
                    const SchemaAllOfFieldsGroup* group = static_cast<const SchemaAllOfFieldsGroup*>(f);

                    for (int g = 0; group->fields[g] != nullptr; g++) {
                        if (group->fields[g]->name == nullptr) {
                            GlobalLogger.Warn(F("AllOfGroup - group->fields[g]->name == nullptr @ key: "), key);
                            continue;
                        }
                        if (strcmp(key, group->fields[g]->name) == 0)
                            return false; // isUnknownField
                    }
                } else if (f->type == FieldType::FieldsGroup) {
                    const SchemaFieldsGroup* group = static_cast<const SchemaFieldsGroup*>(f);
                    if (!isUnknownField2(key, group->fields)) { // recurse into the subgroup
                        return false;
                    }
                } else {
                    if (f->name == nullptr) {
                        GlobalLogger.Warn(F("f->name == nullptr @ key: "), key);
                        continue;
                    }
                    //Serial.println(key);
                    //Serial.print(FieldTypeToString(f->type));
                    //Serial.println(f->name);
                    if (strcmp(key, f->name) == 0)
                        return false; // isUnknownField
                }
            }

            return true; // isUnknownField
        }

        void JsonObjectSchema::ValidateSchema(const JsonObjectSchema* schema, const char* sourceObjTypeName, bool& anyError) {
            if (schema->fields == nullptr) {
                if (schema->typeName != nullptr) {
                    sourceObjTypeName = schema->typeName;
                } else if (sourceObjTypeName == nullptr) {
                    sourceObjTypeName = "null";
                }
                GlobalLogger.Error(F("schema error - JsonObjectSchema - schema->fields == nullptr"), sourceObjTypeName);
            }
        }

        ValidatorResult JsonObjectSchema::ValidateJson(const JsonObjectSchema* jsonObjectSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            if (jsonObjectSchema == nullptr) { return ValidatorResult::Success; } // allow any content

            if (!jsonObj.is<JsonObject>()) { // absolutely failsafe if i have somehow missed it upper in the stream
                anyError = true;

                std::string err = jsonObjectSchema->typeName;
                err += " @ ";
                err += sourceObjTypeName;

                GlobalLogger.Error(F("Expected JsonObject: "), err.c_str());
                return ValidatorResult::FieldTypeMismatch;
            }

            if (jsonObj.as<JsonObject>().size() == 0) {
                if (jsonObjectSchema->emptyPolicy == EmptyPolicy::Warn) {
                    std::string errStr = jsonObjectSchema->typeName;
                    errStr += " @ ";errStr += sourceObjTypeName;
                    GlobalLogger.Warn(F("JsonObject is empty: "), errStr.c_str());
                    
                } else if (jsonObjectSchema->emptyPolicy == EmptyPolicy::Error) {
                    anyError = true;
                    std::string errStr = jsonObjectSchema->typeName;
                    errStr += " @ ";errStr += sourceObjTypeName;
                    GlobalLogger.Error(F("JsonObject is empty: "), errStr.c_str());
                    return ValidatorResult::FieldEmpty;
                } /*else if (jsonObjectSchema->emptyPolicy == EmptyPolicy::Ignore) {
                    // simply do nothing
                }*/
                return ValidatorResult::Success;
            }
            
            // 1. Check unknown fields
            for (const JsonPair& kv : jsonObj.as<JsonObject>()) {
                const char* key = kv.key().c_str();
                if (key == nullptr) {
                    GlobalLogger.Warn(F("key == nullptr: "), jsonObjectSchema->typeName);
                    continue;
                }
                if (jsonObjectSchema->fields == nullptr) {
                    GlobalLogger.Warn(F("jsonObjectSchema->fields == nullptr: "), key);
                    continue;
                }

                if (isUnknownField2(key, jsonObjectSchema->fields)) {
                    if (jsonObjectSchema->unknownFieldPolicy == UnknownFieldPolicy::Ignore) {
                        continue;
                    } else if (jsonObjectSchema->unknownFieldPolicy == UnknownFieldPolicy::Warn) {
                        std::string errMsg = key;
                        errMsg += " @ "; errMsg += jsonObjectSchema->typeName;
                        errMsg += ' '; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                        errMsg += ' '; serializeCollapsed(jsonObj, errMsg);
                        GlobalLogger.Warn(F("Unknown config field: "), errMsg.c_str());
                    } else if (jsonObjectSchema->unknownFieldPolicy == UnknownFieldPolicy::Error) {
                        
                        std::string errMsg = key;
                        errMsg += " @ "; errMsg += jsonObjectSchema->typeName;
                        errMsg += ' '; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                        errMsg += ' '; serializeCollapsed(jsonObj, errMsg);
                        GlobalLogger.Error(F("Unknown config field: "), errMsg.c_str());
                        anyError = true;
                    }                    
                    
                    // as this should not render the json invalid
                    // anyError is not set
                }
            }
            if (jsonObjectSchema->typeName != nullptr) {
                sourceObjTypeName = jsonObjectSchema->typeName;
            } else if (sourceObjTypeName == nullptr) {
                sourceObjTypeName = "nullptr error";
            }
            // 2. Validate each field
            for (int i = 0; jsonObjectSchema->fields[i] != nullptr; ++i) {
                //const SchemaTypeBase& f = *jsonObjectSchema->fields[i];

                JsonSchema::ValidateJson(*jsonObjectSchema->fields[i], sourceObjTypeName, jsonObj, anyError);
                //const FieldTypeRegistryItem& regDefItem = GetFieldTypeRegistryItem(f->type);
                //regDefItem.define.ValidateJson(*f, sourceObjTypeName, jsonObj, anyError);
            }

            // 3. Evaluate modes if available
            if (jsonObjectSchema->modes) {
                int mode = ModeSelector::evaluate(jsonObjectSchema->modes, jsonObj);
                if (mode == -1) {
                    std::string errInfoMsg = "["; errInfoMsg += jsonObjectSchema->typeName; errInfoMsg += "] ";
                    serializeCollapsed(jsonObj, errInfoMsg);
                    GlobalLogger.Error(F("No valid configuration mode found @ "), errInfoMsg.c_str());

                    anyError = true;
                } else if (mode == -2) {
                    std::string errInfoMsg = "["; errInfoMsg += jsonObjectSchema->typeName; errInfoMsg += "] ";
                    serializeCollapsed(jsonObj, errInfoMsg);
                    GlobalLogger.Error(F("Configuration matches multiple modes @ "), errInfoMsg.c_str());
                    anyError = true;
                }
            }

            // 4. Evaluate constraints if available
            if (jsonObjectSchema->constraints) {
                FieldConstraint::evaluate(jsonObj, jsonObjectSchema->typeName, jsonObjectSchema->constraints, anyError);
            }
            return ValidatorResult::Success;
        }

        void JsonObjectSchema::SchemaToJson(const JsonObjectSchema* schema, std::string& out) {

            if (schema == nullptr) {
                out += "{}";
                return;
            }
            out += '{';

            out += "\"type\":\"object\"";
            out += ','; ToJsonString::appendString(out, "name", (schema->typeName!=nullptr)?schema->typeName:"nullptr");
            out += ','; ToJsonString::appendString(out, "unknownPolicy", UnknownFieldPolicyToString(schema->unknownFieldPolicy));
            out += ','; ToJsonString::appendString(out, "emptyPolicy", EmptyPolicyToString(schema->emptyPolicy));

            if (schema->modes != nullptr) {
                out += ','; ModeSelector::ToJson(schema->modes, out);
            }
            if (schema->constraints != nullptr) {
                out += ','; FieldConstraint::ToJson(schema->constraints, out);
            }
            out += ','; ToJsonString::appendKey(out, "fields");
            out += '[';

            for (int i = 0; schema->fields[i] != nullptr; ++i) {
                if (i > 0) out += ",";
                const SchemaTypeBase& field = *schema->fields[i];
                JsonSchema::SchemaToJson(field, out); // shortcut and safer to use, from DALHAL_JSON_Schema_TypesRegistry.h
                //const FieldTypeRegistryItem& regDefItem = GetFieldTypeRegistryItem(field.type);
                //regDefItem.define.ToJson(field, out);
            }

            out += ']';
            out += '}';
        }

        const char* JsonObjectSchema::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

    }

}