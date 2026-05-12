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

#include <System/DeviceUID.h> // getDeviceUID

namespace DALHAL
{

    void HA_DeviceDiscovery::SubscribeToCleanupTopic(PubSubClient& mqtt) {
        uint64_t unitDeviceUID = getDeviceUID(); // this is from DeviceUID.h and usually returns the MAC-adress
        mqtt.subscribe_fmt(DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_CLEANUP_SUBSCRIBE_TOPIC_FMT, /*QoS*/0, unitDeviceUID);
    }

    void HA_DeviceDiscovery::SubscribeToCommandTopic(PubSubClient& mqtt, const char* deviceID_cStr) {
        mqtt.subscribe_fmt(DALHAL_DEV_HOME_ASSISTANT_DD_COMMAND_SUBSCRIBE_TOPIC_FMT,/*QoS*/0, deviceID_cStr);
    }
    
    /*const char* HA_DeviceDiscovery::GetDiscoveryCfgTopic(const char* deviceId_cStr, const char* type_cStr, const char* uid_cStr) {
        const char* cfgFormatStr = DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_FMT;
        uint64_t unitDeviceUID = getDeviceUID(); // this is from DeviceUID.h and usually returns the MAC-adress
        int ddTopicLength = snprintf(nullptr, 0, cfgFormatStr, type_cStr, unitDeviceUID, deviceId_cStr, uid_cStr);
        ddTopicLength++;
        char* topicStr = new char[ddTopicLength];
        snprintf(topicStr, ddTopicLength, cfgFormatStr, type_cStr, unitDeviceUID, deviceId_cStr, uid_cStr);
        return topicStr;
    }*/

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
        //const char* cfgTopic_cStr = GetDiscoveryCfgTopic(ctx.cStr_deviceId, ctx.cStr_type, ctx.cStr_entity_uid);
        //mqtt.beginPublish(cfgTopic_cStr, dryRunPSC.count, true);
        Serial.println(F("HASS DD CONFIG TOPIC FMT: " DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_FMT));
        
        uint64_t unitDeviceUID = getDeviceUID(); // this is from DeviceUID.h and usually returns the MAC-adress
        uint32_t unitDeviceUID_MSB = (uint32_t)(unitDeviceUID>>32);
        uint32_t unitDeviceUID_LSB = (uint32_t)(unitDeviceUID & 0xFFFFFFFF);
        //Serial.print(F("HASS DD CONFIG unitDeviceUID: ")); Serial.print(unitDeviceUID);
        Serial.print(F("HASS DD CONFIG ctx.cStr_deviceId: ")); Serial.println(ctx.cStr_deviceId);
        Serial.print(F("HASS DD CONFIG ctx.cStr_entity_uid: ")); Serial.println(ctx.cStr_entity_uid);
        if (mqtt.connected() == false) {
            GlobalLogger.Error(F("could NOT Begin Publish discovery because of disconnected"));
            return; // no point of continue here
        }
        bool couldBeginPublish = mqtt.beginPublish_fmt(dryRunPSC.count, /*retained*/true,
            DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_FMT, ctx.cStr_type, unitDeviceUID_MSB, unitDeviceUID_LSB, ctx.cStr_deviceId, ctx.cStr_entity_uid);
        
        const char* cfgTopicOut_cStrStart = mqtt.lastTxTopic();
        const char* cfgTopicOut_cStrEnd = cfgTopicOut_cStrStart + mqtt.lastTxTopicLength();
        ZeroCopyString zcCfgTopic(cfgTopicOut_cStrStart, cfgTopicOut_cStrEnd);

        if (couldBeginPublish == false) {
            GlobalLogger.Error(F("could NOT Begin Publish discovery:"), zcCfgTopic);
            return; // no point of continue here
        }
        mqtt.write('{'); // start of json object
        mqtt.write('\n'); // easier debug prints
        HA_DeviceDiscovery::SendDeviceGroupData(mqtt, ctx);
        HA_DeviceDiscovery::SendBaseData(mqtt, ctx);
        if (entityWriter)
            entityWriter(mqtt, topicBasePath);
        mqtt.write('}'); // end of json object
        
        
        // one big issue right now is that endPublish allways returns 1
        if (mqtt.endPublish()) {
            GlobalLogger.Info(F("sent discovery: "), zcCfgTopic);
        } else {
            GlobalLogger.Error(F("could NOT send discovery: "), zcCfgTopic);
        }
        //delete[] cfgTopic_cStr; // safe to do here as beginPublish copies the string
    }

    void HA_DeviceDiscovery::SendAvailabilityTopicCfg(PubSubClient& mqtt, TopicBasePath& topicBasePath) {
        const char* availabilityTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::Status);
        PSC_JsonWriter::kv(mqtt, "availability_topic", availabilityTopicStr); mqtt.write(','); mqtt.write('\n');
        PSC_JsonWriter::kv(mqtt, "payload_available", DALHAL_DEV_HOME_ASSISTANT_DD_AVAILABILITY_ONLINE); mqtt.write(','); mqtt.write('\n');
        PSC_JsonWriter::kv(mqtt, "payload_not_available", DALHAL_DEV_HOME_ASSISTANT_DD_AVAILABILITY_OFFLINE);
    }

    void HA_DeviceDiscovery::SendBaseData(PubSubClient& mqtt, const HA_DD_Context& ctx) {

        // optional parameters
        if (ctx.jsonObj_discovery.size() != 0) {
            PSC_JsonWriter::SendAllItems(mqtt, ctx.jsonObj_discovery);
            mqtt.write(',');
            mqtt.write('\n');
        }
        
        const char* rootName_cStr = DALHAL_DEV_HOME_ASSISTANT_DD_BASENAME;
        
        PSC_JsonWriter::printf_str(mqtt, JSON("unique_id":"%s_%s_%s",\n), rootName_cStr, ctx.cStr_deviceId, ctx.cStr_entity_uid);
        PSC_JsonWriter::kv(mqtt, "name", ctx.cStr_name);
    }

    void HA_DeviceDiscovery::SendDeviceGroupData(PubSubClient& mqtt, const HA_DD_Context& ctx) {
        const char* jsonFmt = JSON(
            "device": {\n
            "identifiers": ["%s"],\n
            "name": "%s"},\n
        );
        //mqtt.printf(jsonFmt, ctx.cStr_groupID, ctx.cStr_groupName);
        // vs
        PSC_JsonWriter::printf_str(mqtt, jsonFmt, ctx.cStr_groupID, ctx.cStr_groupName);
    }

} // namespace DALHAL
