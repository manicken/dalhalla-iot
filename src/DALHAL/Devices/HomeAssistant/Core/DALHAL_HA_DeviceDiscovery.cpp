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
        mqtt.subscribe_fmt(DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_CLEANUP_SUBSCRIBE_TOPIC_FMT, /*QoS*/0, DeviceUID::Get());
    }

    void HA_DeviceDiscovery::SubscribeToCommandTopic(PubSubClient& mqtt) {
        mqtt.subscribe_fmt(DALHAL_DEV_HOME_ASSISTANT_DD_COMMAND_SUBSCRIBE_TOPIC_FMT,/*QoS*/0, DeviceUID::Get());
    }
    /** used by both the dry run counter and the real send */
    void HA_DeviceDiscovery::SendDiscoveryPayload(PubSubClient& psc, const HA_DD_Context& ctx, HADiscoveryWriteFn entityWriter) {
        psc.write('{'); // start of json object
        psc.write('\n'); // easier debug prints
        //if (jsonObjGlobal.isNull() == false)
        HA_DeviceDiscovery::SendDeviceGroupData(psc, ctx);
        HA_DeviceDiscovery::SendBaseData(psc, ctx);
        if (entityWriter)
            entityWriter(psc, ctx);
        psc.write('}'); // end of json object
    }

    void HA_DeviceDiscovery::SendDiscovery(PubSubClient& mqtt, const HA_DD_Context& ctx, HADiscoveryWriteFn entityWriter) {
        // first dry run to calculate payload size
        CountingPubSubClient dryRunPSC;
        SendDiscoveryPayload(dryRunPSC, ctx, entityWriter);

        // second real send 
        /*Serial.printf("\r\nHASS DD - prepare to send (payloadsize=%d):" DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_FMT "\r\n",
            dryRunPSC.count, ctx.cStr_type, unitDeviceUID_MSB, unitDeviceUID_LSB, ctx.cStr_deviceId, ctx.cStr_entity_uid
        );*/

        if (mqtt.connected() == false) {
            GlobalLogger.Error(F("could NOT Begin Publish discovery because of disconnected"));
            return; // no point of continue here
        }
        bool couldBeginPublish = mqtt.beginPublish_fmt(
            dryRunPSC.count, /*retained*/true,
            DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_FMT, 
            ctx.cStr_type, DeviceUID::Get(), ctx.cStr_hass_uid
        );
        
        const char* cfgTopicOut_cStrStart = mqtt.lastTxTopic();
        const char* cfgTopicOut_cStrEnd = cfgTopicOut_cStrStart + mqtt.lastTxTopicLength();
        ZeroCopyString zcCfgTopic(cfgTopicOut_cStrStart, cfgTopicOut_cStrEnd);

        if (couldBeginPublish == false) {
            GlobalLogger.Error(F("could NOT Begin Publish discovery:"), zcCfgTopic);
            return; // no point of continue here
        }
        SendDiscoveryPayload(mqtt, ctx, entityWriter);
        
        if (mqtt.endPublish()) {
            GlobalLogger.Info(F("sent discovery: "), zcCfgTopic);
        } else {
            GlobalLogger.Error(F("could NOT send discovery: "), zcCfgTopic);
        }
        
    }

    void HA_DeviceDiscovery::SendStateTopicCfg(PubSubClient& mqtt, const HA_DD_Context& ctx) {
        mqtt.printf(",\n" JSON("state_topic":) "\"" DALHAL_DEV_HOME_ASSISTANT_DD_ENTITY_TOPIC_FMT "\"", DeviceUID::Get(), ctx.cStr_hass_uid, DALHAL_DEV_HOME_ASSISTANT_DD_STATE_TOPIC_TAIL);
    }

    void HA_DeviceDiscovery::SendCommandTopicCfg(PubSubClient& mqtt, const HA_DD_Context& ctx) {
        mqtt.printf(",\n" JSON("command_topic":) "\"" DALHAL_DEV_HOME_ASSISTANT_DD_ENTITY_TOPIC_FMT "\"", DeviceUID::Get(), ctx.cStr_hass_uid, DALHAL_DEV_HOME_ASSISTANT_DD_COMMAND_TOPIC_TAIL);
    }

    void HA_DeviceDiscovery::SendAvailabilityTopicCfg(PubSubClient& mqtt, const HA_DD_Context& ctx) {
        mqtt.printf(",\n" JSON("availability_topic":) "\"" DALHAL_DEV_HOME_ASSISTANT_DD_ENTITY_TOPIC_FMT "\"", DeviceUID::Get(), ctx.cStr_hass_uid, DALHAL_DEV_HOME_ASSISTANT_DD_STATUS_TOPIC_TAIL);
        
        mqtt.write(','); mqtt.write('\n'); PSC_JsonWriter::kv(mqtt, "payload_available", DALHAL_DEV_HOME_ASSISTANT_DD_AVAILABILITY_ONLINE); 
        mqtt.write(','); mqtt.write('\n'); PSC_JsonWriter::kv(mqtt, "payload_not_available", DALHAL_DEV_HOME_ASSISTANT_DD_AVAILABILITY_OFFLINE);
    }

    void HA_DeviceDiscovery::SendBaseData(PubSubClient& mqtt, const HA_DD_Context& ctx) {

        // optional parameters
        if (ctx.jsonObj_discovery.size() != 0) {
            PSC_JsonWriter::SendAllItems(mqtt, ctx.jsonObj_discovery);
            mqtt.write(',');
            mqtt.write('\n');
        }
        
        mqtt.printf(JSON("unique_id":) "\"" DALHAL_DEV_HOME_ASSISTANT_DD_UNIQUE_ID_FMT "\",\n", DeviceUID::Get(), ctx.cStr_hass_uid);
        if (ctx.cStr_hass_prev_uid != nullptr && ctx.cStr_hass_prev_uid[0] != '\0') { // quick zero length check
            mqtt.printf(JSON("previous_unique_id":) "\"" DALHAL_DEV_HOME_ASSISTANT_DD_UNIQUE_ID_FMT "\",\n", DeviceUID::Get(), ctx.cStr_hass_prev_uid);
        }
        PSC_JsonWriter::kv(mqtt, "name", ctx.cStr_name);
    }

    void HA_DeviceDiscovery::SendDeviceGroupData(PubSubClient& mqtt, const HA_DD_Context& ctx) {
        const char* jsonFmt = JSON(
            "device": {\n
            "identifiers": ["%s"],\n
            "name": "%s"},\n
        );
        mqtt.printf(jsonFmt, ctx.cStr_groupID, ctx.cStr_groupName);
        // vs
        //PSC_JsonWriter::printf_str(mqtt, jsonFmt, ctx.cStr_groupID, ctx.cStr_groupName);
    }


    void HA_DeviceDiscovery::SetAvailability(PubSubClient& mqtt, const char* hass_uid_cStr, bool& wasOnline, bool online) {
        if (online == wasOnline) return;
        
        uint32_t pLength = (online ? sizeof(DALHAL_DEV_HOME_ASSISTANT_DD_AVAILABILITY_ONLINE)
                : sizeof(DALHAL_DEV_HOME_ASSISTANT_DD_AVAILABILITY_OFFLINE)) - 1;

        mqtt.beginPublish_fmt(pLength, /*retained*/false, 
            DALHAL_DEV_HOME_ASSISTANT_DD_ENTITY_TOPIC_FMT, 

            DeviceUID::Get(), 
            hass_uid_cStr, 
            DALHAL_DEV_HOME_ASSISTANT_DD_STATUS_TOPIC_TAIL
        );
        
        mqtt.print(online ? DALHAL_DEV_HOME_ASSISTANT_DD_AVAILABILITY_ONLINE
                : DALHAL_DEV_HOME_ASSISTANT_DD_AVAILABILITY_OFFLINE);
        
        bool success = mqtt.endPublish();

        if (success) {
            wasOnline = online;
        }
    }

    bool HA_DeviceDiscovery::SendState(PubSubClient& mqtt, const char* hass_uid_cStr, const HALValue val) {
        CountingPubSubClient count_psc; // use this to avoid placing value string on heap just to know it's size
        PrintHALValue(val, count_psc); // this allways success so no need to check if the print was success

        bool res = mqtt.beginPublish_fmt(count_psc.count, /*retained*/false, 
            DALHAL_DEV_HOME_ASSISTANT_DD_ENTITY_TOPIC_FMT, 

            DeviceUID::Get(),
            hass_uid_cStr,
            DALHAL_DEV_HOME_ASSISTANT_DD_STATE_TOPIC_TAIL
        );
        if (res == false) {
            return false;
        }
        if (PrintHALValue(val, mqtt) == 0) { return false; }

        return mqtt.endPublish();
    }

    bool HA_DeviceDiscovery::SendState(PubSubClient& mqtt, const char* hass_uid_cStr, const char* state_cStr, uint32_t state_cStr_length) {
        
        bool res = mqtt.beginPublish_fmt(state_cStr_length, /*retained*/false, 
            DALHAL_DEV_HOME_ASSISTANT_DD_ENTITY_TOPIC_FMT, 

            DeviceUID::Get(),
            hass_uid_cStr,
            DALHAL_DEV_HOME_ASSISTANT_DD_STATE_TOPIC_TAIL
        );
        if (res == false) {
            return false;
        }
        if (mqtt.print(state_cStr) == 0) { return false; }
        return mqtt.endPublish();
    }

} // namespace DALHAL
