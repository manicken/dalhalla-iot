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

#include "HAL_JSON_HA_DeviceDiscovery.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"
#include "HAL_JSON_HA_CountingPubSubClient.h"
#include <cstdarg> // variadic

#define JSON(...) #__VA_ARGS__

namespace HAL_JSON
{

    void HA_DeviceDiscovery::SendDiscovery(PubSubClient& mqtt, const JsonVariant& jsonObj, const JsonVariant& jsonObjGlobal, TopicBasePath& topicBasePath, HADiscoveryWriteFn entityWriter) {
        
        // first dry run to calculate payload size
        CountingPubSubClient dryRunPSC;
        if (jsonObjGlobal.isNull() == false)
            HA_DeviceDiscovery::SendDeviceGroupData(dryRunPSC, jsonObjGlobal);
        HA_DeviceDiscovery::SendBaseData(dryRunPSC, jsonObj, topicBasePath);
        if (entityWriter)
            entityWriter(dryRunPSC, jsonObj, topicBasePath);

        // second real send 
        HA_DeviceDiscovery::StartSendData(mqtt, jsonObj, topicBasePath, dryRunPSC.count);
        if (jsonObjGlobal.isNull() == false)
            HA_DeviceDiscovery::SendDeviceGroupData(mqtt, jsonObjGlobal);
        HA_DeviceDiscovery::SendBaseData(mqtt, jsonObj, topicBasePath);
        if (entityWriter)
            entityWriter(mqtt, jsonObj, topicBasePath);
        mqtt.endPublish();
    }

    void HA_DeviceDiscovery::StartSendData(PubSubClient& mqtt, const JsonVariant &jsonObj, TopicBasePath& topicBasePath, int packetLength) {
        const char* typeStr = GetAsConstChar(jsonObj,"type");
        const char* uidStr = GetAsConstChar(jsonObj,"uid");
        ZeroCopyString zcDeviceIdStr = topicBasePath.GetDeviceId();
        int ddTopicLength = snprintf(nullptr, 0, "homeassistant/%s/dalhal_%.*s_%s/config", typeStr, (int)zcDeviceIdStr.Length(), zcDeviceIdStr.start, uidStr);
        ddTopicLength++;
        char* topic = new char[ddTopicLength];
        snprintf(topic, ddTopicLength, "homeassistant/%s/dalhal_%.*s_%s/config", typeStr, (int)zcDeviceIdStr.Length(), zcDeviceIdStr.start, uidStr);
        
        mqtt.beginPublish(topic, packetLength, true);
        delete topic;
    }

    void HA_DeviceDiscovery::SendBaseData(PubSubClient& mqtt, const JsonVariant& jsonObj, TopicBasePath& topicBasePath) {
        // availability_topic
        const char* jsonFmt_availability_topic = JSON(
            "availability_topic":"%s",
            "payload_available":"online",
            "payload_not_available":"offline",
        ); // in the above format the rootTopicPath do allways end with /stat
        const char* availabilityTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::Status);
        PSC_JsonWriter::printf_str(mqtt, jsonFmt_availability_topic, availabilityTopicStr);

