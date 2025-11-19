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

#include "HAL_JSON_HA_Sensor.h"
#include "HAL_JSON_HA_DeviceDiscovery.h"
#include "HAL_JSON_HA_CountingPubSubClient.h"

namespace HAL_JSON {

    void Sensor::SendSpecificDeviceDiscovery(PubSubClient& mqttClient, const JsonVariant& jsonObj) {
        mqttClient.write(',');
        PSC_JsonWriter::copyFromJsonObj(mqttClient, jsonObj, "device_class");
        PSC_JsonWriter::copyFromJsonObj(mqttClient, jsonObj, "unit_of_measurement", true);
    }

    void Sensor::SendDeviceDiscovery(PubSubClient& mqttClient, const JsonVariant& jsonObj, const JsonVariant& jsonObjGlobal) {
        CountingPubSubClient dryRunPSC;

        HA_DeviceDiscovery::SendBaseData(jsonObj, jsonObjGlobal, "dalhal", dryRunPSC);
        SendSpecificDeviceDiscovery(dryRunPSC, jsonObj);
        // here can additional data be sent

        HA_DeviceDiscovery::StartSendBaseData(jsonObj, mqttClient, dryRunPSC.count);
        HA_DeviceDiscovery::SendBaseData(jsonObj, jsonObjGlobal, "dalhal", mqttClient);
        SendSpecificDeviceDiscovery(mqttClient, jsonObj);

        // here can additional data be sent

        mqttClient.endPublish();
    }
    
    Sensor::Sensor(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal) : mqttClient(mqttClient), Device(UIDPathMaxLength::One,type) {
        const char* uidStr = GetAsConstChar(jsonObj, "uid");
        uid = encodeUID(uidStr);
        topic.reserve(sizeof("dalhal/sensor/") + strlen(uidStr));
        topic = "dalhal/sensor/" + std::string(uidStr);
        
        if (ValidateJsonStringField(jsonObj, "source")) {
            ZeroCopyString zcSrcDeviceUidStr = GetAsConstChar(jsonObj, "source");
            cdr = new CachedDeviceRead();
            if (cdr->Set(zcSrcDeviceUidStr) == false) {
                delete cdr;
                cdr = nullptr;
            }
        } else {
            cdr = nullptr;
        }
        SendDeviceDiscovery(mqttClient, jsonObj, jsonObjGlobal);
    }
    Sensor::~Sensor() {
        
    }

    bool Sensor::VerifyJSON(const JsonVariant &jsonObj) {
        if (ValidateJsonStringField(jsonObj, "uid") == false) { SET_ERR_LOC("HA_SENSOR_VJ"); return false; }
        if (ValidateJsonStringField(jsonObj, "device_class") == false) { SET_ERR_LOC("HA_SENSOR_VJ"); return false; }
        if (ValidateJsonStringField(jsonObj, "unit_of_measurement") == false) { SET_ERR_LOC("HA_SENSOR_VJ"); return false; }
        if (ValidateJsonStringField(jsonObj, "source")) {
            ZeroCopyString zcSrcDeviceUidStr = GetAsConstChar(jsonObj, "source");
            CachedDeviceRead cdr;
            if (cdr.Set(zcSrcDeviceUidStr) == false) {
                SET_ERR_LOC("HA_SENSOR_VJ");
                return false;
            }
        }
        return true;
    }

    Device* Sensor::Create(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal) {
        return new Sensor(jsonObj, type, mqttClient, jsonObjGlobal);
    }

    String Sensor::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
       
        return ret;
    }

    void Sensor::loop() {
        if (cdr != nullptr) {
            HALValue val;
            cdr->ReadSimple(val);
            mqttClient.publish(topic.c_str(), val.toString().c_str());
        }
    }
    void Sensor::begin() {

    }

    HALOperationResult Sensor::read(HALValue& val) {
        if (cdr != nullptr) {
            cdr->ReadSimple(val);
            return HALOperationResult::Success;
        }
        return HALOperationResult::UnsupportedOperation;

    }
    HALOperationResult Sensor::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;

        mqttClient.publish(topic.c_str(), val.toString().c_str());
        return HALOperationResult::Success;
    };
    
}