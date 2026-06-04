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

#include <DALHAL/API/DALHAL_BlockStreamer.h>
#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>

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

            std::vector<DeviceRegistryQueueItem> registers;
            std::vector<JsonObjectSchemaQueueItem> objects;
            std::vector<InlineQueueItem> ByReference;

            void clear() {
                if (registers.capacity() < 32) {
                    registers.reserve(32);
                }
                if (objects.capacity() < 32) {
                    objects.reserve(32);
                }
                if (ByReference.capacity() < 32) {
                    ByReference.reserve(32);
                }
                
                registers.clear();
                objects.clear();
                ByReference.clear();
            }

            bool registerContains(const char* regPath) {
                for (int i=0;i<(int)registers.size();++i) {
                    if (strcmp(registers[i].regPath, regPath) == 0) {
                        return true;
                    }
                }
                return false;
            }

            bool objectsContains(const char* id) {
                for (int i=0;i<(int)objects.size();++i) {
                    if (strcmp(objects[i].id, id) == 0) {
                        return true;
                    }
                }
                return false;
            }

            bool ByReferenceContains(const char* id) {
                for (int i=0;i<(int)ByReference.size();++i) {
                    if (strcmp(ByReference[i].id, id) == 0) {
                        return true;
                    }
                }
                return false;
            }

            void addToByReference(const char* id, const SchemaTypeBase& schema) {
                ByReference.push_back( { id, schema } );
            }

            void addToRegistries(const char* regPath, const Registry::DeviceRegistry& reg) {
                registers.push_back( { regPath, reg } );
            }
            void addToObjects(const char* id, const JsonObjectSchema& schema) {
                objects.push_back( { id, schema } );
            }

            void buildJsonSchemas(const Registry::DeviceRegistry& reg, StringBuilderStreamer& sbs) {
                
                for (size_t i=0;i < reg.count; i++) {
                    if (i > 0) {
                        sbs.write_json_value_separator();
                    }
                    if (reg.items[i].typeName == nullptr) {
                        Serial.println("reg.items[i].typeName == nullptr");
                        sbs.write('"');
                        sbs.write(F("typeName null_"));
                        sbs.write((uint32_t)i);
                        sbs.write('"');
                        sbs.write(F(":null"));
                        continue;
                    }
                    sbs.write_jsonKey(reg.items[i].typeName);

                    JsonObjectSchema::SchemaToJson(reg.items[i].def->jsonSchema, sbs, SchemaEmitMode::ByReference);
                }
            }

            void buildCompleteJsonSchemasStartingFrom(const Registry::DeviceRegistry& reg, CommandCallback cb) {

                DALHAL::BlockStreamer bs(cb, "schema", BlockStreamer::DataType::Json);
                StringBuilderStreamer& sbs = bs.writer();

                clear();

                addToRegistries("ROOT", reg);
                
                // here all json is built now we just combine it all
                sbs.write_json_object_begin();
                sbs.write_jsonKey(F("registers"));
                
                sbs.write_json_object_begin();

                for (size_t i=0; i < registers.size(); i++) {
                    if (i > 0) { sbs.write_json_value_separator(); }
                    if (registers[i].regPath == nullptr) {
                        sbs.write('"');
                        sbs.write(F("regPath null_"));
                        sbs.write((uint32_t)i);
                        sbs.write('"');
                        sbs.write(F(":null"));
                    } else {
                        sbs.write_jsonKey(registers[i].regPath);
                        sbs.write_json_object_begin();
                        buildJsonSchemas(registers[i].reg, sbs);
                        sbs.write_json_object_end();
                    }
                }
                sbs.write_json_object_end();
                sbs.write_json_value_separator();
                sbs.write_jsonKey(F("objects"));
                sbs.write_json_array_begin();

                for (size_t i=0;i<objects.size();++i) {
                    if (i > 0) { sbs.write_json_value_separator(); }
                    JsonSchema::JsonObjectSchema::SchemaToJson(&objects[i].schema, sbs, SchemaEmitMode::ByReference);
                }
                sbs.write_json_array_end();
                sbs.write_json_value_separator();
                sbs.write_jsonKey(F("ByReference"));
                sbs.write_json_array_begin();

                for (size_t i=0;i<ByReference.size();++i) {
                    if (i > 0) { sbs.write_json_value_separator(); }
                    JsonSchema::SchemaToJson(ByReference[i].schema, sbs, SchemaEmitMode::Full);
                    // vs above is short form of
                    //const FieldTypeRegistryItem& item = JsonSchema::GetFieldTypeRegistryItem(ByReference[i].schema.type);
                    //item.define->ToJson(ByReference[i].schema, sbs, SchemaEmitMode::Full);
                }
                sbs.write_json_array_end();
                sbs.write_json_object_end();

                // dont forget to clean when done
                clear();
            }
        }

    }
}