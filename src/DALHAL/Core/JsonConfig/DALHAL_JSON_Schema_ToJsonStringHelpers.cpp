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

#include "DALHAL_JSON_Schema_ToJsonStringHelpers.h"

#include <DALHAL/Core/Types/DALHAL_Registry.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfRegistryItems.h>

#include <string>
#include <vector>
#include <cstring>
#include <math.h>

namespace DALHAL {

    namespace JsonSchema {

        namespace ToJsonString {

            std::vector<JsonKeyValue> registers;
            // inlines are not outputted as key/value pairs instead we
            // just use the key here for simple lockup of allready stored names
            std::vector<JsonKeyValue> inlines;

            void clear() {
                registers.reserve(32);
                inlines.reserve(32);
                registers.clear();
                inlines.clear();
            }

            bool registerContains(const char* key) {
                for (int i=0;i<registers.size();++i) {
                    if (strcmp(registers[i].key, key) == 0) {
                        return true;
                    }
                }
                return false;
            }

            bool inlinesContains(const char* key) {
                for (int i=0;i<inlines.size();++i) {
                    if (strcmp(inlines[i].key, key) == 0) {
                        return true;
                    }
                }
                return false;
            }
            void addToInlines(const char* key, std::string contents) {
                inlines.push_back( {key, contents} );
            }

            void buildJsonSchemas(const Registry::Item* reg, std::string &out) {
                bool first = true;
                for (int i=0;reg[i].typeName != nullptr; i++) {
                    if (first == false) {
                        out += ',';
                    } else { first = false; }
                    appendKey(out, reg[i].typeName);
                    JsonObjectSchema::SchemaToJson(reg[i].def->jsonSchema, out);
                }
            }

            void addRegistrySchemaAndBuild(const Registry::Item* reg, const char* regPath) {
                auto idx = registers.size();
                registers.push_back({ regPath, {} });
                std::string out;
                out.reserve(8192);
                buildJsonSchemas(reg, out);
                registers[idx].value = out;
            }

            void buildCompleteJsonSchemasStartingFrom(const Registry::Item* reg, std::string &out) {
                clear();
                addRegistrySchemaAndBuild(reg, "ROOT");
                // here all json is built now we just combine it all
                out = '{';
                appendKey(out, "registers");
                out += '{';
                int itemCount = registers.size();
                for (int i=0;i<itemCount;++i) {
                    if (i > 0) { out += ','; }
                    if (registers[i].key == nullptr) {
                        out += "null" + std::to_string(i) + ":null";
                    } else {
                        appendKey(out, registers[i].key);
                        out += '{';
                        out += registers[i].value;
                        out += '}';
                    }
                }
                out += '}';
                out += ',';
                appendKey(out, "inlines");
                out += '[';
                itemCount = inlines.size();
                for (int i=0;i<itemCount;++i) {
                    if (i > 0) { out += ','; }
                    out += inlines[i].value;
                }
                out += ']';
                out += '}';
                // dont forget to clean when done
                clear();
            }

            void appendQuoted(std::string& out, const char* str) {
                out += '"';
                if (str) out += str;
                out += '"';
            }

            void appendKey(std::string& out, const char* key) {
                appendQuoted(out, key);
                out += ':';
            }

            void appendBool(std::string& out, bool v) {
                out += (v ? "true" : "false");
            }

            void appendBool(std::string& out, const char* key, bool v) {
                appendKey(out, key);
                appendBool(out, v);
            }

            void appendNumber(std::string& out, const char* key, unsigned int v) {
                appendKey(out, key);
                out += std::to_string(v);
            }

            void appendNumber(std::string& out, const char* key, int v) {
                appendKey(out, key);
                out += std::to_string(v);
            }
            
            void appendNumber(std::string& out, const char* key, float v) {
                appendKey(out, key);
                out += std::to_string(v);
            }

            void appendString(std::string& out, const char* key, const char* cStr) {
                appendKey(out, key);
                appendQuoted(out, cStr);
            }

        }

    }
}