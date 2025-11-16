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

    void Sensor::SendDeviceDiscovery(PubSubClient& mqttClient, const JsonVariant& jsonObj, const JsonVariant& jsonObjGlobal) {
        CountingPubSubClient dryRunPSC;

        HA_DeviceDiscovery::SendBaseData(jsonObj, jsonObjGlobal, "dalhal", dryRunPSC);
        // here can additional data be sent

        char topic[128];
        const char* typeStr = jsonObj["type"];
        const char* uidStr = jsonObj["uid"];
        snprintf(topic, sizeof(topic), "homeassistant/%s/%s/config", typeStr, uidStr);
        
        mqttClient.beginPublish(topic, dryRunPSC.count, true);
        HA_DeviceDiscovery::SendBaseData(jsonObj, jsonObjGlobal, "dalhal", mqttClient);
        

        // here can additional data be sent

        mqttClient.endPublish();
    }
    
    Sensor::Sensor(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal) : mqttClient(mqttClient), Device(UIDPathMaxLength::One,type) {

        SendDeviceDiscovery(mqttClient, jsonObj, jsonObjGlobal);
    }
    Sensor::~Sensor() {
        
    }

    bool Sensor::VerifyJSON(const JsonVariant &jsonObj) {

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

    }
    void Sensor::begin() {}
    Device* Sensor::findDevice(UIDPath& path) { return nullptr; }

    HALOperationResult Sensor::read(HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Sensor::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        return HALOperationResult::UnsupportedOperation;
    };
    HALOperationResult Sensor::read(const HALValue& bracketSubscriptVal, HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Sensor::write(const HALValue& bracketSubscriptVal, const HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Sensor::read(const HALReadStringRequestValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Sensor::write(const HALWriteStringRequestValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Sensor::read(const HALReadValueByCmd& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Sensor::write(const HALWriteValueByCmd& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Sensor::exec() { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Sensor::exec(ZeroCopyString& cmd) { return HALOperationResult::UnsupportedOperation; }
    Device::ReadToHALValue_FuncType Sensor::GetReadToHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::WriteHALValue_FuncType Sensor::GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::Exec_FuncType Sensor::GetExec_Function(ZeroCopyString& zcFuncName) {return nullptr; } 

    Device::BracketOpRead_FuncType Sensor::GetBracketOpRead_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::BracketOpWrite_FuncType Sensor::GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    
    HALValue* Sensor::GetValueDirectAccessPtr() { return nullptr; }
}