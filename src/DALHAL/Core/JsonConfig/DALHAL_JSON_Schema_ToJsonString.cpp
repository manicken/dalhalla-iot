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

#include "DALHAL_JSON_Schema_ToJsonString.h"
#include "Types/DALHAL_JSON_Schema_BaseTypes.h"
#include "Types/DALHAL_JSON_Schema_Types.h"

#include <string>
#include <vector>
#include <cstring>

namespace DALHAL {
    
    namespace JsonSchema {
        static void appendQuoted(std::string& out, const char* str) {
            out += '"';
            if (str) out += str;
            out += '"';
        }

        static void appendKey(std::string& out, const char* key) {
            appendQuoted(out, key);
            out += ':';
        }

        static void appendBool(std::string& out, bool v) {
            out += (v ? "true" : "false");
        }

        struct RegSchema {
            const char* regPath;   // unique identifier for the registry
            std::string schema;    // the JSON schema generated for this registry
        };

        // Vector to track generated registries
        std::vector<RegSchema> generatedRegistries;

        // Helper to check if a registry has already been generated
        bool isRegistryGenerated(const char* regPath) {
            for (auto &r : generatedRegistries) {
                if (strcmp(r.regPath, regPath) == 0) return true;
            }
            return false;
        }

        void buildJsonSchemas(const Registry::Item* reg, std::string &out) {
            bool first = true;
            for (int i=0;reg[i].typeName != nullptr; i++) {
                if (first == false) {
                    out += ',';
                } else { first = false; }
                appendKey(out, reg[i].typeName);
                buildJsonSchema(reg[i].def->jsonSchema, out);
            }
        }

        // Adds a new registry schema to the vector
        void addRegistrySchemaAndBuild(const Registry::Item* reg, const char* regPath) {
            auto idx = generatedRegistries.size();
            generatedRegistries.push_back({ regPath, {} });
            std::string out;
            out.reserve(8192);
            buildJsonSchemas(reg, out);
            generatedRegistries[idx].schema = out;
        }

        void buildCompleteJsonSchemasStartingFrom(const Registry::Item* reg, std::string &out) {
            generatedRegistries.clear();
            generatedRegistries.reserve(32);
            addRegistrySchemaAndBuild(reg, "ROOT");
            out = '{';
            for (int i=0;i<generatedRegistries.size();++i) {
                if (i > 0) { out += ','; }
                if (generatedRegistries[i].regPath == nullptr) {
                    out += "null" + std::to_string(i) + ":null";
                } else {
                    appendKey(out, generatedRegistries[i].regPath);
                    out += '{';
                    out += generatedRegistries[i].schema;
                    out += '}';
                }
            }
            out += '}';
            // dont forget to clean when done
            generatedRegistries.clear();
        }

        void appendModes(std::string& out, const ModeSelector* modes)
        {
            if (!modes) return;
            out += ',';
            appendKey(out, "modes");
            out += '[';
            bool firstMode = true;
            for (size_t i = 0; modes[i].name; ++i) {
                const auto& mode = modes[i];
                if (!firstMode) { out += ','; }
                else { firstMode = false; }
                out += '{';
                // mode name
                appendKey(out, "name");
                appendQuoted(out, mode.name ? mode.name : "null");
                // conjunctions
                out += ',';
                appendKey(out, "conjunctions");
                const auto* conj = mode.conjunctions;
                if (conj == nullptr) {
                    out += "null"; // a empty array mean something different
                    out += '}';
                    continue;
                }
                out += '[';
                bool firstConj = true;
                for (size_t j = 0; conj[j].fieldRef; ++j) {
                    const auto& c = conj[j];
                    // skip invalid entries safely
                    if (!c.fieldRef || !c.fieldRef->name) continue;
                    if (!firstConj) { out += ','; }
                    else { firstConj = false; }
                    out += '{';
                    appendKey(out, "name");
                    appendQuoted(out, c.fieldRef->name);
                    out += ',';
                    appendKey(out, "required");
                    out += c.required ? "true" : "false";
                    out += '}';
                }
                out += ']';
                out += '}';
            }
            out += ']';
        }

