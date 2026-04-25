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

#include "DALHAL_HA_BinarySensor_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_String.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Consumer.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_CreateFunctionContext.h>

#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_DeviceDiscovery.h>

#include <DALHAL/Core/Manager/DALHAL_DeviceManager.h>

#include <DALHAL/Core/Types/DALHAL_Consumer.h>

#include "DALHAL_HA_BinarySensor.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace HA_BinarySensor {

            constexpr SchemaString nameField = {"name", FieldPolicy::Required};
            constexpr SchemaObject discoveryField = {"discovery", FieldPolicy::Optional, nullptr}; // nullptr here makes it completely ignore whats inside for now

            constexpr const SchemaTypeBase* fields[] = {
                //&CommonBase::disabled_type_uidreq_note_group,
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                //&CommonTime::refreshTimeGroupFields, // DALHAL_CommonSchemas_Time
                //&CommonConsumer::sourceField,
                //&CommonConsumer::eventSourceField,
                &CommonConsumer::consumerFieldsGroup, // includes: refreshTimeGroupFields, sourceField, eventSourceField
                &nameField, 
                &discoveryField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "HA_BinarySensor",
                fields,
                CommonConsumer::consumerDeviceModes, // DALHAL_CommonSchemas_Consumer
                nullptr, // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(DALHAL::HA_CreateFunctionContext& context, DALHAL::HA_BinarySensor* out) {
                const char* uid_cStr = JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*(context.jsonObjItem));
                out->uid = encodeUID(uid_cStr);

                JsonSchema::CommonConsumer::ConsumerStruct consumerData;
                JsonSchema::ModeSelector::Apply(JsonSchema::HA_BinarySensor::Root.modes, context, &consumerData);
                const char* source_cStr = nullptr;
                const char* eventSource_cStr = nullptr;

                if (consumerData.mode == DALHAL::Consumer::Mode::TimedRefresh) {
                    out->refreshMs = consumerData.refreshtimems;
                    source_cStr = consumerData.source;
                } else if (consumerData.mode == DALHAL::Consumer::Mode::Event) {
                    out->refreshMs = 0;
                    source_cStr = consumerData.source;
                    eventSource_cStr = consumerData.eventSource;
                } else if (consumerData.mode == DALHAL::Consumer::Mode::Manual) {
                    out->refreshMs = 0;

                } // else there are no other modes, it's either above or else the cfg is rejected and this will not happend

                // right now we include error check here as the consumer schema stuff is not yet finished
                if (source_cStr != nullptr) {
                    ZeroCopyString zcSrcDeviceUidStr = source_cStr;
                    out->cdr = new CachedDeviceRead();
                    if (out->cdr->Set(zcSrcDeviceUidStr) == false) {
                        // emit errors inside so no reporting is needed here unless one need to specific
                        delete out->cdr;
                        out->cdr = nullptr;
                    }
                }
                if (eventSource_cStr != nullptr) {
                    ZeroCopyString zcSrcDeviceUidStr = eventSource_cStr;
                    DeviceManager::GetDeviceEvent(zcSrcDeviceUidStr, &out->eventSource);
                }
                // have the following to make the modes explicit for now
                // later it will be directly determined by the mode extractors
                // as they will check if the sources exist before deciding the final mode
                if (out->refreshMs == 0) {
                    if (out->cdr == nullptr || out->eventSource == nullptr) {
                        // it make no sense if either is nullptr
                        // when refreshMs == 0
                        out->consumerMode = DALHAL::Consumer::Mode::Manual;
                    } else { //  this mean that both are valid
                        out->consumerMode = DALHAL::Consumer::Mode::Event;
                    }
                } else if (out->cdr != nullptr) {
                    out->consumerMode = DALHAL::Consumer::Mode::TimedRefresh;
                } else {
                    out->consumerMode = DALHAL::Consumer::Mode::Manual;
                }

                const char* deviceId_cStr = context.deviceId_cStr;
        
                out->topicBasePath.Set(deviceId_cStr, uid_cStr);

                DALHAL::HA_DeviceDiscovery::SendDiscovery(
                    context.mqttClient, 
                    deviceId_cStr, 
                    context.deviceType, 
                    uid_cStr, 
                    *context.jsonObjItem, 
                    *(context.jsonGlobal), 
                    out->topicBasePath, 
                    DALHAL::HA_BinarySensor::SendDeviceDiscovery
                );

            }

        }

    }

}