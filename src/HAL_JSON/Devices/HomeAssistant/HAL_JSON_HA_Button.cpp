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

#include "HAL_JSON_HA_Button.h"
#include "HAL_JSON_HA_DeviceDiscovery.h"
#include "HAL_JSON_HA_CountingPubSubClient.h"
#include "HAL_JSON_HA_Constants.h"


namespace HAL_JSON {

    void Button::SendDeviceDiscovery(PubSubClient& mqtt, const JsonVariant& jsonObj, TopicBasePath& topicBasePath) {
        const char* cmdTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::Command);
        PSC_JsonWriter::printf_str(mqtt, JSON(,"command_topic":"%s"), cmdTopicStr);
    }
    
    Button::Button(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) : mqttClient(mqttClient), Device(UIDPathMaxLength::One,type) {
        const char* uidStr = GetAsConstChar(jsonObj, "uid");
        uid = encodeUID(uidStr);
        const char* deviceId_cStr = jsonObjRoot["deviceId"];
  
        topicBasePath.Set(deviceId_cStr, uidStr);

        if (ValidateJsonStringField(jsonObj, "target")) {
            ZeroCopyString zcSrcDeviceUidStr = GetAsConstChar(jsonObj, "target");
            cda = new CachedDeviceAccess(zcSrcDeviceUidStr);
            /*if (cda->Set(zcSrcDeviceUidStr) == false) {
                delete cdr;
                cdr = nullptr;
            }*/
        } else {
            cda = nullptr;
        }
        //refreshMs = ParseRefreshTimeMs(jsonObj, 5000);

        const char* cfgTopic_cStr = HA_DeviceDiscovery::GetDiscoveryCfgTopic(deviceId_cStr, type, uidStr);
        HA_DeviceDiscovery::SendDiscovery(mqttClient, deviceId_cStr, cfgTopic_cStr, jsonObj, jsonObjGlobal, topicBasePath, Button::SendDeviceDiscovery);
        delete[] cfgTopic_cStr;

        //lastMs = millis()-refreshMs; // force a direct update after start
    }
    Button::~Button() {
        delete cda;
    }

    bool Button::VerifyJSON(const JsonVariant &jsonObj) {
        if (ValidateJsonStringField(jsonObj, "uid") == false) { SET_ERR_LOC("HA_SENSOR_VJ"); return false; }
        if (ValidateJsonStringField(jsonObj, "name") == false) { SET_ERR_LOC("HA_SENSOR_VJ"); return false; }
        if (ValidateJsonStringField(jsonObj, "target")) {
            //ZeroCopyString zcSrcDeviceUidStr = GetAsConstChar(jsonObj, "target");
            /*CachedDeviceAccess cdaTmp;
            if (cdaTmp.Set(zcSrcDeviceUidStr) == false) {
                SET_ERR_LOC("HA_SENSOR_VJ");
                return false;
            }*/
        }
        return true;
    }

    Device* Button::Create(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) {
        return new Button(jsonObj, type, mqttClient, jsonObjGlobal, jsonObjRoot);
    }

    String Button::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
       
        return ret;
    }

    HALOperationResult Button::exec(const ZeroCopyString& cmd) {
        return cda->Exec();
    }

    HALOperationResult Button::exec() {
        return cda->Exec();
    }

}