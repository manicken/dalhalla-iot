/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

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

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "DALHAL_HA_TopicBasePath.h"


#define DALHAL_HA_DD_CFG_ROOT_TOPIC "homeassistant"
#define DALHAL_HA_DD_CFG_TOPIC_FORMAT DALHAL_HA_DD_CFG_ROOT_TOPIC "/%s/" DALHAL_DEVICES_HOME_ASSISTANT_ROOTNAME "_%012llX_%s_%s/config"

namespace DALHAL
{
    typedef void (*HADiscoveryWriteFn)(
        PubSubClient& mqtt,
        const JsonVariant& obj,
        TopicBasePath& topicBasePath
    );

    class HA_DeviceDiscovery {
    public:
        static void SendDiscovery(PubSubClient& mqtt, const char* deviceId_cStr, const char* type_cStr, const char* uid_cStr, const JsonVariant& jsonObj, const JsonVariant& jsonObjGlobal, TopicBasePath& topicBasePath, HADiscoveryWriteFn entityWriter = nullptr);
        static void SendAvailabilityTopicCfg(PubSubClient& mqtt, TopicBasePath& topicBasePath);
        /** note this return a owned ptr so it need delete[] when done with */
        static const char* GetDiscoveryCfgTopic(const char* deviceId_cStr, const char* type_cStr, const char* uid_cStr);
        
    private:
        static void SendBaseData(PubSubClient& mqtt, const char* deviceId_cStr, const JsonVariant& jsonObj);
        static void SendDeviceGroupData(PubSubClient& mqtt, const JsonVariant& jsonObjDeviceGroup);
    };

    class PSC_JsonWriter {
    public:
        static void key(PubSubClient& mqtt, const char* key);
        static void kv(PubSubClient& mqtt, const char* key, const char* value);
        static void kv(PubSubClient& mqtt, const JsonPair& kv);
        static void val(PubSubClient& mqtt, const JsonVariant& valueObj);
        static void copyFromJsonObj(PubSubClient& mqtt, const JsonVariant &jsonObj, const char* key, bool last = false);
        static void SendAllItems(PubSubClient& mqtt, const JsonVariant &jsonObj);
        static void printf_str(PubSubClient& mqtt, const char* fmt, ...)
                __attribute__((format(printf, 2, 3)));
        static void printf_zcstr(PubSubClient& mqtt, const char* fmt, ...)
                __attribute__((format(printf, 2, 3)));
        static void printf_str_indexed(PubSubClient& mqtt, const char* fmt, const char* args[], int argCount=0);
    };
} // namespace DALHAL
