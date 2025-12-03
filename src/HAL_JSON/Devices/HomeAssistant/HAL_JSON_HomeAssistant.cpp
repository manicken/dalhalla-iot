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
#include "HAL_JSON_HA_Constants.h"

namespace HAL_JSON {

    bool HomeAssistant::VerifyJSON(const JsonVariant &jsonObj) {
        if (ValidateJsonStringField(jsonObj, "deviceId") == false) { SET_ERR_LOC("HA_ROOT_VJ"); return false; }
        if (ValidateJsonStringField(jsonObj, "host") == false) { SET_ERR_LOC("HA_ROOT_VJ"); return false; }
        if (jsonObj.containsKey("groups")) {
            if (jsonObj["groups"].is<JsonArray>()) return true;
            GlobalLogger.Error(F("HASS cfg groups is not a array"));
        } else if (jsonObj.containsKey("items")) {
            if (jsonObj["items"].is<JsonArray>()) return true;
            GlobalLogger.Error(F("HASS cfg items is not a array"));
        }
        GlobalLogger.Error(F("HASS cfg do not contain either groups or items array"));
        return false;
    }

    Device* HomeAssistant::Create(const JsonVariant &jsonObj, const char* type) {
        return new HomeAssistant(jsonObj, type);
    }
    
    HomeAssistant::HomeAssistant(const JsonVariant &jsonObj, const char* type) : Device(type) {
        devices = nullptr; // ensure that it's set
        deviceCount = 0;
        deviceID = std::string(GetAsConstChar(jsonObj, "deviceId"));

        mqttClient.setClient(wifiClient);
        const char* uidStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);
        //Serial.printf("device UID input:>>>%s<<< output:>>>%s<<<\n", uidStr, decodeUID(uid).c_str());
        if (jsonObj.containsKey("port")) {
            port = GetAsUINT16(jsonObj, "port");
        } else {
            port = HAL_JSON_HOME_ASSISTANT_DEFAULT_PORT;
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
        
        if (jsonObj.containsKey("user"))
            username = GetAsConstChar(jsonObj, "user");
        if (jsonObj.containsKey("pass"))
            password = GetAsConstChar(jsonObj, "pass");

        Connect();

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

    void HomeAssistant::Connect() {
        const char* clientIdFormat_cStr = "%s_%s_%s";
        const char* rootName_cStr = HAL_JSON_DEVICES_HOME_ASSISTANT_ROOTNAME;
        std::string uidStr = decodeUID(uid);
        const char* uid_cStr = uidStr.c_str();
        const char* deviceID_cStr = deviceID.c_str();

        int commandTopicStrLength = snprintf(nullptr, 0, clientIdFormat_cStr, rootName_cStr, deviceID_cStr, uid_cStr) + 1;
        char* clientIdStr = new char[commandTopicStrLength];
        snprintf(clientIdStr, commandTopicStrLength, clientIdFormat_cStr, rootName_cStr, deviceID_cStr, uid_cStr);
        Serial.printf("connecting using clientId:%s\n", clientIdStr);
        if (username.length() != 0 && password.length() != 0) { // the default connect is using CleanSession = true
            if (mqttClient.connect(clientIdStr, username.c_str(), password.c_str()) == false) {
                //Serial.println("ERR - HA MQTT nocould not connect using credentials");
            }
        } else {
            if (mqttClient.connect(clientIdStr) == false) { // the default connect is using CleanSession = true
                //Serial.println("ERROR - HASS MQTT could not connect without credentials");
            } 
        }
        delete[] clientIdStr;
        if (mqttClient.connected()) {
            Serial.println("HASS - MQTT connected to brooker");
            SubscribeToCommandTopic();
        }
    }

    void HomeAssistant::SubscribeToCommandTopic() {
        
        const char* cmdTopicFormat_cStr = "%s/%s/+/%s";
        const char* rootName_cStr = HAL_JSON_DEVICES_HOME_ASSISTANT_ROOTNAME;
        const char* cmdName_cStr = HAL_JSON_HOME_ASSISTANT_TOPICBASEPATH_COMMAND;
        const char* deviceID_cStr = deviceID.c_str();
        int commandTopicStrLength = snprintf(nullptr, 0, cmdTopicFormat_cStr, rootName_cStr, deviceID_cStr, cmdName_cStr) + 1;
        char* commandTopicStr = new char[commandTopicStrLength];
        snprintf(commandTopicStr, commandTopicStrLength, cmdTopicFormat_cStr, rootName_cStr, deviceID_cStr, cmdName_cStr);
        mqttClient.subscribe(commandTopicStr);
        delete[] commandTopicStr;
    }

    void HomeAssistant::mqttCallback(char* topic, byte* payload, unsigned int length) {
        Serial.printf("\r\nmqttCallback\r\ntopic:%s\r\n payload:%.*s\r\n", topic, (int)length, (char*)payload);

        // wrap in a ZeroCopyString for neat functions
        ZeroCopyString zcTopic(topic);

        // find the lastSlash
        const char* lastSlash = zcTopic.FindCharReverse('/');
        if (lastSlash == nullptr) { Serial.println("last slash not found"); return; }

        // split of the tail
        ZeroCopyString zcTopicTail = zcTopic.SplitOffTail(lastSlash);

        const TopicSuffix cmdTopic = GetTopicSuffix(TopicBasePathMode::Command);
        if (zcTopicTail != cmdTopic.str) { Serial.printf("\nis not command:%s\n", zcTopicTail.ToString().c_str()); return; } // not command

        // find second-to-last slash
        const char* secondLastSlash = zcTopic.FindCharReverse('/');
        if (secondLastSlash == nullptr) { Serial.println("second last slash not found"); return; }

        ZeroCopyString zcUID = zcTopic.SplitOffTail(secondLastSlash);
        //std::string uidDbgStr = zcUID.ToString();
        //Serial.printf("\r\nextracted UID:>>>%s<<<\r\n", uidDbgStr.c_str());
        // encode as uid for fast lockup
        HAL_UID uid = encodeUID(zcUID);

        // Iterate over all devices
        for (int i = 0; i < deviceCount; i++) {
            Device* dev = devices[i];
            if (uid != dev->uid) continue;

            ZeroCopyString zcCmdStr((char*)payload, (char*)payload+length);
            dev->exec(zcCmdStr);
            //dev->HandleCommand(zcCmdStr);
            return;  // found a match
        }
        Serial.println("device not found");
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
            
            const char* type_cStr = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
            
            const HA_DeviceTypeDef* def = Get_HA_DeviceTypeDef(type_cStr);
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
            const char* type_cStr = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
            
            const HA_DeviceTypeDef* def = Get_HA_DeviceTypeDef(type_cStr);
            devices[index++] = def->Create_Function(item, def->typeName, mqttClient, groupObj, jsonObj);
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
                const char* type_cStr = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
        
                const HA_DeviceTypeDef* def = Get_HA_DeviceTypeDef(type_cStr);

                devices[newItemIndex++] = def->Create_Function(item, def->typeName, mqttClient, jsonObjGrpItem, jsonObj);
            
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
        
        const char* type_cStr = GetAsConstChar(jsonObjItem, HAL_JSON_KEYNAME_TYPE);
        const HA_DeviceTypeDef* def = Get_HA_DeviceTypeDef(type_cStr);
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

    String HomeAssistant::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->type;
        ret += "\",\"items\":[";
        for (int i=0;i<deviceCount;i++) {
            ret += '{';
            ret += devices[i]->ToString();
            ret += '}';
            if (i<deviceCount-1) ret += ',';
        }
        ret += ']';
       
        return ret;
    }

    void HomeAssistant::loop() {
        if (!devices) return;

        if (mqttClient.connected() == false) {
            
            unsigned long now = millis();
            if (now - lastReconnectAttempt >= reconnectInterval) {
                lastReconnectAttempt = now;
                Serial.println("HASS - MQTT connection is down, trying to reconnect");
                Connect();
            }
            return;
        }
        mqttClient.loop();

        for (int i=0;i<deviceCount;i++) {
            devices[i]->loop();
        }

    }

    void HomeAssistant::begin() {
        if (!devices) return;
        for (int i=0;i<deviceCount;i++) {
            devices[i]->begin();
        }
    }

    DeviceFindResult HomeAssistant::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(devices, deviceCount, path, this, outDevice);
    }

    
}