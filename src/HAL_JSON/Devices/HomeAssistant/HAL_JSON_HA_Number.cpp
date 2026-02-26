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

#include "HAL_JSON_HA_Number.h"
#include "HAL_JSON_HA_DeviceDiscovery.h"
#include "HAL_JSON_HA_CountingPubSubClient.h"
#include "HAL_JSON_HA_Constants.h"

#include "../../Support/HAL_JSON_Logger.h"
#include "../../Core/HAL_JSON_JSON_Config_Defines.h"
#include "../../Support/HAL_JSON_ArduinoJSON_ext.h"

namespace HAL_JSON {

    void Number::SendDeviceDiscovery(PubSubClient& mqtt, const JsonVariant& jsonObj, TopicBasePath& topicBasePath) {
        const char* cmdTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::Command);
        PSC_JsonWriter::printf_str(mqtt, JSON(,"command_topic":"%s"), cmdTopicStr);
        const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
        PSC_JsonWriter::printf_str(mqtt, JSON(,"state_topic":"%s"), stateTopicStr);
    }
    
    Number::Number(const JsonVariant &jsonObj, const char* type_cStr, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) : mqttClient(mqttClient), Device(type_cStr) {
        const char* uidStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);
        const char* deviceId_cStr = jsonObjRoot["deviceId"];

          
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

        HA_DeviceDiscovery::SendDiscovery(mqttClient, deviceId_cStr, type_cStr, uidStr, jsonObj, jsonObjGlobal, topicBasePath, Number::SendDeviceDiscovery);
        
    }
    Number::~Number() {
        delete cda;
    }

    bool Number::VerifyJSON(const JsonVariant &jsonObj) {
        if (ValidateJsonStringField(jsonObj, "uid") == false) { SET_ERR_LOC("HA_SENSOR_VJ"); return false; }
        if (ValidateJsonStringField(jsonObj, "name") == false) { SET_ERR_LOC("HA_SENSOR_VJ"); return false; }
        if (ValidateJsonStringField(jsonObj, "target")) {
            ZeroCopyString zcSrcDeviceUidStr = GetAsConstChar(jsonObj, "target");
            CachedDeviceAccess cdaTmp;
            if (cdaTmp.Set(zcSrcDeviceUidStr) == false) {
                SET_ERR_LOC("HA_SENSOR_VJ");
                return false;
            }
        }
        return true;
    }

    Device* Number::Create(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) {
        return new Number(jsonObj, type, mqttClient, jsonObjGlobal, jsonObjRoot);
    }

    String Number::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
       
        return ret;
    }

    HALOperationResult Number::read(HALValue& val) {
        if (cda != nullptr) {
            return cda->ReadSimple(val);
        } else {
            val = currentValue;
            return HALOperationResult::Success;
        }
    }
    HALOperationResult Number::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;

        currentValue = val;
        sendCurrentValue();

        if (cda != nullptr) {
            return cda->WriteSimple(val);
        }
        return HALOperationResult::UnsupportedOperation;
    };

    HALOperationResult Number::exec(const ZeroCopyString& cmd) {

        NumberResult numberRes = cmd.ConvertStringToNumber();     
        HALValue valState;
        switch (numberRes.type) {
            case NumberType::UINT32:
                Serial.println("val type was uint");
                valState = numberRes.u32;
                break;
            case NumberType::INT32:
                Serial.println("val type was int");
                valState = numberRes.i32;
                break;
            case NumberType::FLOAT:
                Serial.println("val type was float");
                valState = numberRes.f32;
                break;
            default:
                // send back old value on fail
                sendCurrentValue();
                return HALOperationResult::WriteValueNaN;
        }

        HALOperationResult res = (cda!=nullptr)?cda->WriteSimple(valState):HALOperationResult::Success;
        if (res == HALOperationResult::Success) {
            //Serial.println("Number exec OK");
            currentValue = valState;
        } else {
            Serial.println("Number exec fail");
        }
        sendCurrentValue();
        return res;
        
    }

    void Number::sendCurrentValue() {
        const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
        mqttClient.publish(stateTopicStr, currentValue.toString().c_str());
    }

}