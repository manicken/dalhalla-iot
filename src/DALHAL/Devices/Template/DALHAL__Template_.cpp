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
    
    /* override */
    const Registry::DefineBase* _Template_::GetRegistryDefine() {
        return &RegistryDefine;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::Exec> _Template_::execFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(_Template_::exec_Template_Function, "primary"),
        DALHAL_FUNCTION_ENTRY("_Template_", _Template_::exec_Template_Function, "help"),
    };
    HALOperationResult _Template_::exec_Template_Function(Device* device) {

#if HAS_REACTIVE_EXEC(TEMPLATE)
        triggerExec();
#endif
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> _Template_::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(_Template_::readValue_Template_Function, "primary"),
        DALHAL_FUNCTION_ENTRY("_Template_", _Template_::readValue_Template_Function, "help"),
    };
    HALOperationResult _Template_::readValue_Template_Function(Device* device, HALValue& val) {

#if HAS_REACTIVE_READ(TEMPLATE)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::WriteHALValue> _Template_::writeValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(_Template_::writeValue_Template_Function, "primary"),
        DALHAL_FUNCTION_ENTRY("_Template_", _Template_::writeValue_Template_Function, "help"),
    };
    HALOperationResult _Template_::writeValue_Template_Function(Device* device, const HALValue& val) {

#if HAS_REACTIVE_WRITE(TEMPLATE)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::BracketOpRead> _Template_::bracketOpReadFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(_Template_::bracketOpRead_Template_Function, "primary"),
        DALHAL_FUNCTION_ENTRY("_Template_", _Template_::bracketOpRead_Template_Function, "help"),
    };
    HALOperationResult _Template_::bracketOpRead_Template_Function(Device* device, const HALValue& subscriptValue, HALValue& outValue) {

#if HAS_REACTIVE_BRACKET_READ(TEMPLATE)
        triggerBracketRead();
#endif
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::BracketOpWrite> _Template_::bracketOpWriteFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(_Template_::bracketOpWrite_Template_Function, "primary"),
        DALHAL_FUNCTION_ENTRY("_Template_", _Template_::bracketOpWrite_Template_Function, "help"),
    };
    HALOperationResult _Template_::bracketOpWrite_Template_Function(Device* device, const HALValue& subscriptValue, const HALValue& inValue) {

#if HAS_REACTIVE_BRACKET_WRITE(TEMPLATE)
        triggerBracketWrite();
#endif
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::ReadString> _Template_::readStringFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(_Template_::readString_Template_Function, "primary"),
        DALHAL_FUNCTION_ENTRY("_Template_", _Template_::readString_Template_Function, "help"),
    };
    HALOperationResult _Template_::readString_Template_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {

        // Note here about trigger, 
        // dont think there is any practicular reason to exmit events on string read function
        // as events are mostly emitted to make scripting easier
        // and to link updates
        // this is a CLI API function only and thus there is no reason to emit events here
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::WriteString> _Template_::writeStringFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(_Template_::writeString_Template_Function, "primary"),
        DALHAL_FUNCTION_ENTRY("_Template_", _Template_::writeString_Template_Function, "help"),
    };
    HALOperationResult _Template_::writeString_Template_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {

        // Note here about trigger, 
        // dont think there is any practicular reason to exmit events on string write function
        // as events are mostly emitted to make scripting easier
        // and to link updates
        // this is a CLI API function only and thus there is no reason to emit events here
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable _Template_::FunctionTable = {
        DALHAL_FUNCTION_TABLE_ENTRY(execFunctions),

        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeValueFunctions),

        DALHAL_FUNCTION_TABLE_ENTRY(bracketOpReadFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(bracketOpWriteFunctions),

        DALHAL_FUNCTION_TABLE_ENTRY(readStringFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeStringFunctions),
    };

    constexpr DeviceFunctionTable _Template_::FunctionTable2 = {
        EmptyFunctionTable<FunctionTypes::Exec>,

        EmptyFunctionTable<FunctionTypes::ReadToHALValue>,
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,

        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,

        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
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

    HALValue* _Template_::GetValueDirectAccessPtr() { return nullptr; }
}