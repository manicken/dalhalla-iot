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

#include <cstdint>
#include <cstring>
#include <string>

#include <ArduinoJson.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Core/Types/DALHAL_Value.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_FieldType.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_FieldPolicy.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_FieldGuiFlags.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>

namespace DALHAL {

    namespace JsonSchema {

        struct SchemaTypeBase {
            const char* name;    // flash string
            FieldType type;
            FieldPolicy policy;
            FieldGuiFlagsType guiFlags;
            size_t structOffset;

        protected:
            static bool ValidateSchemaNameNotNull(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName); 
            static ValidatorResult ValidateFieldPresenceAndPolicy(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError);
            static void SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out);
        public:
            /** special note about using this, it do internally dispatch to the correct type using the JsonSchema type registry */
            HALValue ExtractViaRegistryFrom(const JsonVariant& jsonObj) const;
        
        protected:
            constexpr SchemaTypeBase(const char* n, FieldType t, FieldPolicy policy)
                : name(n), type(t), policy(policy), guiFlags(Gui::None), structOffset(0) {}

            constexpr SchemaTypeBase(const char* n, FieldType t, FieldPolicy policy, FieldGuiFlagsType guiFlags)
                : name(n), type(t), policy(policy), guiFlags(guiFlags), structOffset(0) {}

            constexpr SchemaTypeBase(const char* n, FieldType t, FieldPolicy policy, size_t structOffset)
                : name(n), type(t), policy(policy), guiFlags(Gui::None), structOffset(structOffset) {}

            constexpr SchemaTypeBase(const char* n, FieldType t, FieldPolicy policy, FieldGuiFlagsType guiFlags, size_t structOffset)
                : name(n), type(t), policy(policy), guiFlags(guiFlags), structOffset(structOffset) {}
        };

    }

  }
