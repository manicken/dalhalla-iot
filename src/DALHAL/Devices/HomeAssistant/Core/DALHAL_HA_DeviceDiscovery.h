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
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_TopicBasePath.h>

namespace DALHAL
{
    typedef void (*HADiscoveryWriteFn)(
        PubSubClient& mqtt,
        TopicBasePath& topicBasePath
    );

    struct HA_DD_Context {
        /** the UID of the virtual/logical device */
        const char* cStr_entity_uid = nullptr;
        /** the ID of the MCU */
        const char* cStr_deviceId = nullptr;
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
            /** the UID of the virtual/logical device */
            const char* cStr_entity_uid,
            /** the ID of the MCU */
            const char* cStr_deviceId,
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
            cStr_entity_uid(cStr_entity_uid),
            cStr_deviceId(cStr_deviceId),
            cStr_type(cStr_type),
            cStr_name(cStr_name),
            cStr_groupID(cStr_groupID),
            cStr_groupName(cStr_groupName),
            jsonObj_discovery(jsonObj_discovery)
            {}

    };

    class HA_DeviceDiscovery {
    public:
        static void SendDiscovery(PubSubClient& mqtt, const HA_DD_Context& ctx, TopicBasePath& topicBasePath, HADiscoveryWriteFn entityWriter); //const char* deviceId_cStr, const char* type_cStr, const char* uid_cStr, const JsonVariant& jsonObj, const JsonVariant& jsonObjGlobal, TopicBasePath& topicBasePath, HADiscoveryWriteFn entityWriter = nullptr);
        static void SendAvailabilityTopicCfg(PubSubClient& mqtt, TopicBasePath& topicBasePath);
        /** note this return a owned ptr so it need delete[] when done with */
        static const char* GetDiscoveryCfgTopic(const char* deviceId_cStr, const char* type_cStr, const char* uid_cStr);

        static void SubscribeToCommandTopic(PubSubClient& mqtt, const char* deviceID_cStr);
        static void SubscribeToCleanupTopic(PubSubClient& mqtt);
        
    private:
        static void SendBaseData(PubSubClient& mqtt, const HA_DD_Context& ctx);//, const char* deviceId_cStr, const JsonVariant& jsonObj);
        static void SendDeviceGroupData(PubSubClient& mqtt, const HA_DD_Context& ctx);//, const JsonVariant& jsonObjDeviceGroup);
    };

    
} // namespace DALHAL
