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

#include "HAL_JSON_HomeAssistant.h"
#include "HAL_JSON_HA_DeviceTypeReg.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"
#include "HAL_JSON_HA_TopicBasePath.h"
#include <WiFi.h>
#include <Arduino.h>

namespace HAL_JSON {
    
    HomeAssistant::HomeAssistant(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        devices = nullptr; // ensure that it's set
        deviceCount = 0;

        mqttClient.setClient(wifiClient);
        const char* uidStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);
        if (jsonObj.containsKey("port")) {
            port = GetAsUINT16(jsonObj, "port");
        } else {
            port = 1883;
        }

        const char* hostStr = GetAsConstChar(jsonObj, "host");
        host = std::string(hostStr);
        if (WiFi.hostByName(host.c_str(), ip)) {
            // Successfully resolved
            mqttClient.setServer(ip, port);
        } else {
            Serial.printf("Failed to resolve %s, will retry later\n", host.c_str());
            // Optionally, store host string for next retry attempt
            mqttClient.setServer(host.c_str(), port); // last-resort attempt
            // could also have retry when doing automatic reconnect
        }

        clientId = "dalhal_" + std::string(uidStr) + "_" + std::to_string(millis() & 0xFFFF);

        if (jsonObj.containsKey("user"))
            username = GetAsConstChar(jsonObj, "user");
        if (jsonObj.containsKey("pass"))
            password = GetAsConstChar(jsonObj, "pass");

        // connect directly here so that device discovery can use it
        if (username.length() != 0 && password.length() != 0) {
            mqttClient.connect(clientId.c_str(), username.c_str(), password.c_str());
        } else {
            mqttClient.connect(clientId.c_str());
        }

        if (jsonObj.containsKey("groups")) {
            ConstructDevicesFromFlattenGroupsItems(jsonObj);
            // TODO add support for non flatten group items
            // so that group uid:s can be included in the addressing scheme
        } else {
            ConstructDevicesNonGrouped(jsonObj);
        }
        
