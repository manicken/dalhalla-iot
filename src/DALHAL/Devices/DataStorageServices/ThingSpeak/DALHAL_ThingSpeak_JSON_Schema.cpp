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

#include "DALHAL_ThingSpeak_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Bool.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringSizeConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringUID_Path.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>

#include "DALHAL_ThingSpeak.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace ThingSpeak {

            constexpr SchemaUInt firstUpdateAfterSecondsField = {"firstUpdateAfterSeconds", FieldPolicy::Optional, (unsigned int)0, (unsigned int)0, (unsigned int)0};
            constexpr SchemaBool testserverField = {"testserver", FieldPolicy::Optional, false};
            constexpr SchemaStringSizeConstrained keyField = {"key", FieldPolicy::Required, "0123456789ABCDEF", (unsigned int)16, (unsigned int)16}; // here min/max defines so that the string must be exact 16 characters long

            constexpr SchemaStringUID_Path itemsF1 = {"1", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF2 = {"2", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF3 = {"3", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF4 = {"4", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF5 = {"5", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF6 = {"6", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF7 = {"7", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF8 = {"8", FieldPolicy::Optional};

            constexpr const SchemaTypeBase* itemsFields[] = {&itemsF1, &itemsF2, &itemsF3, &itemsF4, &itemsF5, &itemsF6, &itemsF7, &itemsF8, nullptr};

            constexpr JsonObjectSchema itemsFieldScheme = {
                "ThingSpeakField",
                itemsFields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Error,
                UnknownFieldPolicy::Error,
            };

            constexpr SchemaObject itemsField = {"items", FieldPolicy::Required, &itemsFieldScheme};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &CommonTime::refreshTimeGroupFields,
                &firstUpdateAfterSecondsField,
                &testserverField,
                &keyField,
                &itemsField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "ThingSpeak",
                fields,
                nullptr, // no modes
                nullptr, // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };


            void Extractors::Apply(const DALHAL::DeviceCreateContext& context, DALHAL::ThingSpeak* out) {
                const JsonVariant& jsonObj = *(context.jsonObjItem);

                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*(context.jsonObjItem)));

                HALValue refreshTimeMsTemp = JsonSchema::CommonTime::refreshTimeGroupFields.ExtractFrom(*(context.jsonObjItem));
                if (refreshTimeMsTemp.isSet()) {
                    out->refreshTimeMs = refreshTimeMsTemp.toUInt();
                    out->useOwnTaskLoop = true;
                } else {
                    out->refreshTimeMs = 0;
                    out->useOwnTaskLoop = false;
                }
                bool testserverEnabled = JsonSchema::ThingSpeak::testserverField.ExtractFrom(*(context.jsonObjItem));
                if (testserverEnabled) {
        #ifdef DALHAL_DEVICES_THINGSPEAK_TEST_SERVER
                    out->TS_ROOT_URL = "http://" DALHAL_DEVICES_THINGSPEAK_TEST_SERVER ":8083/update?api_key=";
        #else
                    out->ts_root_url = "http://127.0.0.1:8083/update?api_key=";
        #endif
                } else {
                    out->ts_root_url = "http://api.thingspeak.com/update?api_key=";
                }

                const char* keyStr = JsonSchema::ThingSpeak::keyField.ExtractFrom(*(context.jsonObjItem));
                strncpy(out->api_key, keyStr, sizeof(out->api_key) - 1);
                out->api_key[sizeof(out->api_key) - 1] = '\0'; // ensure null-termination


                const JsonObject& items = JsonSchema::SchemaObject::GetValidatedJsonObject(JsonSchema::ThingSpeak::itemsField, jsonObj);
                out->fieldCount = items.size();
                out->fields = new ThingSpeakField[out->fieldCount];
                int index = 0;
                for (const JsonPair& kv : items) {
                    //int fieldIndex = atoi(kv.key().c_str());
                    uint8_t fieldIndex = kv.key().c_str()[0] - '0';
                    const char* uidPathAndFuncName_cStr = kv.value().as<const char*>();
                    out->fields[index++].Set(fieldIndex, uidPathAndFuncName_cStr);
                }

                out->urlApi.reserve(strlen(out->ts_root_url) + strlen(out->api_key) + out->fieldCount*(strlen("&fieldx=")+32));

                if (out->useOwnTaskLoop) {
                    uint32_t firstUpdateAfterSeconds = JsonSchema::ThingSpeak::firstUpdateAfterSecondsField.ExtractFrom(*(context.jsonObjItem));
                    if (firstUpdateAfterSeconds == 0) {
                        out->lastUpdateMs = millis() - out->refreshTimeMs; // force a first update directly
                    } else {
                        out->lastUpdateMs = millis() - firstUpdateAfterSeconds*1000; // force a first update after timeout
                    }
                }
            }

        }

    }

}