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

#pragma once

#include <stddef.h>
#include <string>
#include <DALHAL/Core/Types/DALHAL_Device.h> // can include complete definition
#include <DALHAL/Core/Types/DALHAL_Registry.h>
#include <DALHAL/Core/Types/DALHAL_DeviceFunctionTypes.h>
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>
#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>

#define DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE             uint32_t

namespace DALHAL {

    // forward declarations
    class HALValue;
    enum class HALOperationResult;

    namespace FunctionValueType {
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _UInt_ = 0x01;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _Int_ = 0x02;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _Float_ = 0x04;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _Bool_ = 0x08;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _All_ = _UInt_ | _Int_ | _Float_ | _Bool_;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _Number_ = _UInt_ | _Int_ | _Float_;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _UInt_Int_ = _UInt_ | _Int_;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _None_ = 0x00;

        void PrintTo(DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE mask, DALHAL::StringBuilderStreamer& sbs);
        inline bool HasFlag(DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE mask, DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE flag) {
            return (mask & flag) == flag;
        }
        inline bool HasAnyFlag(DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE mask, DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE flag) {
            return (mask & flag)>0;
        }
    }

    template<typename Fn>
    struct FunctionEntry {
        const char* name;
        Fn fn;
        const char* help;
        DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE rwTypeMask;
        DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE bracketTypeMask;

        constexpr FunctionEntry(const char* name, Fn fn, const char* help) 
            : name(name), fn(fn), help(help), rwTypeMask(FunctionValueType::_None_), bracketTypeMask(FunctionValueType::_None_) {}
        
        constexpr FunctionEntry(const char* name, Fn fn, const char* help, 
                DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE rwTypeMask) 
            : name(name), fn(fn), help(help), rwTypeMask(rwTypeMask), bracketTypeMask(FunctionValueType::_None_) {}

        constexpr FunctionEntry(const char* name, Fn fn, const char* help, 
                DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE rwTypeMask,
                DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE bracketTypeMask) 
            : name(name), fn(fn), help(help), rwTypeMask(rwTypeMask), bracketTypeMask(bracketTypeMask) {}
    };

    template<typename Fn>
    struct FunctionTable_t {
        const FunctionEntry<Fn>* items;
        size_t count;

        constexpr FunctionTable_t(const FunctionEntry<Fn>* items, size_t count) : items(items), count(count) {}
    };

    template<typename Fn>
    static constexpr FunctionTable_t<Fn> EmptyFunctionTable = {
        nullptr,
        0
    };

    

    template<typename Fn>
    static Fn GetDeviceFunction(const FunctionTable_t<Fn>& funcTable, const ZeroCopyString& zcFuncName) {
        for (size_t i = 0; i<funcTable.count; ++i) {
            if (zcFuncName.EqualsIC(funcTable.items[i].name)) {
                return funcTable.items[i].fn;
            }
        }
        return nullptr;
    }

    template<typename Fn>
    static const FunctionEntry<Fn>* GetDeviceFunctionEntry(const FunctionTable_t<Fn>& funcTable, const ZeroCopyString& zcFuncName) {
        for (size_t i = 0; i<funcTable.count; ++i) {
            if (zcFuncName.EqualsIC(funcTable.items[i].name)) {
                return &funcTable.items[i];
            }
        }
        return nullptr;
    }

    template<typename Fn>
    static void GetDeviceFunctions(const FunctionTable_t<Fn>& funcTable, DALHAL::StringBuilderStreamer& sbs) {

        sbs.write_json_array_begin();
        for (size_t i = 0; i<funcTable.count; ++i) {
            if (i>0) { sbs.write_json_value_separator(); }
            sbs.write(F("{\"name\":\"")); if (funcTable.items[i].name) { sbs.write(funcTable.items[i].name); } sbs.write_char('"'); // yes name can be nullptr or empty as that signal to use the standard read/write function if available
            sbs.write(F(",\"help\":\"")); if (funcTable.items[i].help) { sbs.write(funcTable.items[i].help); } sbs.write_char('"');
            if (funcTable.items[i].rwTypeMask != FunctionValueType::_None_) {
                sbs.write(F(",\"rwTypeMask\":")); FunctionValueType::PrintTo(funcTable.items[i].rwTypeMask, sbs);
            }
            if (funcTable.items[i].bracketTypeMask != FunctionValueType::_None_) {
                sbs.write(F(",\"bracketTypeMask\":")); FunctionValueType::PrintTo(funcTable.items[i].bracketTypeMask, sbs);
            }
            sbs.write_json_object_end();
        }
        sbs.write_json_array_end();
    }

