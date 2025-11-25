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
#include "HAL_JSON_HA_TopicBasePath.h"
#include "../../HAL_JSON_ZeroCopyString.h"

namespace HAL_JSON
{
    typedef void (*HADiscoveryWriteFn)(
        PubSubClient& mqtt,
        const JsonVariant& obj,
        TopicBasePath& topicBasePath
    );

    class HA_DeviceDiscovery {
    public:
        static void SendDiscovery(PubSubClient& mqtt, const JsonVariant& jsonObj, const JsonVariant& jsonObjGlobal, TopicBasePath& topicBasePath, HADiscoveryWriteFn entityWriter = nullptr);
    private:
        static void StartSendData(PubSubClient& mqtt, const JsonVariant &jsonObj, TopicBasePath& topicBasePath, int packetLength);
        static void SendBaseData(PubSubClient& mqtt, const JsonVariant &jsonObj, TopicBasePath& topicBasePath);
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
} // namespace HAL_JSON
