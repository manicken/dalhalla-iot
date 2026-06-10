/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2026 Jannik Svensson

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

#include "DALHAL__Template_.h"

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include "DALHAL__Template__JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase _Template_::RegistryDefine = {
        Create,
        &JsonSchema::_Template_,
        DALHAL_REACTIVE_EVENT_TABLE(_TEMPLATE_),
        &_Template_::FunctionTable
    };
    //volatile const void* keep__Template_ = &DALHAL::_Template_::RegistryDefine;

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::Exec_FuncType> _Template_::execFunctions[] = {
        {"_Template_", &_Template_::exec_Template_Function, "help"}
    };
    HALOperationResult _Template_::exec_Template_Function(Device* device) {
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::ReadToHALValue_FuncType> _Template_::readValueFunctions[] = {
        {"_Template_", &_Template_::readValue_Template_Function, "help"}
    };
    HALOperationResult _Template_::readValue_Template_Function(Device* device, HALValue& val) {
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::WriteHALValue_FuncType> _Template_::writeValueFunctions[] = {
        {"_Template_", &_Template_::writeValue_Template_Function, "help"}
    };
    HALOperationResult _Template_::writeValue_Template_Function(Device* device, const HALValue& val) {
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::BracketOpRead_FuncType> _Template_::bracketOpReadFunctions[] = {
        {"_Template_", &_Template_::bracketOpRead_Template_Function, "help"}
    };
    HALOperationResult _Template_::bracketOpRead_Template_Function(Device* device, const HALValue& subscriptValue, HALValue& outValue) {
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::BracketOpWrite_FuncType> _Template_::bracketOpWriteFunctions[] = {
        {"_Template_", &_Template_::bracketOpWrite_Template_Function, "help"}
    };
    HALOperationResult _Template_::bracketOpWrite_Template_Function(Device* device, const HALValue& subscriptValue, const HALValue& inValue) {
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::ReadString_FuncType> _Template_::readStringFunctions[] = {
        {"_Template_", &_Template_::readString_Template_Function, "help"}
    };
    HALOperationResult _Template_::readString_Template_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::WriteString_FuncType> _Template_::writeStringFunctions[] = {
        {"_Template_", &_Template_::writeString_Template_Function, "help"}
    };
    HALOperationResult _Template_::writeString_Template_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable _Template_::FunctionTable = {
        {execFunctions, sizeof(execFunctions) / sizeof(execFunctions[0])}, 

        {readValueFunctions, sizeof(readValueFunctions) / sizeof(readValueFunctions[0])}, 
        {writeValueFunctions, sizeof(writeValueFunctions) / sizeof(writeValueFunctions[0])}, 

        {bracketOpReadFunctions, sizeof(bracketOpReadFunctions) / sizeof(bracketOpReadFunctions[0])}, 
        {bracketOpWriteFunctions, sizeof(bracketOpWriteFunctions) / sizeof(bracketOpWriteFunctions[0])},

        {readStringFunctions, sizeof(readStringFunctions) / sizeof(readStringFunctions[0])}, 
        {writeStringFunctions, sizeof(writeStringFunctions) / sizeof(writeStringFunctions[0])}, 
    };
        

    _Template_::_Template_(DeviceCreateContext& context) : _Template__DeviceBase(context.deviceType) {
        //const JsonVariant& jsonObj = *(context.jsonObjItem);
    }

    Device* _Template_::Create(DeviceCreateContext& context) {
        return new _Template_(context);
    }

    void _Template_::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pin"), pin);
    }

    void _Template_::loop() {}
    void _Template_::begin() {
#if HAS_REACTIVE_BEGIN(TEMPLATE)
        triggerBegin();
#endif        
    }
    DeviceFindResult _Template_::findDevice(UIDPath& path, Device*& outDevice) { return DeviceFindResult::SubDevicesNotSupported; }

    HALOperationResult _Template_::read(HALValue& val) {
#if HAS_REACTIVE_READ(TEMPLATE)
        triggerRead();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult _Template_::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
#if HAS_REACTIVE_WRITE(TEMPLATE)
        triggerWrite();
#endif
        return HALOperationResult::UnsupportedOperation;
    };
    HALOperationResult _Template_::read(const HALValue& bracketSubscriptVal, HALValue& val) {
#if HAS_REACTIVE_BRACKET_READ(TEMPLATE)
        triggerBracketRead();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult _Template_::write(const HALValue& bracketSubscriptVal, const HALValue& val) {
#if HAS_REACTIVE_BRACKET_WRITE(TEMPLATE)
        triggerBracketWrite();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult _Template_::read(const HALReadStringRequestValue& val) {
#if HAS_REACTIVE_READ(TEMPLATE)
        triggerRead();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult _Template_::write(const HALWriteStringRequestValue& val) {
#if HAS_REACTIVE_WRITE(TEMPLATE)
        triggerWrite();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult _Template_::read(const HALReadValueByCmd& val) {
#if HAS_REACTIVE_READ(TEMPLATE)
        triggerRead();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult _Template_::write(const HALWriteValueByCmd& val) {
#if HAS_REACTIVE_WRITE(TEMPLATE)
        triggerWrite();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult _Template_::exec() {
#if HAS_REACTIVE_EXEC(TEMPLATE)
        triggerExec();
#endif
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult _Template_::exec(const ZeroCopyString& cmd) {
#if HAS_REACTIVE_EXEC(TEMPLATE)
        triggerExec();
#endif
        return HALOperationResult::UnsupportedOperation;
    }

    Device::ReadToHALValue_FuncType _Template_::GetReadToHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::WriteHALValue_FuncType _Template_::GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::Exec_FuncType _Template_::GetExec_Function(ZeroCopyString& zcFuncName) {return nullptr; } 

    Device::BracketOpRead_FuncType _Template_::GetBracketOpRead_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::BracketOpWrite_FuncType _Template_::GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    
    HALValue* _Template_::GetValueDirectAccessPtr() { return nullptr; }
}