        void appendFieldConstraints(std::string& out, const FieldConstraint* constraints)
        {
            if (!constraints) return;
            out += ',';
            appendKey(out, "constraints");
            out += '[';
            for (int i=0;constraints[i].type != FieldConstraint::Type::Void; ++i) {
                if (i>0) out += ',';
                out += '{';
                appendKey(out, "fieldA"); appendQuoted(out, constraints[i].fieldA->name);
                out += ',';
                appendKey(out, "type"); appendQuoted(out, FieldConstraintTypeToString(constraints[i].type));
                out += ',';
                appendKey(out, "fieldB"); appendQuoted(out, constraints[i].fieldB->name);
                out += '}';
            }
            out += ']';
        }

        void buildJsonSchema(const JsonObjectSchema* schema, std::string& out)
        {
            if (!schema) {
                out += "{}";
                return;
            }

            out += '{';

            out += "\"type\":\"object\",";
            out += "\"name\":\"";
            out += schema->typeName;
            out += "\",";

            out += "\"unknownPolicy\":\"";
            out += UnknownFieldPolicyToString(schema->unknownFieldPolicy);
            out += "\",";
            out += "\"emptyPolicy\":\"";
            out += EmptyPolicyToString(schema->emptyPolicy);
            out += '"';
            appendModes(out, schema->modes);
            appendFieldConstraints(out, schema->constraints);
            out += ',';
            appendKey(out, "fields");
            out += '[';

            for (int i = 0; schema->fields[i] != nullptr; ++i) {
                if (i > 0) out += ",";
                buildField(schema->fields[i], out);
            }

            out += ']';
            out += '}';
        }

        void buildField(const FieldBase* f, std::string& out)
        {
            if (!f) return;

            switch (f->type)
            {
                case FieldType::FieldsGroup:
                {
                    // 🔥 flatten
                    const FieldsGroup* group = static_cast<const FieldsGroup*>(f);
                    for (int i = 0; group->fields[i] != nullptr; ++i) {
                        buildField(group->fields[i], out);
                        if (group->fields[i + 1] != nullptr)
                            out += ",";
                    }
                    return;
                }

                case FieldType::OneOfGroup:
                {
                    buildOneOfGroup(static_cast<const OneOfGroup*>(f), out);
                    return;
                }

                case FieldType::AllOfGroup:
                {
                    buildAllOfGroup(static_cast<const AllOfGroup*>(f), out);
                    return;
                }

                default:
                    buildPrimitiveField(f, out);
                    return;
            }
        }

