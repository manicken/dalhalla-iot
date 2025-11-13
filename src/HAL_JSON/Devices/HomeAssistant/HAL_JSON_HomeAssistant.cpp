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

namespace HAL_JSON {
    
    HomeAssistant::HomeAssistant(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        mqttClient.setClient(wifiClient);
        
        port = 1883;
    }
    HomeAssistant::~HomeAssistant() {
        
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
    void HomeAssistant::begin() {}
    Device* HomeAssistant::findDevice(UIDPath& path) { return nullptr; }

    HALOperationResult HomeAssistant::read(HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult HomeAssistant::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        return HALOperationResult::UnsupportedOperation;
    };
    HALOperationResult HomeAssistant::read(const HALValue& bracketSubscriptVal, HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult HomeAssistant::write(const HALValue& bracketSubscriptVal, const HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult HomeAssistant::read(const HALReadStringRequestValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult HomeAssistant::write(const HALWriteStringRequestValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult HomeAssistant::read(const HALReadValueByCmd& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult HomeAssistant::write(const HALWriteValueByCmd& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult HomeAssistant::exec() { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult HomeAssistant::exec(ZeroCopyString& cmd) { return HALOperationResult::UnsupportedOperation; }
    Device::ReadToHALValue_FuncType HomeAssistant::GetReadToHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::WriteHALValue_FuncType HomeAssistant::GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::Exec_FuncType HomeAssistant::GetExec_Function(ZeroCopyString& zcFuncName) {return nullptr; } 

    Device::BracketOpRead_FuncType HomeAssistant::GetBracketOpRead_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::BracketOpWrite_FuncType HomeAssistant::GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    
    HALValue* HomeAssistant::GetValueDirectAccessPtr() { return nullptr; }
}