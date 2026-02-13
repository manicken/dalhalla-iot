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

#include "HAL_JSON_HA_Switch.h"
#include "HAL_JSON_HA_DeviceDiscovery.h"
#include "HAL_JSON_HA_CountingPubSubClient.h"
#include "HAL_JSON_HA_Constants.h"


namespace HAL_JSON {

    const char* Switch::PAYLOAD_OFF = "OFF";
    const char* Switch::PAYLOAD_ON = "ON";

    void Switch::SendDeviceDiscovery(PubSubClient& mqtt, const JsonVariant& jsonObj, TopicBasePath& topicBasePath) {
        const char* cmdTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::Command);
        PSC_JsonWriter::printf_str(mqtt, JSON(,"command_topic":"%s"), cmdTopicStr);
        PSC_JsonWriter::printf_str(mqtt, JSON(,"payload_on":"%s"), Switch::PAYLOAD_ON);
        PSC_JsonWriter::printf_str(mqtt, JSON(,"payload_off":"%s"), Switch::PAYLOAD_OFF);
        const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
        PSC_JsonWriter::printf_str(mqtt, JSON(,"state_topic":"%s"), stateTopicStr);
    }
    
    Switch::Switch(const JsonVariant &jsonObj, const char* type_cStr, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) : mqttClient(mqttClient), Device(type_cStr) {
        const char* uidStr = GetAsConstChar(jsonObj, "uid");
        uid = encodeUID(uidStr);
        const char* deviceId_cStr = jsonObjRoot["deviceId"];

        if (jsonObj["momentary"].is<bool>()) { // do return false if key not found
            momentary = jsonObj["momentary"].as<bool>();
        } else {
            momentary = false;
        }
  
        topicBasePath.Set(deviceId_cStr, uidStr);

        if (ValidateJsonStringField(jsonObj, "target")) {
            ZeroCopyString zcSrcDeviceUidStr = GetAsConstChar(jsonObj, "target");
            cda = new CachedDeviceAccess();
            if (cda->Set(zcSrcDeviceUidStr) == false) {
                delete cda;
                cda = nullptr;
            }
        } else {
            cda = nullptr;
        }
        //refreshMs = ParseRefreshTimeMs(jsonObj, 5000);

        //const char* cfgTopic_cStr = HA_DeviceDiscovery::GetDiscoveryCfgTopic(deviceId_cStr, type, uidStr);
        HA_DeviceDiscovery::SendDiscovery(mqttClient, deviceId_cStr, type_cStr, uidStr, jsonObj, jsonObjGlobal, topicBasePath, Switch::SendDeviceDiscovery);
        //delete[] cfgTopic_cStr;
    }
    Switch::~Switch() {
        delete cda;
    }

    bool Switch::VerifyJSON(const JsonVariant &jsonObj) {
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

    Device* Switch::Create(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) {
        return new Switch(jsonObj, type, mqttClient, jsonObjGlobal, jsonObjRoot);
    }

    String Switch::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
       
        return ret;
    }

    HALOperationResult Switch::read(HALValue& val) {
        if (cda != nullptr) {
            return cda->ReadSimple(val);
        }
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Switch::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;


        const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
        if (val.asUInt() == 0) {
            mqttClient.publish(stateTopicStr, Switch::PAYLOAD_OFF);
        } else {
            mqttClient.publish(stateTopicStr, Switch::PAYLOAD_ON);
        }
        if (cda != nullptr) {
            return cda->WriteSimple(val);
        }
        return HALOperationResult::UnsupportedOperation;
    };

    HALOperationResult Switch::exec(const ZeroCopyString& cmd) {
        if (momentary == false) {
            HALValue valState;
            HALOperationResult res = HALOperationResult::NotSet;
            const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
            if (cmd == Switch::PAYLOAD_ON) {
                valState.set((uint32_t)1);
            } else if (cmd == Switch::PAYLOAD_OFF) {
                valState.set((uint32_t)0);
            } else {
                return HALOperationResult::UnsupportedCommand; // or some error code
            }
            res = cda->WriteSimple(valState);
            if (res == HALOperationResult::Success) {
                Serial.println("switch exec OK");
                if (valState.asUInt() == 0)
                    mqttClient.publish(stateTopicStr, Switch::PAYLOAD_OFF);
                else
                    mqttClient.publish(stateTopicStr, Switch::PAYLOAD_ON);
            } else {
                Serial.println("switch exec fail");
            }
            return res;
        } else {
            const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
            mqttClient.publish(stateTopicStr, Switch::PAYLOAD_OFF);
            return cda->Exec();
        }
    }

}