        void buildPrimitiveField(const FieldBase* f, std::string& out)
        {
            out += "{";

            // name
            out += "\"name\":\"";
            out += f->name ? f->name : "null";
            out += "\",";

            // type
            out += "\"type\":\"";
            out += FieldTypeToString(f->type);
            out += "\",";

            // required
            out += "\"required\":";
            out += (f->policy == FieldPolicy::Required) ? "true" : "false";

            // ---- type-specific extras ----

            switch (f->type)
            {
                case FieldType::Int:
                {
                    auto fi = static_cast<const FieldInt*>(f);
                    out += ",\"min\":";
                    out += std::to_string(fi->minValue);
                    out += ",\"max\":";
                    out += std::to_string(fi->maxValue);
                    out += ",\"default\":";
                    out += std::to_string(fi->defaultValue);
                    break;
                }

                case FieldType::UInt:
                {
                    auto fu = static_cast<const FieldUInt*>(f);
                    out += ",\"min\":";
                    out += std::to_string(fu->minValue);
                    out += ",\"max\":";
                    out += std::to_string(fu->maxValue);
                    out += ",\"default\":";
                    out += std::to_string(fu->defaultValue);
                    break;
                }

                case FieldType::Float:
                {
                    auto ff = static_cast<const FieldFloat*>(f);
                    out += ",\"min\":";
                    if (isnanf(ff->minValue)) {
                        out += "null";
                    } else {
                        out += std::to_string(ff->minValue);
                    }
                    out += ",\"max\":";
                    if (isnanf(ff->maxValue)) {
                        out += "null";
                    } else {
                        out += std::to_string(ff->maxValue);
                    }
                    out += ",\"default\":";
                    out += std::to_string(ff->defaultValue);
                    break;
                }
                case FieldType::UID:
                case FieldType::UID_Path:
                case FieldType::StringBase: {
                    auto fs = static_cast<const FieldStringBase*>(f);
                    out += ",\"default\":\"";
                    if (fs->defaultValue) out += fs->defaultValue;
                    out += '"';
                    break;
                }
                case FieldType::StringAnyOfByFuncConstrained:
                case FieldType::StringAnyOfArrayConstrained: {
                    auto fs = static_cast<const FieldStringAnyOfByFuncConstrained*>(f);

                    out += ",\"allowedValues\":";
                    if (fs->describe) {
                        out += fs->describe(fs->ctx); // this should return a json string array encloused by []
                    } else {
                        out += "null";
                    }

                    out += ",\"default\":\"";
                    if (fs->defaultValue) out += fs->defaultValue;
                    out += '"';
                    break;
                }
                
                case FieldType::StringSizeConstrained:
                {
                    auto fs = static_cast<const FieldStringSizeConstrained*>(f);

                    out += ",\"minLength\":";
                    out += std::to_string(fs->minLength);

                    out += ",\"maxLength\":";
                    out += std::to_string(fs->maxLength);
                    out += ",\"default\":\"";
                    if (fs->defaultValue) out += fs->defaultValue;
                    out += '"';
                    
                    break;
                }

                case FieldType::Array:
                {
                    auto fa = static_cast<const FieldArray*>(f);

                    out += ",\"emptyPolicy\":\"";
                    out += EmptyPolicyToString(fa->emptyPolicy);
                    out += "\"";

                    if (fa->subtype) {
                        out += ",\"subtype\":";
                        buildJsonSchema(fa->subtype, out);
                    }
                    break;
                }

                case FieldType::Object:
                {
                    auto fo = static_cast<const FieldObject*>(f);

                    out += ",\"object\":true";

                    if (fo->subtype) {
                        out += ",\"subtype\":";
                        buildJsonSchema(fo->subtype, out);
                    }
                    break;
                }

                case FieldType::RegistryArray: {
                    auto fo = static_cast<const FieldRegistryArray*>(f);
                    out += ',';
                    appendKey(out, "regPath");
                    appendQuoted(out, fo->regPath);

                    if (isRegistryGenerated(fo->regPath) == false) {
                        addRegistrySchemaAndBuild(fo->subtypes, fo->regPath);
                    }
                    
                    break;
                }
                
                case FieldType::HexBytes: {
                    auto fhb = static_cast<const FieldHexBytes*>(f);
                    out += ',';
                    appendKey(out, "byteCount");
                    out += std::to_string(fhb->byteCount);
                    out += ",\"default\":\"";
                    if (fhb->defaultValue) out += fhb->defaultValue;
                    out += '"';
                    
                }

                default:
                    break;
            }

            out += "}";
        }

        void buildOneOfGroup(const OneOfGroup* group, std::string& out)
        {
            out += "{";
            
            out += "\"name\":\"";
            out += group->name ? group->name : "AnyOf";
            out += "\",";

            out += "\"type\":\"OneOfGroup\",";

            out += "\"required\":";
            out += (group->policy == FieldPolicy::Required) ? "true" : "false";

            out += ",\"fields\":[";

            for (int i = 0; group->fields[i] != nullptr; ++i) {
                if (i > 0) out += ",";
                buildField(group->fields[i], out);
            }

            out += "]}";
        }

        void buildAllOfGroup(const AllOfGroup* group, std::string& out)
        {
            out += "{";
            
            out += "\"name\":\"";
            out += group->name ? group->name : "AllOf";
            out += "\",";

            out += "\"type\":\"AllOfGroup\",";

            out += "\"required\":";
            out += (group->policy == FieldPolicy::Required) ? "true" : "false";

            out += ",\"fields\":[";

            for (int i = 0; group->fields[i] != nullptr; ++i) {
                if (i > 0) out += ",";
                buildField(group->fields[i], out);
            }

            out += "]";
            out += "}";
        }

    }

}