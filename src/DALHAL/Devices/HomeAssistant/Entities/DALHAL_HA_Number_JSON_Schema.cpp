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

#include "DALHAL_HA_Number_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_String.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringUID_Path.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_CreateFunctionContext.h>

#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_DeviceDiscovery.h>

#include "DALHAL_HA_Number.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace HA_Number {

            constexpr SchemaString nameField = {"name", FieldPolicy::Required};
            constexpr SchemaObject discoveryField = {"discovery", FieldPolicy::Optional, nullptr}; // nullptr here makes it completely ignore whats inside for now
            constexpr SchemaStringUID_Path targetField = {"target", FieldPolicy::Required};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &targetField,
                &nameField, 
                &discoveryField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "HA_Number",
                fields,
                nullptr, // no modes
                nullptr, // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(DALHAL::HA_CreateFunctionContext& context, DALHAL::HA_Number* out) {
                const JsonVariant& jsonObj = *(context.jsonObjItem);

                const char* uid_cStr = JsonSchema::GetValue(JsonSchema::CommonBase::uidFieldRequired, context).asConstChar();
                out->uid = encodeUID(uid_cStr);

                const char* deviceId_cStr = context.deviceId_cStr;
        
                out->topicBasePath.Set(deviceId_cStr, uid_cStr);

                const char* target_cStr = JsonSchema::GetValue(JsonSchema::HA_Number::targetField, context).asConstChar();
                
                ZeroCopyString zcSrcDeviceUidStr = target_cStr; // target_cStr cannot be nullptr as that is a required field
                out->cda = new CachedDeviceAccess();
                if (out->cda->Set(zcSrcDeviceUidStr) == false) {
                    delete out->cda;
                    out->cda = nullptr;
                }

                DALHAL::HA_DeviceDiscovery::SendDiscovery(
                    context.mqttClient, 
                    deviceId_cStr, 
                    context.deviceType, 
                    uid_cStr, 
                    jsonObj, 
                    *(context.jsonGlobal), 
                    out->topicBasePath, 
                    DALHAL::HA_Number::SendDeviceDiscovery
                );
            }

        }

    }

}