        mqttClient.setCallback([this](char* t, byte* p, unsigned int l){
            this->mqttCallback(t, p, l);
        });
    }

    void HomeAssistant::mqttCallback(char* topic, byte* payload, unsigned int length) {

        const char* lastSlash = strrchr(topic, '/');
        if (!lastSlash) return;
        const TopicSuffix cmdTopic = GetTopicSuffix(TopicBasePathMode::Command);
        if (strcmp(lastSlash + 1, cmdTopic.str) != 0) return; // not command

        // find second-to-last slash
        const char* secondLastSlash = lastSlash;
        while (secondLastSlash > topic && *secondLastSlash != '/') secondLastSlash--;
        if (*secondLastSlash != '/') return;

        ZeroCopyString zcUID(secondLastSlash + 1, lastSlash);
        // encode as uid for fast lockup
        HAL_UID uid = encodeUID(zcUID);

        // Iterate over all devices
        for (int i = 0; i < deviceCount; i++) {
            Device* dev = devices[i];
            if (uid != dev->uid) continue;

            ZeroCopyString zcCmdStr((char*)payload, (char*)payload+length);
            dev->exec(zcCmdStr);
            //dev->HandleCommand(zcCmdStr);
            break;  // found a match
        }
    }

    void HomeAssistant::ConstructDevicesNonGrouped(const JsonVariant& jsonObj) {
        const JsonArray& jsonArrayItems = jsonObj["items"];
        int arrayCount = jsonArrayItems.size();
        bool* validItems = new bool[arrayCount];
        int validItemCount = 0;
        // first pass count and check valid items
        for (int i=0;i<arrayCount;i++) {
            const JsonVariant& item = jsonArrayItems[i];
            if (IsConstChar(item) == true) { validItems[i] = false; continue; }// comment item
            if (Device::DisabledInJson(item) == true) { validItems[i] = false; continue; } // disabled
            if (ValidateJsonStringField(item, HAL_JSON_KEYNAME_TYPE) == false) { validItems[i] = false; continue; }
            
            const char* type = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
            
            const HA_DeviceTypeDef* def = Get_HA_DeviceTypeDef(type);
            // no nullcheck is needed as ValidateJSON ensures that all types are correct
            if (def->Verify_JSON_Function(item) == false) { validItems[i] = false; continue; }
            validItemCount++;
            validItems[i] = true;
        }
        deviceCount = validItemCount;
        devices = new Device*[deviceCount](); // create array and initialize all to nullptr
        int index = 0;
        // second pass create devices
        const JsonVariant& groupObj = jsonObj["group"];
        for (int i=0;i<arrayCount;i++) {
            if (validItems[i] == false) continue;

            const JsonVariant& item = jsonArrayItems[i];
            const char* type = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
            
            const HA_DeviceTypeDef* def = Get_HA_DeviceTypeDef(type);
            devices[index++] = def->Create_Function(item, type, mqttClient, groupObj, jsonObj);
        }
        delete[] validItems;
    }

    void HomeAssistant::ConstructDevicesFromFlattenGroupsItems(const JsonVariant& jsonObj) {
        int count = 0;
        const JsonArray& jsonArrayGroups = jsonObj["groups"];
        int jsonArrayGroupsCount = jsonArrayGroups.size();
        for (int i=0;i<jsonArrayGroupsCount;i++) {
            count += jsonArrayGroups[i]["items"].size();
        }
        bool* validItems = new bool[count]();
        int validItemIndex = 0;
        int validItemCount = 0;
        // first pass count and mark valid items
        for (int i=0;i<jsonArrayGroupsCount;i++) {
            const JsonArray& jsonArrayItems = jsonArrayGroups[i]["items"];
            int jsonArrayItemsCount = jsonArrayItems.size();
            for (int j=0;j<jsonArrayItemsCount;j++) {
                bool valid = GetFlattenGroupsValidItem(jsonArrayItems[j]);
                if (valid) validItemCount++;
                validItems[validItemIndex++] = valid;
            }
        }
        deviceCount = validItemCount;
        devices = new Device*[deviceCount]();
        validItemIndex = 0;
        int newItemIndex = 0;
        for (int i=0;i<jsonArrayGroupsCount;i++) {
            const JsonVariant& jsonObjGrpItem = jsonArrayGroups[i];
            const JsonArray& jsonArrayItems = jsonObjGrpItem["items"];
            int jsonArrayItemsCount = jsonArrayItems.size();
            for (int j=0;j<jsonArrayItemsCount;j++) {
                if (validItems[validItemIndex++] == false) continue;

                const JsonVariant& item = jsonArrayItems[j];
                const char* type = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
        
                const HA_DeviceTypeDef* def = Get_HA_DeviceTypeDef(type);

                devices[newItemIndex++] = def->Create_Function(item, type, mqttClient, jsonObjGrpItem, jsonObj);
            
            }
        }
        delete[] validItems;
    }

    bool HomeAssistant::GetFlattenGroupsValidItem(const JsonVariant& jsonObjItem) {
        // check if comment
        if (IsConstChar(jsonObjItem) == true) { return false; }
        // check if disabled
        if (Device::DisabledInJson(jsonObjItem) == true) { return false; }
        // check if type is a valid string
        if (ValidateJsonStringField(jsonObjItem, HAL_JSON_KEYNAME_TYPE) == false) { return false; }
        
        const char* type = GetAsConstChar(jsonObjItem, HAL_JSON_KEYNAME_TYPE);
        const HA_DeviceTypeDef* def = Get_HA_DeviceTypeDef(type);
        // no nullcheck is needed as ValidateJSON ensures that all types are correct
        if (def->Verify_JSON_Function(jsonObjItem) == false) { return false; }
        return true;
    }

    HomeAssistant::~HomeAssistant() {
        if (devices) {
            for (int i=0;i<deviceCount;i++) {
                delete devices[i];
            }
            delete[] devices;
        }
    }

    bool HomeAssistant::VerifyJSON(const JsonVariant &jsonObj) {

        return true;
    }

    Device* HomeAssistant::Create(const JsonVariant &jsonObj, const char* type) {
        return new HomeAssistant(jsonObj, type);
    }

    String HomeAssistant::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
       
        return ret;
    }

    void HomeAssistant::loop() {
        if (!devices) return;

        if (mqttClient.connected() == false) {
            unsigned long now = millis();
            if (now - lastReconnectAttempt >= reconnectInterval) {
                lastReconnectAttempt = now;
                bool connected = false;
                if (username.length() != 0 && password.length() != 0) {
                    connected = mqttClient.connect(clientId.c_str(), username.c_str(), password.c_str());
                } else {
                    connected = mqttClient.connect(clientId.c_str());
                }
                if (!connected) {
                    Serial.println("MQTT reconnect failed, will retry...");
                    return;
                }
            } else { 
                return;  // not enough time passed since last attempt
            }
        }

        for (int i=0;i<deviceCount;i++) {
            devices[i]->loop();
        }

    }

    void HomeAssistant::begin() {

    }

    Device* HomeAssistant::findDevice(UIDPath& path) {
        return Device::findInArray(reinterpret_cast<Device**>(devices), deviceCount, path, this);
    }

    
}