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
#include <WiFi.h>

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

        const JsonVariant* jsonObjGrp = nullptr;

        if (jsonObj.containsKey("group")) {
            // one global group def
            jsonObjGrp = jsonObj["group"];
            

        } else if (jsonObj.containsKey("groups")) {
            // multiple device discovery group defs
            // that in turn contain the actual items
            bool* validItems = nullptr;
            int validItemCount = 0;
            GetFlattenGroupsValidItems(jsonObj, &validItems, &validItemCount);
            deviceCount = validItemCount;
            devices = new Device*[deviceCount](); // create array and initialize all to nullptr
            int index = 0;
            // go throught all here
            const JsonArray& jsonArrayGroups = jsonObj["groups"];
            int jsonArrayGroupsCount = jsonArrayGroups.size();

            
            delete validItems; // dont forget to delete/free
        }

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
        for (int i=0;i<arrayCount;i++) {
            if (validItems[i] == false) continue;

            const JsonVariant& item = jsonArrayItems[i];
            const char* type = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
            
            const HA_DeviceTypeDef* def = Get_HA_DeviceTypeDef(type);
            
            devices[index++] = def->Create_Function(item, type, mqttClient);
            def->SendDiscovery_Function(mqttClient, item, *jsonObjGrp);
        }
        
    }

    void HomeAssistant::GetFlattenGroupsValidItems(const JsonVariant& jsonObj, bool** validItems, int* validItemCount) {
        int count = 0;
        const JsonArray& jsonArrayGroups = jsonObj["groups"];
        int jsonArrayGroupsCount = jsonArrayGroups.size();
        for (int i=0;i<jsonArrayGroupsCount;i++) {
            count += jsonArrayGroups[i]["items"].size();
        }
        *validItems = new bool[count]();
        int validItemIndex = 0;
        *validItemCount = 0;
        for (int i=0;i<jsonArrayGroupsCount;i++) {
            const JsonArray& jsonArrayItems = jsonArrayGroups[i]["items"];
            int jsonArrayItemsCount = jsonArrayItems.size();
            for (int j=0;j<jsonArrayItemsCount;j++) {
                bool valid = GetFlattenGroupsValidItem(jsonArrayItems[j]);
                if (valid) (*validItemCount)++;
                (*validItems)[validItemIndex++] = valid;
            }
        }
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

    }

    void HomeAssistant::begin() {

    }

    Device* HomeAssistant::findDevice(UIDPath& path) {
        return Device::findInArray(reinterpret_cast<Device**>(devices), deviceCount, path, this);
    }

    
}