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

#pragma once

#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Groups/DALHAL_JSON_Schema_SchemaFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Groups/DALHAL_JSON_Schema_SchemaAllOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Groups/DALHAL_JSON_Schema_SchemaOneOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <string>

namespace DALHAL {
    
    namespace JsonSchema {
        void buildJsonSchemas(const Registry::Item* reg, std::string &out);
        void buildCompleteJsonSchemasStartingFrom(const Registry::Item* reg, std::string &out);
        void buildJsonSchema(const JsonObjectSchema* schema, std::string& out);
        void buildField(const SchemaTypeBase* f, std::string& out);
        void buildPrimitiveField(const SchemaTypeBase* f, std::string& out);
        void buildOneOfGroup(const SchemaOneOfFieldsGroup* group, std::string& out);
        void buildAllOfGroup(const SchemaAllOfFieldsGroup* group, std::string& out);
    }

}