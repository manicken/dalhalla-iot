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
#include <cstdarg> // variadic

#define JSON(...) #__VA_ARGS__

namespace HAL_JSON
{
    void HA_DeviceDiscovery::StartSendBaseData(const JsonVariant &jsonObj, PubSubClient& mqtt, int packetLength) {
        char topic[128];
        const char* typeStr = GetAsConstChar(jsonObj,"type");
        const char* uidStr = GetAsConstChar(jsonObj,"uid");
        snprintf(topic, sizeof(topic), "homeassistant/%s/%s/config", typeStr, uidStr);
        
        mqtt.beginPublish(topic, packetLength, true);
    }

    void HA_DeviceDiscovery::SendBaseData(const JsonVariant& jsonObj, const JsonVariant& jsonObjDeviceGroup, const char* rootName, PubSubClient& mqtt) {
        
        const char* uidStr = GetAsConstChar(jsonObj,"uid");

        if (jsonObjDeviceGroup.isNull() == false) {
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
        if (ValidateJsonStringField(jsonObj,"icon")){
            PSC_JsonWriter::copyFromJsonObj(mqtt, jsonObj, "icon");
        }
        // availability_topic
        if (jsonObj.containsKey("use_availability_topic")) {
            const char* plAvailableStr = GetValidatedJsonStringField(jsonObj,"payload_available", "online");
            const char* plNotAvailableStr = GetValidatedJsonStringField(jsonObj,"payload_not_available", "offline");

            const char* jsonFmt = JSON(
                "availability_topic":"%s_%s/status",
                "payload_available":"%s",
                "payload_not_available":"%s",
            );
            PSC_JsonWriter::printf_str(mqtt, jsonFmt, rootName, uidStr, plAvailableStr, plNotAvailableStr);
        }
        const char* args[] = { rootName, GetAsConstChar(jsonObj,"type"), uidStr, GetAsConstChar(jsonObj, "name"), nullptr };
        const char* jsonFmt = JSON(
            "state_topic":"%0/%1/%2",
            "unique_id":"%0_%2",
            "name":"%3"
        );
        PSC_JsonWriter::printf_str_indexed(mqtt, jsonFmt, args);
        /*PSC_JsonWriter::printf_str(mqtt, "\"state_topic\":\"%s/%s/%s\",", rootName, typeStr, uidStr);
        PSC_JsonWriter::printf_str(mqtt, "\"unique_id\":\"%s_%s\",", rootName, uidStr);
        PSC_JsonWriter::copyFromJsonObj(mqtt, jsonObj, "name", true);
        */
        // dont write comma here as the caller takes care of that
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

    void PSC_JsonWriter::kv(PubSubClient& mqtt, const char* key, const char* value, bool last/* = false*/) {
        mqtt.write('"');
        mqtt.write((uint8_t*)key, strlen(key));
        mqtt.write('"');
        mqtt.write(':');
        mqtt.write('"');
        mqtt.write((uint8_t*)value, strlen(value));
        mqtt.write('"');
        if (false == last)
            mqtt.write(',');
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
