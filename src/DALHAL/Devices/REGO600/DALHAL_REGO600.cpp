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

#include "DALHAL_REGO600.h"

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>
#include <DALHAL/API/DALHAL_BlockStreamer.h>

#include <DALHAL/Support/ConvertHelper.h>

#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include "DALHAL_REGO600_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase REGO600::RegistryDefine = {
        Create,
        &JsonSchema::REGO600::Root,
        DALHAL_REACTIVE_EVENT_TABLE(REGO600),
        &REGO600::FunctionTable
    };
    
    /* override */
    const Registry::DefineBase* REGO600::GetRegistryDefine() {
        return &RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::ReadString> REGO600::readStringFunctions[] = {
        DALHAL_FUNCTION_ENTRY("lcd", REGO600::readString_RequestWholeLCD_Function, "request whole lcd contents"),
        DALHAL_FUNCTION_ENTRY("sysreg", REGO600::readString_RequestRegisterValue_Function, "read system register"),
        DALHAL_FUNCTION_ENTRY("dump", REGO600::readString_RequestRamDump_Function, "dump ram using lcd read offset"),
    };

    constexpr FunctionEntry<FunctionTypes::WriteString> REGO600::writeStringFunctions[] = {
        DALHAL_FUNCTION_ENTRY("sysreg", REGO600::writeString_WriteRegisterValue_Function, "write system register value"),
    };

    constexpr FunctionEntry<FunctionTypes::Exec> REGO600::execFunctions[] = {
        DALHAL_FUNCTION_ENTRY("activate_error_ws_print", REGO600::exec_activate_error_print_Function, "activate error print to websocket"),
        DALHAL_FUNCTION_ENTRY("deactivate_error_ws_print", REGO600::exec_deactivate_error_print_Function, "deactivate error print to websocket"),
    };

    /* impossible to do with the current dalhal synced reads
    constexpr FunctionEntry<FunctionTypes::BracketOpRead> REGO600::bracketOpReadFunctions[] = {
        DALHAL_FUNCTION_ENTRY("sysreg", REGO600::bracketOpRead_ReadRegisterValue_Function, "read a register value using the bracket function, where the reg index is given as the subscript value"),
    };
    */

    constexpr FunctionEntry<FunctionTypes::BracketOpWrite> REGO600::bracketOpWriteFunctions[] = {
        DALHAL_FUNCTION_ENTRY("sysreg", REGO600::bracketOpWrite_WriteRegisterValue_Function, "write a register value using the bracket function, where the reg index is given as the subscript value"),
    };

    constexpr DeviceFunctionTable REGO600::FunctionTable = {
        DALHAL_FUNCTION_TABLE_ENTRY(execFunctions),

        EmptyFunctionTable<FunctionTypes::ReadToHALValue>,
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,

        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        DALHAL_FUNCTION_TABLE_ENTRY(bracketOpWriteFunctions),

        DALHAL_FUNCTION_TABLE_ENTRY(readStringFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeStringFunctions),
    };

    
    void requestWholeLCD_Callback(void* cb_ctx, void* dataCtx, Drivers::REGO600::RequestMode mode) {
        auto* ctx = static_cast<CommandCallbackByValue*>(cb_ctx);
        BlockStreamer bs(ctx->cb, "rego600/lcd", BlockStreamer::DataType::Json);
        delete ctx;
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        sbs.write_jsonMemberStart(F("lines"));
        sbs.write_json_array_begin();
        const char* lcdData = (const char*)dataCtx;
        for (size_t i = 0; i < 4; i++) {
            if (i>0) {
                sbs.write_json_value_separator();
            }
            sbs.write_jsonQuoted(&lcdData[i*20], 20);
        }
        sbs.write_json_array_end();
        sbs.write_json_object_end();
        
    }
    HALOperationResult REGO600::readString_RequestWholeLCD_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        REGO600& self = static_cast<REGO600&>(*device);
        auto* ctx = new CommandCallbackByValue{sbs.GetCommandCallback()};
        
        if (!self.rego600->RequestWholeLCD(requestWholeLCD_Callback, ctx)) {
            sbs.write_jsonString(F("error"), F("cannot start new request - possible reasons are that it's allready one in progress"));
            return HALOperationResult::ExecutionFailed;
        }
        sbs.write_jsonString(F("status"), F("OK"));
        sbs.write_json_value_separator();
        sbs.write_jsonString(F("request"), F("enqueued"));
        return HALOperationResult::Success;
    }

    void requestRegisterValue_Callback(void* cb_ctx, void* dataCtx, Drivers::REGO600::RequestMode mode) {
        auto* ctx = static_cast<CommandCallbackByValue*>(cb_ctx);
        
        BlockStreamer bs(ctx->cb, "rego600/systemregister/read", BlockStreamer::DataType::Json);
        delete ctx;

        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        Drivers::REGO600::Request* req = static_cast<Drivers::REGO600::Request*>(dataCtx);
        if (req == nullptr) {
            sbs.write_jsonString(F("error"), F("request is nullptr"));
            sbs.write_json_object_end();
            return;
        }

        if (req->response.value == nullptr) {
            sbs.write_jsonString(F("error"), F("req->response.value is nullptr"));
            sbs.write_json_object_end();
            return;
        }
        uint16_t uintValue = req->response.value->toUInt();
        int16_t intValue = req->response.value->toInt();
        
        sbs.write_jsonMemberStart(F("hex"));
        sbs.write_char('"');
        sbs.write_asHex((uint16_t)uintValue);
        sbs.write_char('"');
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("uint"), uintValue);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("int"), intValue);
        sbs.write_json_object_end();
        
    }

    HALOperationResult REGO600::readString_RequestRegisterValue_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        REGO600& self = static_cast<REGO600&>(*device);
        ZeroCopyString zcRegIndex = zcStrParameters.SplitOffHead('/');
        if (zcRegIndex.IsEmpty()) {
            GlobalLogger.Error(F("REGO600 RequestRegisterValue missing regIndex parameter"));
            return HALOperationResult::InvalidArgument;
        }
        uint32_t regIndex = 0;
        if (!zcRegIndex.ConvertTo_uint32(regIndex)) {
            GlobalLogger.Error(F("REGO600 RequestRegisterValue regIndex parameter is not a unsigned integer"));
            return HALOperationResult::InvalidArgument;
        }
        auto* ctx = new CommandCallbackByValue{sbs.GetCommandCallback()};
        const Drivers::REGO600::OpCodeInfo& opInfo = Drivers::REGO600::REGO600Driver::getCmdInfo(Drivers::REGO600::CommandID::ReadSystemRegister);
        Drivers::REGO600::REGO600Driver::ManualRawEntry.address = regIndex;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.minVal.u16 = 0;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.maxVal.u16 = 65535;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.multiplier = 1;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.valueType = Drivers::REGO600::ValueType::Unsigned;

        auto req = std::make_unique<Drivers::REGO600::Request>(opInfo, Drivers::REGO600::REGO600Driver::ManualRawEntry);
        
        
        if (!self.rego600->OneTimeRequest(std::move(req), requestRegisterValue_Callback, ctx)) {
            sbs.write_jsonString(F("error"), F("cannot start new request - possible reasons are that it's allready one in progress"));
            return HALOperationResult::ExecutionFailed;
        }
        sbs.write_jsonString(F("info"), F("OK"));
        sbs.write_json_value_separator();
        sbs.write_jsonString(F("request"), F("enqueued"));
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("regIndex"));
        sbs.write_char('"');
        sbs.write_asHex((uint16_t)regIndex);
        sbs.write_char('"');
     
        return HALOperationResult::Success;
    }

    void writeRegisterValue_Callback(void* cb_ctx, void* dataCtx, Drivers::REGO600::RequestMode mode) {
        auto* ctx = static_cast<CommandCallbackByValue*>(cb_ctx);
        
        BlockStreamer bs(ctx->cb, "rego600/systemregister/write", BlockStreamer::DataType::Json);
        delete ctx;

        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        sbs.write_jsonString(F("value written"), F("ok"));
        sbs.write_json_object_end();
    }

    HALOperationResult REGO600::writeString_WriteRegisterValue_Function(Device* device, const ZeroCopyString& zcStrParameters, StringBuilderStreamer& sbs) {
        REGO600& self = static_cast<REGO600&>(*device);
        ZeroCopyString zcCopy = zcStrParameters;
        ZeroCopyString zcRegIndex = zcCopy.SplitOffHead('/');
        ZeroCopyString zcValue = zcCopy.SplitOffHead('/');
        if (zcRegIndex.IsEmpty()) {
            GlobalLogger.Error(F("REGO600 WriteRegisterValue missing regIndex parameter"));
            return HALOperationResult::InvalidArgument;
        }
        if (zcValue.IsEmpty()) {
            GlobalLogger.Error(F("REGO600 WriteRegisterValue missing value parameter"));
            return HALOperationResult::InvalidArgument;
        }
        uint32_t regIndex = 0;
        if (!zcRegIndex.ConvertTo_uint32(regIndex)) {
            GlobalLogger.Error(F("REGO600 WriteRegisterValue regIndex parameter is not a unsigned integer"));
            return HALOperationResult::InvalidArgument;
        }
        uint32_t value = 0;
        if (!zcValue.ConvertTo_uint32(value)) {
            GlobalLogger.Error(F("REGO600 WriteRegisterValue value parameter is not a unsigned integer"));
            return HALOperationResult::InvalidArgument;
        }
        auto* ctx = new CommandCallbackByValue{sbs.GetCommandCallback()};
        const Drivers::REGO600::OpCodeInfo& opInfo = Drivers::REGO600::REGO600Driver::getCmdInfo(Drivers::REGO600::CommandID::WriteSystemRegister);
        Drivers::REGO600::REGO600Driver::ManualRawEntry.address = regIndex;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.minVal.u16 = 0;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.maxVal.u16 = 65535;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.multiplier = 1;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.valueType = Drivers::REGO600::ValueType::Unsigned;
        Drivers::REGO600::REGO600Driver::ManualRequestValue = value;

        auto req = std::make_unique<Drivers::REGO600::Request>(opInfo, Drivers::REGO600::REGO600Driver::ManualRawEntry, Drivers::REGO600::REGO600Driver::ManualRequestValue);
               
        if (!self.rego600->OneTimeRequest(std::move(req), writeRegisterValue_Callback, ctx)) {
            sbs.write_jsonString(F("error"), F("cannot start new request - possible reason is that it's allready one in progress"));
            return HALOperationResult::ExecutionFailed;
        }
        //sbs.write_jsonString(F("info"), F("OK"));
        //sbs.write_json_value_separator();
        sbs.write_jsonString(F("request"), F("enqueued"));
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("regIndex"));
        sbs.write_char('"');
        sbs.write_asHex((uint16_t)regIndex);
        sbs.write_char('"');
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("value"));
        sbs.write_char('"');
        sbs.write_asHex((uint16_t)value);
        sbs.write_char('"');
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("opcode"));
        sbs.write_char('"');
        sbs.write_asHex((uint16_t)opInfo.opcode);
        sbs.write_char('"');
        //sbs.write_json_value_separator();
     
        return HALOperationResult::Success;
    }

    void requestRamDump_Callback(void* cb_ctx, void* dataCtx, Drivers::REGO600::RequestMode mode) {
        auto* ctx = static_cast<CommandCallbackByValue*>(cb_ctx);
        
        BlockStreamer bs(ctx->cb, "rego600/ramdump/read", BlockStreamer::DataType::Json);
        delete ctx;

        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        Drivers::REGO600::Request* req = static_cast<Drivers::REGO600::Request*>(dataCtx);
        if (req == nullptr) {
            sbs.write_jsonString(F("error"), F("request is nullptr"));
            sbs.write_json_object_end();
            return;
        }

        if (req->response.value == nullptr) {
            sbs.write_jsonString(F("error"), F("req->response.value is nullptr"));
            sbs.write_json_object_end();
            return;
        }
        uint8_t* bytes = req->response.rawBytes;

        sbs.write_jsonMemberStart(F("bytes"));
        sbs.write_json_array_begin();
        for (size_t i = 0; i < 20; i++) {
            if(i>0) { sbs.write_json_value_separator(); }
            sbs.write_char('"');
            sbs.write_asHex((uint8_t)bytes[i]);
            sbs.write_char('"');
        }
        sbs.write_json_array_end();

        /*uint16_t* words = reinterpret_cast<uint16_t*>(bytes);
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("words"));
        sbs.write_json_array_begin();
        for (size_t i = 0; i < 20; i++) {
            if(i>0) { sbs.write_json_value_separator(); }
            sbs.write_char('"');
            sbs.write_asHex((uint16_t)words[i]);
            sbs.write_char('"');
        }
        sbs.write_json_array_end();

        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("words_dec"));
        sbs.write_json_array_begin();
        for (size_t i = 0; i < 10; i++) {
            if(i>0) { sbs.write_json_value_separator(); }
            sbs.write(words[i]);
        }
        sbs.write_json_array_end();
*/
        sbs.write_json_object_end();
    }

    HALOperationResult REGO600::readString_RequestRamDump_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        REGO600& self = static_cast<REGO600&>(*device);
        ZeroCopyString zcRegIndex = zcStrParameters.SplitOffHead('/');
        if (zcRegIndex.IsEmpty()) {
            GlobalLogger.Error(F("REGO600 RequestRegisterValue missing regIndex parameter"));
            return HALOperationResult::InvalidArgument;
        }
        uint32_t regIndex = 0;
        if (!zcRegIndex.ConvertTo_uint32(regIndex)) {
            GlobalLogger.Error(F("REGO600 RequestRegisterValue regIndex parameter is not a unsigned integer"));
            return HALOperationResult::InvalidArgument;
        }
        auto* ctx = new CommandCallbackByValue{sbs.GetCommandCallback()};
        const Drivers::REGO600::OpCodeInfo& opInfo = Drivers::REGO600::REGO600Driver::getCmdInfo(Drivers::REGO600::CommandID::RamDump);
        Drivers::REGO600::REGO600Driver::ManualRawEntry.address = regIndex;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.minVal.u16 = 0;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.maxVal.u16 = 65535;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.multiplier = 1;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.valueType = Drivers::REGO600::ValueType::Unsigned;

        auto req = std::make_unique<Drivers::REGO600::Request>(opInfo, Drivers::REGO600::REGO600Driver::ManualRawEntry, self.rego600->GetRawBytesBufferRef());
        
        
        if (!self.rego600->OneTimeRequest(std::move(req), requestRamDump_Callback, ctx)) {
            sbs.write_jsonString(F("error"), F("cannot start new request - possible reasons are that it's allready one in progress"));
            return HALOperationResult::ExecutionFailed;
        }
        sbs.write_jsonString(F("info"), F("OK"));
        sbs.write_json_value_separator();
        sbs.write_jsonString(F("request"), F("enqueued"));
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("regIndex"));
        sbs.write_char('"');
        sbs.write_asHex((uint16_t)regIndex);
        sbs.write_char('"');
     
        return HALOperationResult::Success;
    }

    void writeRegisterValueByBracketOP_Callback(void* cb_ctx, void* dataCtx, Drivers::REGO600::RequestMode mode) {
        REGO600* self = static_cast<REGO600*>(cb_ctx);
        
#if HAS_REACTIVE_BRACKET_WRITE(REGO600)
        self->triggerBracketWrite();
#endif
    }

    HALOperationResult REGO600::bracketOpWrite_WriteRegisterValue_Function(Device* device, const HALValue& subscriptValue, const HALValue& inValue) {
        REGO600& self = static_cast<REGO600&>(*device);

        if (subscriptValue.getType() != HALValue::Type::UINT) {
            return HALOperationResult::InvalidArgument;
        }

        const Drivers::REGO600::OpCodeInfo& opInfo = Drivers::REGO600::REGO600Driver::getCmdInfo(Drivers::REGO600::CommandID::WriteSystemRegister);
        Drivers::REGO600::REGO600Driver::ManualRawEntry.address = subscriptValue.toUInt();
        Drivers::REGO600::REGO600Driver::ManualRawEntry.minVal.u16 = 0;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.maxVal.u16 = 65535;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.multiplier = 1;
        Drivers::REGO600::REGO600Driver::ManualRawEntry.valueType = Drivers::REGO600::ValueType::Unsigned;
        Drivers::REGO600::REGO600Driver::ManualRequestValue = inValue.toUInt();

        auto req = std::make_unique<Drivers::REGO600::Request>(opInfo, Drivers::REGO600::REGO600Driver::ManualRawEntry, Drivers::REGO600::REGO600Driver::ManualRequestValue);
               
        if (!self.rego600->OneTimeRequest(std::move(req), writeRegisterValueByBracketOP_Callback, &self)) {
            return HALOperationResult::ExecutionFailed;
        }
     
        return HALOperationResult::Success;
    }

    HALOperationResult REGO600::exec_activate_error_print_Function(Device* device) {
        Drivers::REGO600::ErrorReport::emitErrorsToWebSocket = true;
        return HALOperationResult::Success;
    }

    HALOperationResult REGO600::exec_deactivate_error_print_Function(Device* device) {
        Drivers::REGO600::ErrorReport::emitErrorsToWebSocket = false;
        return HALOperationResult::Success;
    }

    
    
    REGO600::REGO600(DeviceCreateContext& context) : REGO600_DeviceBase(context.deviceType) {
        JsonSchema::REGO600::Extractors::Apply(context, this);
    }

    REGO600::~REGO600() {
        if (rego600 != nullptr)
            delete rego600;
        if (requestList != nullptr) { // if for example the allocation did fail
            for (int i=0;i<registerItemCount; i++) {
                delete requestList[i];
            }
            delete[] requestList;
        }
        if (registerItems != nullptr) { // if for example the allocation did fail
            for (int i=0;i<registerItemCount; i++) {
                delete registerItems[i];
            }
            delete[] registerItems;
        }
        pinMode(rxPin, INPUT); // input
        pinMode(txPin, INPUT); // input
    }
    void REGO600::begin() {
        rego600->begin(); // this will initialize a first request
#if HAS_REACTIVE_BEGIN(REGO600)
        triggerBegin();
#endif
    }
    void REGO600::loop() {
        rego600->loop();
#if HAS_REACTIVE_CYCLE_COMPLETE(REGO600)
        if (rego600->RefreshLoopDone()) { // one hit flag check and clear if set
            triggerCycleComplete();
        }
#endif
    }

    Device* REGO600::Create(DeviceCreateContext& context) {
        return new REGO600(context);
    }

    void REGO600::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
        
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("items"));
        sbs.write_json_array_begin();

        for (int i=0;i<registerItemCount;i++) {
            if (i > 0) { sbs.write_json_value_separator(); }
            
            sbs.write_json_object_begin();
   
            registerItems[i]->PrintTo(sbs);

            sbs.write_json_value_separator();
            sbs.write_jsonMemberStart(F("opcode"));
            sbs.write_doublequote();
            sbs.write_asHex((uint8_t)requestList[i]->info.opcode);
            sbs.write_doublequote();

            sbs.write_json_value_separator();
            sbs.write_jsonMemberStart(F("addr"));
            sbs.write_doublequote();
            sbs.write_asHex(requestList[i]->def.address);
            sbs.write_doublequote();

            sbs.write_json_object_end();
        }
        sbs.write_json_object_end();
    }

    DeviceFindResult REGO600::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(registerItems, registerItemCount, path, this, outDevice);
    }

}