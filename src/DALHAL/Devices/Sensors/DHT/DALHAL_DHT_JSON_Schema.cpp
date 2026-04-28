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

#include "DALHAL_DHT_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringAnyOfArrayConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>

#include <DHTesp.h>
#include "DALHAL_DHT.h"

// DHT models
#define DALHAL_TYPE_DHT_MODEL_DHT11       "DHT11"
#define DALHAL_TYPE_DHT_MODEL_DHT22       "DHT22"
#define DALHAL_TYPE_DHT_MODEL_AM2302      "AM2302"
#define DALHAL_TYPE_DHT_MODEL_RHT03       "RTH03"

namespace DALHAL {

    namespace JsonSchema {

        namespace DHT {

            struct ModelItem {
                const char* name;
                DHTesp::DHT_MODEL_t model;
            };

            constexpr ModelItem notFoundItem = {nullptr, DHTesp::DHT_MODEL_t::AUTO_DETECT};

            constexpr ModelItem modelsTable[] = {
                {DALHAL_TYPE_DHT_MODEL_DHT11, DHTesp::DHT_MODEL_t::DHT11},
                {DALHAL_TYPE_DHT_MODEL_DHT22, DHTesp::DHT_MODEL_t::DHT22},
                {DALHAL_TYPE_DHT_MODEL_AM2302, DHTesp::DHT_MODEL_t::AM2302},
                {DALHAL_TYPE_DHT_MODEL_RHT03, DHTesp::DHT_MODEL_t::RHT03},
            };
            constexpr size_t modelsTable_size = sizeof(modelsTable)/sizeof(modelsTable[0]);

            const ModelItem& GetModel(const char* name) {
                for (int i=0; i<(int)modelsTable_size; ++i) {
                    if (strcasecmp(modelsTable[i].name, name) == 0) {
                        return modelsTable[i];
                    }
                }
                return notFoundItem;
            }

            bool CheckModelType(void* ctx, const char* type) { // here ctx is not used as we can access the table directly
                const ModelItem& def = GetModel(type);
                if (def.name != nullptr) {
                    return true;
                }
                return false;
            }

            std::string GetModelStrings(void* ctx) { // here ctx is not used as we can access the table directly
                std::string out;
                out = '[';
                for (int i=0; i<(int)modelsTable_size; ++i) {
                    if (i>0) {
                        out += ',';
                    }
                    out += '"'; out += modelsTable[i].name; out += '"';
                }
                out += ']';
                return out;
            }

            constexpr SchemaStringAnyOfByFuncConstrained modelField = {"model", FieldPolicy::Required, DALHAL_TYPE_DHT_MODEL_DHT11, CheckModelType, GetModelStrings, nullptr};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &modelField,
                &CommonTime::refreshTimeGroupFieldsRequired,
                &CommonPins::InputOutputPinField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "DHT",
                fields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Error,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(DALHAL::DeviceCreateContext& context, DALHAL::DHT* out) {
                const JsonVariant& jsonObj = *(context.jsonObjItem);
                
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(jsonObj));
                out->pin = JsonSchema::CommonPins::InputOutputPinField.ExtractFrom(jsonObj);
                out->refreshTimeMs = JsonSchema::CommonTime::refreshTimeGroupFieldsRequired.ExtractFrom(jsonObj).asUInt();
                
                const char* modelStr = JsonSchema::DHT::modelField.ExtractFrom(jsonObj);
                const ModelItem& def = GetModel(modelStr);
                out->dht.setup(out->pin, def.model);
            }
            
        }

    }

}