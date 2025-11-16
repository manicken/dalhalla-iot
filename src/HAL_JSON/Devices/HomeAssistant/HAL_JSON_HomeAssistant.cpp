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

        std::string clientId = "dalhalla_" + std::string(uidStr) + "_" + std::to_string(millis() & 0xFFFF);

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

        if (jsonObj.containsKey("group")) {
            // one global group def

        } else if (jsonObj.containsKey("groups")) {
            // one group def that can contain items

        }
        
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