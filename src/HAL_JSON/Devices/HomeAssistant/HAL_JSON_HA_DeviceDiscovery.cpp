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
            const char* deviceGroupUID = jsonObjDeviceGroup["uid"];
            PSC_JsonWriter::key(mqtt, "identifiers");
            mqtt.write('[');
            mqtt.write('"');
            mqtt.write((uint8_t*)deviceGroupUID, strlen(deviceGroupUID));
            mqtt.write('"');
            mqtt.write(']');
            mqtt.write(',');
            PSC_JsonWriter::kv(mqtt, "manufacturer", "Dalhal"); // hardcode for now
            PSC_JsonWriter::kv(mqtt, "model", "Virtual Sensor"); // hardcode for now
            PSC_JsonWriter::kv(mqtt, "name", jsonObjDeviceGroup["name"], true); // true == last item
            mqtt.write('}');
            mqtt.write(',');
        }

        if (ValidateJsonStringField(jsonObj,"icon")){
            const char* iconStr = GetAsConstChar(jsonObj, "icon");
            PSC_JsonWriter::kv(mqtt, "icon", iconStr);
        }
        
        // state_topic
        PSC_JsonWriter::key(mqtt, "state_topic");
        mqtt.write('"');
        mqtt.write((uint8_t*)rootName, strlen(rootName));
        mqtt.write('/');
        const char* typeStr = jsonObj["type"];
        mqtt.write((uint8_t*)typeStr, strlen(typeStr));
        mqtt.write('/');
        const char* uidStr = jsonObj["uid"];
        mqtt.write((uint8_t*)uidStr, strlen(uidStr));
        mqtt.write('"');
        mqtt.write(',');
        // unique_id
        PSC_JsonWriter::key(mqtt, "unique_id");
        mqtt.write('"');
        mqtt.write((uint8_t*)rootName, strlen(rootName));
        mqtt.write('_');
        mqtt.write((uint8_t*)uidStr, strlen(uidStr));
        mqtt.write('"');
        mqtt.write(',');
        // device_class only used by sensors
        const char* device_class = jsonObj["device_class"];
        if (device_class != nullptr) {
            PSC_JsonWriter::kv(mqtt, "device_class", device_class);
        }
        // availability_topic
        if (jsonObj.containsKey("use_availability_topic")) {
            
            PSC_JsonWriter::key(mqtt, "availability_topic");
            mqtt.write('"');
            mqtt.write((uint8_t*)rootName, strlen(rootName));
            mqtt.write('_');
            mqtt.write((uint8_t*)uidStr, strlen(uidStr));
            mqtt.write((uint8_t*)"/status", 8);
            mqtt.write('"');
            mqtt.write(',');
            if (ValidateJsonStringField(jsonObj,"payload_available")) {
                const char* paStr = GetAsConstChar(jsonObj, "payload_available");
                PSC_JsonWriter::kv(mqtt, "payload_available", paStr);
            }
            else {
                PSC_JsonWriter::kv(mqtt, "payload_available", "online"); // default value
            }
            if (ValidateJsonStringField(jsonObj,"payload_not_available")) {
                const char* paStr = GetAsConstChar(jsonObj, "payload_not_available");
                PSC_JsonWriter::kv(mqtt, "payload_not_available", paStr);
            }
            else {
                PSC_JsonWriter::kv(mqtt, "payload_not_available", "offline"); // default value
            }
        }

        PSC_JsonWriter::kv(mqtt, "name", jsonObj["name"], true); // true == dont add comma here
        // dont write comma here as the caller takes care of that
    }

    void PSC_JsonWriter::key(PubSubClient& mqtt, const char* key) {
        mqtt.write('"');
        mqtt.write((uint8_t*)key, strlen(key));
        mqtt.write('"');
        mqtt.write(':');
    }

    void PSC_JsonWriter::kv(PubSubClient& mqtt, const char* key, const char* value, bool last = false) {
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

} // namespace HAL_JSON