        // optional parameters
        if (jsonObj.containsKey("discovery")) {
            PSC_JsonWriter::SendAllItems(mqtt, jsonObj["discovery"]);
        }
        /*const char* uidStr = GetAsConstChar(jsonObj,"uid");
        const char* nameStr = GetAsConstChar(jsonObj, "name");
        const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
        const char* args[] = { stateTopicStr, deviceIdStr, uidStr, nameStr, nullptr };
        const char* jsonFmt = JSON(
            "state_topic":"%0",
            "unique_id":"dalhal_%1_%2",
            "name":"%3"
        ); // in the above format the rootTopicPath do allways end with /stat so to get the word state we only need to add e
        PSC_JsonWriter::printf_str_indexed(mqtt, jsonFmt, args);
        */
        ZeroCopyString zcUidStr = GetAsConstChar(jsonObj,"uid");
        ZeroCopyString zcNameStr = GetAsConstChar(jsonObj, "name");
        ZeroCopyString zcStateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
        ZeroCopyString zcDeviceIdStr = topicBasePath.GetDeviceId();
        const char* jsonFmt = JSON(
            "state_topic":"%s",
            "unique_id":"dalhal_%s_%s",
            "name":"%s"
        );
        PSC_JsonWriter::printf_zcstr(mqtt, jsonFmt, &zcStateTopicStr, &zcDeviceIdStr, &zcUidStr, &zcNameStr);
        // dont write comma here as the caller takes care of that
    }

    void HA_DeviceDiscovery::SendDeviceGroupData(PubSubClient& mqtt, const JsonVariant& jsonObjDeviceGroup) {
        const char* deviceGroupUIDstr = GetAsConstChar(jsonObjDeviceGroup,"uid");
        const char* nameStr = GetAsConstChar(jsonObjDeviceGroup, "name");
        const char* manufacturerStr = GetAsConstChar(jsonObjDeviceGroup, "manufacturer");
        if (manufacturerStr == nullptr) manufacturerStr = "Dalhal";
        const char* modelStr = GetAsConstChar(jsonObjDeviceGroup, "model");
        if (modelStr == nullptr) modelStr = "Virtual Sensor";

        const char* jsonFmt = JSON(
            "device": {
            "identifiers": ["%s"],
            "manufacturer": "%s",
            "model": "%s",
            "name": "%s"
            },
        );
        PSC_JsonWriter::printf_str(mqtt, jsonFmt, deviceGroupUIDstr, manufacturerStr, modelStr, nameStr);
    }

    void PSC_JsonWriter::key(PubSubClient& mqtt, const char* key) {
        mqtt.write('"');
        mqtt.write((uint8_t*)key, strlen(key));
        mqtt.write('"');
        mqtt.write(':');
    }

    void PSC_JsonWriter::copyFromJsonObj(PubSubClient& mqtt, const JsonVariant &jsonObj, const char* key, bool last/* = false*/) {
        mqtt.write('"');
        mqtt.write((uint8_t*)key, strlen(key));
        mqtt.write('"');
        mqtt.write(':');
        mqtt.write('"');
        const char* value = GetAsConstChar(jsonObj, key);
        mqtt.write((uint8_t*)value, strlen(value));
        mqtt.write('"');
        if (false == last)
            mqtt.write(',');
    }

    void PSC_JsonWriter::SendAllItems(PubSubClient& mqtt, const JsonVariant &jsonObj) {
        if (jsonObj.is<JsonObject>() == false) return;
        JsonObject jsonObjItems = jsonObj.as<JsonObject>();
        int itemCount = jsonObjItems.size();
        if (itemCount == 0) return;
        int index = 0;
        for (const JsonPair& keyValue : jsonObjItems) {
            PSC_JsonWriter::kv(mqtt, keyValue);
            if (index < itemCount-1) mqtt.write(',');
            index++;
        }
    }

    void PSC_JsonWriter::kv(PubSubClient& mqtt, const JsonPair& keyValue) {
        const char* keyStr = keyValue.key().c_str();
        mqtt.write('"');
        mqtt.write((uint8_t*)keyStr, keyValue.key().size());
        mqtt.write('"');
        mqtt.write(':');
        PSC_JsonWriter::val(mqtt, keyValue.value());
    }

    void PSC_JsonWriter::val(PubSubClient& mqtt, const JsonVariant& valueObj) {
        if (valueObj.is<const char*>()) {
            const char* valStr = valueObj.as<const char*>();
            mqtt.write('"');
            mqtt.write((uint8_t*)valStr, strlen(valStr));
            mqtt.write('"');
        } else if (valueObj.is<bool>()) {
            bool valBool = valueObj.as<bool>();
            if (valBool)
                mqtt.write((uint8_t*)"true", 4);
            else
                mqtt.write((uint8_t*)"false", 5);
        } else if (valueObj.is<JsonArray>()) {
            // finalize object
            mqtt.write('[');
            const JsonArray& items = valueObj.as<JsonArray>();
            int itemCount = items.size();
            for (int i=0;i<itemCount;i++) {
                PSC_JsonWriter::val(mqtt, items[i]);
                if (i<itemCount-1) mqtt.write(',');
            }
            mqtt.write(']');
        } else if (valueObj.is<JsonObject>()) {
            mqtt.write('{');
            PSC_JsonWriter::SendAllItems(mqtt, valueObj);
            mqtt.write('}');
        } else if (valueObj.isNull()) {
            mqtt.write((uint8_t*)"null", 4);
        } else {
            String tmp;
            serializeJson(valueObj, tmp);
            mqtt.write((uint8_t*)tmp.c_str(), tmp.length());
        }
    }

    void PSC_JsonWriter::kv(PubSubClient& mqtt, const char* key, const char* value) {
        mqtt.write('"');
        mqtt.write((uint8_t*)key, strlen(key));
        mqtt.write('"');
        mqtt.write(':');
        mqtt.write('"');
        mqtt.write((uint8_t*)value, strlen(value));
        mqtt.write('"');
    }

    void PSC_JsonWriter::printf_str(PubSubClient& mqtt, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        const char* segmentStart = fmt;
        while (*fmt) {
            if (*fmt == '%' && *(fmt+1) == 's') {

                // Write literal segment before %s
                int len = fmt-segmentStart;
                if (len > 0) {
                    mqtt.write((const uint8_t*)segmentStart, len);
                }

                // Write argument
                const char* s = va_arg(args, const char*);
                mqtt.write((const uint8_t*)s, strlen(s));

                fmt += 2;              // skip "%s"
                segmentStart = fmt;    // start next literal
            } else {
                fmt++;
            }
        }
        // Write remaining literal text
        int len = fmt-segmentStart;
        if (len > 0) {
            mqtt.write((const uint8_t*)segmentStart, len);
        }

        va_end(args);
    }

    void PSC_JsonWriter::printf_zcstr(PubSubClient& mqtt, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        const char* segmentStart = fmt;
        while (*fmt) {
            if (*fmt == '%' && *(fmt+1) == 's') {

                // Write literal segment before %s
                int len = fmt-segmentStart;
                if (len > 0) {
                    mqtt.write((const uint8_t*)segmentStart, len);
                }

                // Write argument
                ZeroCopyString* s = va_arg(args, ZeroCopyString*);
                mqtt.write((const uint8_t*)s->start, s->Length());

                fmt += 2;              // skip "%s"
                segmentStart = fmt;    // start next literal
            } else {
                fmt++;
            }
        }
        // Write remaining literal text
        int len = fmt-segmentStart;
        if (len > 0) {
            mqtt.write((const uint8_t*)segmentStart, len);
        }

        va_end(args);
    }

    void PSC_JsonWriter::printf_str_indexed(PubSubClient& mqtt, const char* fmt, const char* args[], int argCount) {
        const char* p = fmt;
        const char* segmentStart = fmt;
        if (argCount == 0) {
            for (; args[argCount]; argCount++);
            // no point to continue if there is no parameters
            if (argCount == 0) return;
        }
                
        while (*p) {
            if (*p == '%' && *(p+1) >= '0' && *(p+1) <= '9') {
                // Write literal segment before %s
                int len = p-segmentStart;
                if (len > 0) {
                    mqtt.write((const uint8_t*)segmentStart, len);
                }
                

                int idx = *(p+1) - '0';
                if (idx < argCount) {
                    const char* s = args[idx];
                    mqtt.write((const uint8_t*)s, strlen(s));
                }
                p += 2;
                segmentStart = p;
            } else {
                //mqtt.write((uint8_t)*p);
                p++;
            }
        }
        // Write remaining literal text
        int len = p-segmentStart;
        if (len > 0) {
            mqtt.write((const uint8_t*)segmentStart, len);
        }
    }



} // namespace HAL_JSON
