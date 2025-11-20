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

namespace HAL_JSON {

    void Switch::SendDeviceDiscovery(PubSubClient& mqttClient, const JsonVariant& jsonObj, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) {
        // first dry run to calculate payload size
        CountingPubSubClient dryRunPSC;
        HA_DeviceDiscovery::SendDeviceGroupData(dryRunPSC, jsonObjGlobal);
        HA_DeviceDiscovery::SendBaseData(dryRunPSC, jsonObj, "dalhal");
        // second real send 
        HA_DeviceDiscovery::StartSendData(mqttClient, jsonObj, dryRunPSC.count);
        HA_DeviceDiscovery::SendDeviceGroupData(mqttClient, jsonObjGlobal);
        HA_DeviceDiscovery::SendBaseData(mqttClient, jsonObj, "dalhal");
        mqttClient.endPublish();
    }
    
    Switch::Switch(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) : mqttClient(mqttClient), Device(UIDPathMaxLength::One,type) {
        

        SendDeviceDiscovery(mqttClient, jsonObj, jsonObjGlobal, jsonObjRoot);
    }
    Switch::~Switch() {
        
    }

    bool Switch::VerifyJSON(const JsonVariant &jsonObj) {

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

    void Switch::loop() {

    }
    void Switch::begin() {

    }

    HALOperationResult Switch::read(HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Switch::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        return HALOperationResult::UnsupportedOperation;
    };

}