    struct DeviceFunctionTable {

        

        const FunctionTable_t<FunctionTypes::Exec> exec;
        const FunctionTable_t<FunctionTypes::ReadToHALValue> readValue;
        const FunctionTable_t<FunctionTypes::WriteHALValue> writeValue;
        const FunctionTable_t<FunctionTypes::BracketOpRead> bracketOpRead;
        const FunctionTable_t<FunctionTypes::BracketOpWrite> bracketOpWrite;
        const FunctionTable_t<FunctionTypes::ReadString> readString;
        const FunctionTable_t<FunctionTypes::WriteString> writeString;

        constexpr DeviceFunctionTable(
            const FunctionTable_t<FunctionTypes::Exec> exec,
            const FunctionTable_t<FunctionTypes::ReadToHALValue> readValue,
            const FunctionTable_t<FunctionTypes::WriteHALValue> writeValue,
            const FunctionTable_t<FunctionTypes::BracketOpRead> bracketOpRead,
            const FunctionTable_t<FunctionTypes::BracketOpWrite> bracketOpWrite,
            const FunctionTable_t<FunctionTypes::ReadString> readString,
            const FunctionTable_t<FunctionTypes::WriteString> writeString
        ) : 
            exec(exec), 
            readValue(readValue), 
            writeValue(writeValue), 
            bracketOpRead(bracketOpRead), 
            bracketOpWrite(bracketOpWrite), 
            readString(readString), 
            writeString(writeString)
        {}

        void PrintTo(DALHAL::StringBuilderStreamer& sbs) const;

    };

    template<typename Fn>
    static const FunctionTable_t<Fn>* GetTable(const DeviceFunctionTable* t)
    {
        if constexpr (std::is_same_v<Fn, FunctionTypes::Exec>) {
            return &t->exec;
        } else if constexpr (std::is_same_v<Fn, FunctionTypes::ReadToHALValue>) {
            return &t->readValue;
        } else if constexpr (std::is_same_v<Fn, FunctionTypes::WriteHALValue>) {
            return &t->writeValue;
        } else if constexpr (std::is_same_v<Fn, FunctionTypes::BracketOpRead>) {
            return &t->bracketOpRead;
        } else if constexpr (std::is_same_v<Fn, FunctionTypes::BracketOpWrite>) {
            return &t->bracketOpWrite;
        } else if constexpr (std::is_same_v<Fn, FunctionTypes::ReadString>) {
            return &t->readString;
        } else if constexpr (std::is_same_v<Fn, FunctionTypes::WriteString>) {
            return &t->writeString;
        } else {
            static_assert(sizeof(Fn) == 0,
              "GetTable<Fn>: unsupported FunctionTypes specialization");
        }

        return nullptr;
    }

    template<typename Fn>
    struct FunctionLookupResult {
        HALOperationResult result;
        Fn fn;
    };

    template<typename Fn>
    static FunctionLookupResult<Fn> GetDeviceFunction(Device* device, const ZeroCopyString& zcFuncName) {
        if (device == nullptr) {// absolute failsafe
            return { HALOperationResult::DeviceNotFound, nullptr }; 
        } 
        const Registry::DefineBase* regDef = device->GetRegistryDefine();
        if (regDef == nullptr) { 
            return { HALOperationResult::RegistryDefineNotFound, nullptr };
        }
        const DeviceFunctionTable* devFuncTable = regDef->functionTable;
        if (devFuncTable == nullptr) {
            return { HALOperationResult::FunctionTableNotFound, nullptr };
        }

        const FunctionTable_t<Fn>* table = GetTable<Fn>(devFuncTable);
        if (!table) {
            return { HALOperationResult::FunctionTable_t_NotFound, nullptr };
        }
        if (table->items == nullptr || table->count == 0) {
            return { HALOperationResult::UnsupportedOperation, nullptr };
        }
        Fn fn = GetDeviceFunction(*table, zcFuncName);
        if (fn == nullptr) {
            return { HALOperationResult::UnsupportedCommand, nullptr };
        }
        return { HALOperationResult::Success, fn };
    }
}