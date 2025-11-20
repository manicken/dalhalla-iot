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


    void Sensor::SendDeviceDiscovery(PubSubClient& mqttClient, const JsonVariant& jsonObj, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) {
        // first dry run to calculate payload size
        CountingPubSubClient dryRunPSC;
        if (jsonObjGlobal.isNull() == false)
            HA_DeviceDiscovery::SendDeviceGroupData(dryRunPSC, jsonObjGlobal);
        HA_DeviceDiscovery::SendBaseData(dryRunPSC, jsonObj, "dalhal");
        // second real send 
        HA_DeviceDiscovery::StartSendData(mqttClient, jsonObj, dryRunPSC.count);
        if (jsonObjGlobal.isNull() == false)
            HA_DeviceDiscovery::SendDeviceGroupData(mqttClient, jsonObjGlobal);
        HA_DeviceDiscovery::SendBaseData(mqttClient, jsonObj, "dalhal");
        mqttClient.endPublish();
    }
    
    Sensor::Sensor(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) : mqttClient(mqttClient), Device(UIDPathMaxLength::One,type) {
        const char* uidStr = GetAsConstChar(jsonObj, "uid");
        uid = encodeUID(uidStr);
        state_topic.reserve(sizeof("dalhal/sensor/") + strlen(uidStr));
        state_topic = "dalhal/sensor/" + std::string(uidStr);
        availability_topic = "dalhal_" + std::string(uidStr) + "/status";
        
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
        refreshMs = ParseRefreshTimeMs(jsonObj, 5000);
        SendDeviceDiscovery(mqttClient, jsonObj, jsonObjGlobal, jsonObjRoot);
        wasOnline = false;
        lastMs = millis()-refreshMs; // force a direct update after start
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

    Device* Sensor::Create(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) {
        return new Sensor(jsonObj, type, mqttClient, jsonObjGlobal, jsonObjRoot);
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

        if (cdr == nullptr) return; // nothing to automate

        unsigned long now = millis();
        if (now - lastMs < refreshMs) {
            return;
        }
        lastMs = now;

        HALValue val;
        HALOperationResult res = cdr->ReadSimple(val);
        if (res == HALOperationResult::Success) {
            if (!wasOnline) {
                mqttClient.publish(availability_topic.c_str(), "online");
                wasOnline = true;
            }
            mqttClient.publish(state_topic.c_str(), val.toString().c_str());
        } else {
            if (wasOnline) {
                mqttClient.publish(availability_topic.c_str(), "offline");
                wasOnline = false;
            }
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
        if (!wasOnline) {
            mqttClient.publish(availability_topic.c_str(), "online");
            wasOnline = true;
        }
        mqttClient.publish(state_topic.c_str(), val.toString().c_str());
        return HALOperationResult::Success;
    };
    
}