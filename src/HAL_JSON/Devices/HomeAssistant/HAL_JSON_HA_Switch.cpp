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

namespace HAL_JSON {

    void Switch::SendDeviceDiscovery(PubSubClient& mqttClient, const JsonVariant &jsonObj, const JsonVariant &jsonObjGlobal) {
        
    }
    
    Switch::Switch(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient) : mqttClient(mqttClient), Device(UIDPathMaxLength::One,type) {
        
    }
    Switch::~Switch() {
        
    }

    bool Switch::VerifyJSON(const JsonVariant &jsonObj) {

        return true;
    }

    Device* Switch::Create(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient) {
        return new Switch(jsonObj, type, mqttClient);
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
    void Switch::begin() {}
    Device* Switch::findDevice(UIDPath& path) { return nullptr; }

    HALOperationResult Switch::read(HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Switch::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        return HALOperationResult::UnsupportedOperation;
    };
    HALOperationResult Switch::read(const HALValue& bracketSubscriptVal, HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Switch::write(const HALValue& bracketSubscriptVal, const HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Switch::read(const HALReadStringRequestValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Switch::write(const HALWriteStringRequestValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Switch::read(const HALReadValueByCmd& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Switch::write(const HALWriteValueByCmd& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Switch::exec() { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Switch::exec(ZeroCopyString& cmd) { return HALOperationResult::UnsupportedOperation; }
    Device::ReadToHALValue_FuncType Switch::GetReadToHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::WriteHALValue_FuncType Switch::GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::Exec_FuncType Switch::GetExec_Function(ZeroCopyString& zcFuncName) {return nullptr; } 

    Device::BracketOpRead_FuncType Switch::GetBracketOpRead_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::BracketOpWrite_FuncType Switch::GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    
    HALValue* Switch::GetValueDirectAccessPtr() { return nullptr; }
}