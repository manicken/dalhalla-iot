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

#include "DALHAL_template.h"
#include "../../Support/DALHAL_Logger.h"
#include "../../Core/Device/DALHAL_JSON_Config_Defines.h"
#include "../../Support/DALHAL_ArduinoJSON_ext.h"
#include "../../Core/Manager/DALHAL_GPIO_Manager.h"

namespace DALHAL {

    Template::Template(const JsonVariant &jsonObj, const char* type) : TemplateDeviceBase(type) {

    }

    bool Template::VerifyJSON(const JsonVariant &jsonObj) {
        // this is a check only to verify that the pin cfg exist
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinFunc::OUT) | static_cast<uint8_t>(GPIO_manager::PinFunc::IN)));
    }

    Device* Template::Create(const JsonVariant &jsonObj, const char* type) {
        return new Template(jsonObj, type);
    }

    String Template::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        return ret;
    }

    void Template::loop() {}
    void Template::begin() {
#if HAS_REACTIVE(TEMPLATE, BEGIN)
        triggerBegin();
#endif        
    }
    DeviceFindResult Template::findDevice(UIDPath& path, Device*& outDevice) { return DeviceFindResult::SubDevicesNotSupported; }

    HALOperationResult Template::read(HALValue& val) {
#if HAS_REACTIVE(TEMPLATE, READ)
        triggerRead();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Template::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
#if HAS_REACTIVE(TEMPLATE, WRITE)
        triggerWrite();
#endif
        return HALOperationResult::UnsupportedOperation;
    };
    HALOperationResult Template::read(const HALValue& bracketSubscriptVal, HALValue& val) {
#if HAS_REACTIVE(TEMPLATE, BRACKET_READ)
        triggerBracketRead();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Template::write(const HALValue& bracketSubscriptVal, const HALValue& val) {
#if HAS_REACTIVE(TEMPLATE, BRACKET_WRITE)
        triggerBracketWrite();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Template::read(const HALReadStringRequestValue& val) {
#if HAS_REACTIVE(TEMPLATE, READ)
        triggerRead();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Template::write(const HALWriteStringRequestValue& val) {
#if HAS_REACTIVE(TEMPLATE, WRITE)
        triggerWrite();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Template::read(const HALReadValueByCmd& val) {
#if HAS_REACTIVE(TEMPLATE, READ)
        triggerRead();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Template::write(const HALWriteValueByCmd& val) {
#if HAS_REACTIVE(TEMPLATE, WRITE)
        triggerWrite();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Template::exec() {
#if HAS_REACTIVE(TEMPLATE, EXEC)
        triggerExec();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Template::exec(const ZeroCopyString& cmd) {
#if HAS_REACTIVE(TEMPLATE, EXEC)
        triggerExec();
#endif
        return HALOperationResult::UnsupportedOperation;
    }

    Device::ReadToHALValue_FuncType Template::GetReadToHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::WriteHALValue_FuncType Template::GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::Exec_FuncType Template::GetExec_Function(ZeroCopyString& zcFuncName) {return nullptr; } 

    Device::BracketOpRead_FuncType Template::GetBracketOpRead_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::BracketOpWrite_FuncType Template::GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    
    HALValue* Template::GetValueDirectAccessPtr() { return nullptr; }
}