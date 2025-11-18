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
        
        if (jsonObjDeviceGroup.isNull() == false) {
            PSC_JsonWriter::key(mqtt, "device");
            mqtt.write('{');
            const char* deviceGroupUID = GetAsConstChar(jsonObjDeviceGroup,"uid");
            PSC_JsonWriter::key(mqtt, "identifiers");
            mqtt.write('[');
            mqtt.write('"');
            mqtt.write((uint8_t*)deviceGroupUID, strlen(deviceGroupUID));
            mqtt.write('"');
            mqtt.write(']');
            mqtt.write(',');
            PSC_JsonWriter::kv(mqtt, "manufacturer", "Dalhal"); // hardcode for now
            PSC_JsonWriter::kv(mqtt, "model", "Virtual Sensor"); // hardcode for now
            PSC_JsonWriter::copyFromJsonObj(mqtt, jsonObjDeviceGroup, "name", true); // true == last item
            mqtt.write('}');
            mqtt.write(',');
        }

        if (ValidateJsonStringField(jsonObj,"icon")){
            PSC_JsonWriter::copyFromJsonObj(mqtt, jsonObj, "icon");
        }
        const char* typeStr = GetAsConstChar(jsonObj,"type");
        const char* uidStr = GetAsConstChar(jsonObj,"uid");
        
        // state_topic
        psc_printf_s(mqtt, "\"state_topic\":\"%s/%s/%s\",", rootName, uidStr);
        PSC_JsonWriter::key(mqtt, "state_topic");
        mqtt.write('"');
        mqtt.write((uint8_t*)rootName, strlen(rootName));
        mqtt.write('/');
        
        mqtt.write((uint8_t*)typeStr, strlen(typeStr));
        mqtt.write('/');
        
        mqtt.write((uint8_t*)uidStr, strlen(uidStr));
        mqtt.write('"');
        mqtt.write(',');
        // unique_id
        psc_printf_s(mqtt, "\"unique_id\":\"%s_%s\",", rootName, uidStr);
        /*PSC_JsonWriter::key(mqtt, "unique_id");
        mqtt.write('"');
        mqtt.write((uint8_t*)rootName, strlen(rootName));
        mqtt.write('_');
        mqtt.write((uint8_t*)uidStr, strlen(uidStr));
        mqtt.write('"');
        mqtt.write(',');
        */
        // availability_topic
        if (jsonObj.containsKey("use_availability_topic")) {
            
            psc_printf_s(mqtt, "\"availability_topic\":\"%s_%s/status\",", rootName, uidStr);
            /*
            PSC_JsonWriter::key(mqtt, "availability_topic");
            mqtt.write('"');
            mqtt.write((uint8_t*)rootName, strlen(rootName));
            mqtt.write('_');
            mqtt.write((uint8_t*)uidStr, strlen(uidStr));
            mqtt.write((uint8_t*)"/status", 8);
            mqtt.write('"');
            mqtt.write(',');
            */
            if (ValidateJsonStringField(jsonObj,"payload_available")) {
                PSC_JsonWriter::copyFromJsonObj(mqtt, jsonObj, "payload_available");
            }
            else {
                PSC_JsonWriter::kv(mqtt, "payload_available", "online"); // default value
            }
            if (ValidateJsonStringField(jsonObj,"payload_not_available")) {
                PSC_JsonWriter::copyFromJsonObj(mqtt, jsonObj, "payload_not_available");
            }
            else {
                PSC_JsonWriter::kv(mqtt, "payload_not_available", "offline"); // default value
            }
        }
        const char* nameStr = GetAsConstChar(jsonObj, "name");
        PSC_JsonWriter::kv(mqtt, "name", nameStr, true); // true == dont add comma here
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

    void psc_printf_s(PubSubClient& mqtt, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);

        while (*fmt) {
            if (*fmt == '%' && *(fmt+1) == 's') {
                const char* s = va_arg(args, const char*);
                mqtt.write((const uint8_t*)s, strlen(s));
                fmt += 2;
            } else {
                mqtt.write((uint8_t)*fmt);
                fmt++;
            }
        }

        va_end(args);
    }


} // namespace HAL_JSON
