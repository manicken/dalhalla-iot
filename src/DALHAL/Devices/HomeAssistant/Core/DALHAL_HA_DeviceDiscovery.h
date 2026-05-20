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

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <DALHAL/Core/Types/DALHAL_Value.h>
#include <DALHAL/Core/Types/DALHAL_ValuePrinter.h>

namespace DALHAL
{
    

    struct HA_DD_Context {
        
        /** the UID used to generate the hass unique_id */
        const char* cStr_hass_uid = nullptr;
        /** the prev UID used to generate the hass previous_unique_id */
        const char* cStr_hass_prev_uid = nullptr;
        /** the type of the HASS entity */
        const char* cStr_type = nullptr;
        /** the displayed name of the HASS entity */
        const char* cStr_name = nullptr;
        /** the group UID */
        const char* cStr_groupID = nullptr;
        /** the displayed name of the group */
        const char* cStr_groupName = nullptr;
        /** the resolved/extracted discovery object */
        const JsonObject& jsonObj_discovery;

        HA_DD_Context(
            /** the UID used to generate the hass unique_id */
            const char* cStr_hass_uid,
            /** the prev UID used to generate the hass previous_unique_id */
            const char* cStr_hass_prev_uid,
            /** the type of the HASS entity */
            const char* cStr_type,
            /** the displayed name of the HASS entity */
            const char* cStr_name,
            /** the group UID */
            const char* cStr_groupID,
            /** the displayed name of the group */
            const char* cStr_groupName,
            /** the resolved/extracted discovery object */
            const JsonObject& jsonObj_discovery
        ) : 
            cStr_hass_uid(cStr_hass_uid),
            cStr_hass_prev_uid(cStr_hass_prev_uid),
            cStr_type(cStr_type),
            cStr_name(cStr_name),
            cStr_groupID(cStr_groupID),
            cStr_groupName(cStr_groupName),
            jsonObj_discovery(jsonObj_discovery)
            {}

    };

    typedef void (*HADiscoveryWriteFn)(
        PubSubClient& mqtt,
        const HA_DD_Context& ctx
    );

    class HA_DeviceDiscovery {
    public:
        /** used by both the dry run counter and the real send */
        static void SendDiscoveryPayload(PubSubClient& mqtt, const HA_DD_Context& ctx, HADiscoveryWriteFn entityWriter);
        static void SendDiscovery(PubSubClient& mqtt, const HA_DD_Context& ctx, HADiscoveryWriteFn entityWriter);
        static void SendAvailabilityTopicCfg(PubSubClient& mqtt, const HA_DD_Context& ctx);
        static void SendStateTopicCfg(PubSubClient& mqtt, const HA_DD_Context& ctx);
        static void SendCommandTopicCfg(PubSubClient& mqtt, const HA_DD_Context& ctx);

        static void SubscribeToCommandTopic(PubSubClient& mqtt);
        static void SubscribeToCleanupTopic(PubSubClient& mqtt);

        static void SetAvailability(PubSubClient& mqtt, const char* hass_uid_cStr, bool& wasOnline, bool online);

        static bool SendState(PubSubClient& mqtt, const char* hass_uid_cStr, const HALValue val);
        static bool SendState(PubSubClient& mqtt, const char* hass_uid_cStr, const char* state_cStr, uint32_t state_cStr_length);
        
    private:
        static void SendBaseData(PubSubClient& mqtt, const HA_DD_Context& ctx);//, const char* deviceId_cStr, const JsonVariant& jsonObj);
        static void SendDeviceGroupData(PubSubClient& mqtt, const HA_DD_Context& ctx);//, const JsonVariant& jsonObjDeviceGroup);
    };

    
} // namespace DALHAL
