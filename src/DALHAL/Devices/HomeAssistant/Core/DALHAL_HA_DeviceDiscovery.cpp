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

#include "DALHAL_HA_DeviceDiscovery.h"

#include "DALHAL_PubSubClient_JsonWriter.h"

#include <cstdarg> // variadic

#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_Constants.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_CountingPubSubClient.h>

#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <System/DeviceUID.h>

namespace DALHAL
{

    const char* HA_DeviceDiscovery::GetDiscoveryCfgTopic(const char* deviceId_cStr, const char* type_cStr, const char* uid_cStr) {
        const char* cfgFormatStr = DALHAL_HA_DD_CFG_TOPIC_FORMAT;
        uint64_t deviceUID = getDeviceUID(); // this is from System.h and usually returns the MAC-adress
        int ddTopicLength = snprintf(nullptr, 0, cfgFormatStr, type_cStr, deviceUID, deviceId_cStr, uid_cStr);
        ddTopicLength++;
        char* topicStr = new char[ddTopicLength];
        snprintf(topicStr, ddTopicLength, cfgFormatStr, type_cStr, deviceUID, deviceId_cStr, uid_cStr);
        return topicStr;
    }

    void HA_DeviceDiscovery::SendDiscovery(PubSubClient& mqtt, const HA_DD_Context& ctx, TopicBasePath& topicBasePath, HADiscoveryWriteFn entityWriter) {//, const char* deviceId_cStr, const char* type_cStr, const char* uid_cStr, const JsonVariant& jsonObj, const JsonVariant& jsonObjGlobal, TopicBasePath& topicBasePath, HADiscoveryWriteFn entityWriter) {
        
        // first dry run to calculate payload size
        CountingPubSubClient dryRunPSC;
        dryRunPSC.write('{'); // start of json object
        dryRunPSC.write('\n'); // easier debug prints
        //if (jsonObjGlobal.isNull() == false)
        HA_DeviceDiscovery::SendDeviceGroupData(dryRunPSC, ctx);
        
        HA_DeviceDiscovery::SendBaseData(dryRunPSC, ctx);
        if (entityWriter)
            entityWriter(dryRunPSC, topicBasePath);
        dryRunPSC.write('}'); // end of json object

        // second real send 
        const char* cfgTopic_cStr = GetDiscoveryCfgTopic(ctx.cStr_deviceId, ctx.cStr_type, ctx.cStr_entity_uid);
        mqtt.beginPublish(cfgTopic_cStr, dryRunPSC.count, true);
        
        

        mqtt.write('{'); // start of json object
        mqtt.write('\n'); // easier debug prints
        HA_DeviceDiscovery::SendDeviceGroupData(mqtt, ctx);
        HA_DeviceDiscovery::SendBaseData(mqtt, ctx);
        if (entityWriter)
            entityWriter(mqtt, topicBasePath);
        mqtt.write('}'); // end of json object
        if (mqtt.endPublish()) {
            GlobalLogger.Info(F("sent discovery:"), cfgTopic_cStr);
        } else {
            GlobalLogger.Error(F("while trying to send discovery:"), cfgTopic_cStr);
        }
        delete[] cfgTopic_cStr; // safe to do here as beginPublish copies the string
    }

    void HA_DeviceDiscovery::SendAvailabilityTopicCfg(PubSubClient& mqtt, TopicBasePath& topicBasePath) {
        const char* availabilityTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::Status);
        PSC_JsonWriter::kv(mqtt, "availability_topic", availabilityTopicStr); mqtt.write(','); mqtt.write('\n');
        PSC_JsonWriter::kv(mqtt, "payload_available", DALHAL_HOME_ASSISTANT_AVAILABILITY_ONLINE); mqtt.write(','); mqtt.write('\n');
        PSC_JsonWriter::kv(mqtt, "payload_not_available", DALHAL_HOME_ASSISTANT_AVAILABILITY_OFFLINE);
    }

    void HA_DeviceDiscovery::SendBaseData(PubSubClient& mqtt, const HA_DD_Context& ctx) {

        // optional parameters
        if (ctx.jsonObj_discovery.size() != 0) {
            PSC_JsonWriter::SendAllItems(mqtt, ctx.jsonObj_discovery);
            mqtt.write(',');
            mqtt.write('\n');
        }
        
        const char* rootName_cStr = DALHAL_DEVICES_HOME_ASSISTANT_ROOTNAME;
        
        PSC_JsonWriter::printf_str(mqtt, JSON("unique_id":"%s_%s_%s",\n), rootName_cStr, ctx.cStr_deviceId, ctx.cStr_entity_uid);
        PSC_JsonWriter::kv(mqtt, "name", ctx.cStr_name);
    }

    void HA_DeviceDiscovery::SendDeviceGroupData(PubSubClient& mqtt, const HA_DD_Context& ctx) {
        const char* jsonFmt = JSON(
            "device": {\n
            "identifiers": ["%s"],\n
            "name": "%s"},\n
        );
        PSC_JsonWriter::printf_str(mqtt, jsonFmt, ctx.cStr_groupID, ctx.cStr_groupName);
    }

} // namespace DALHAL
