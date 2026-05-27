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
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>

#define DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE             uint32_t

namespace DALHAL {

    // forward declarations
    class Device; 
    class HALValue;
    enum class HALOperationResult;
    struct HALReadStringRequestValue;
    struct HALWriteStringRequestValue;

    namespace FunctionValueType {
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _UInt_ = 0x01;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _Int_ = 0x02;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _Float_ = 0x04;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _Bool_ = 0x08;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _All_ = _UInt_ | _Int_ | _Float_ | _Bool_;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _Number_ = _UInt_ | _Int_ | _Float_;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _UInt_Int_ = _UInt_ | _Int_;
        constexpr DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE _None_ = 0x00;

        std::string ToString(DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE mask);
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
        
        constexpr FunctionEntry(const char* name, Fn fn, const char* help, DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE rwTypeMask) 
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
    static std::string GetDeviceFunctions(const FunctionTable_t<Fn>& funcTable) {
        std::string ret;
        ret += '[';
        for (size_t i = 0; i<funcTable.count; ++i) {
            if (i>0) { ret += ','; }
            ret += "{\"name\":\""; ret += funcTable.items[i].name?funcTable.items[i].name:""; ret += '"'; // yes name can be nullptr or empty as that signal to use the standard read/write function if available
            //ret += ",\"help\":\""; ret += funcTable.items[i].help?funcTable.items[i].help:""; ret += '"';
            if (funcTable.items[i].rwTypeMask != FunctionValueType::_None_) {
                ret += ",\"rwTypeMask\":"; ret += FunctionValueType::ToString(funcTable.items[i].rwTypeMask);
            }
            if (funcTable.items[i].bracketTypeMask != FunctionValueType::_None_) {
                ret += ",\"bracketTypeMask\":"; ret += FunctionValueType::ToString(funcTable.items[i].bracketTypeMask);
            }
            ret += '}';
        }
        ret += ']';
        return ret;
    }

    struct DeviceFunctionTable {

        using Exec_FuncType = HALOperationResult (*)(Device*);
        using ReadToHALValue_FuncType = HALOperationResult (*)(Device* device, HALValue& outValue);
        using WriteHALValue_FuncType = HALOperationResult (*)(Device* device, const HALValue& value);
        using BracketOpRead_FuncType = HALOperationResult (*)(Device* device, const HALValue& subscriptValue, HALValue& outValue);
        using BracketOpWrite_FuncType = HALOperationResult (*)(Device* device, const HALValue& subscriptValue, const HALValue& value);
        using ReadString_FuncType = HALOperationResult (*)(Device* device, std::string& outStr);
        using WriteString_FuncType = HALOperationResult (*)(Device* device, ZeroCopyString zcStr);

        const FunctionTable_t<Exec_FuncType> exec;
        const FunctionTable_t<ReadToHALValue_FuncType> readValue;
        const FunctionTable_t<WriteHALValue_FuncType> writeValue;
        const FunctionTable_t<BracketOpRead_FuncType> bracketOpRead;
        const FunctionTable_t<BracketOpWrite_FuncType> bracketOpWrite;
        const FunctionTable_t<ReadString_FuncType> readString;
        const FunctionTable_t<WriteString_FuncType> writeString;

        constexpr DeviceFunctionTable(
            const FunctionTable_t<Exec_FuncType> exec,
            const FunctionTable_t<ReadToHALValue_FuncType> readValue,
            const FunctionTable_t<WriteHALValue_FuncType> writeValue,
            const FunctionTable_t<BracketOpRead_FuncType> bracketOpRead,
            const FunctionTable_t<BracketOpWrite_FuncType> bracketOpWrite,
            const FunctionTable_t<ReadString_FuncType> readString,
            const FunctionTable_t<WriteString_FuncType> writeString
        ) : 
            exec(exec), 
            readValue(readValue), 
            writeValue(writeValue), 
            bracketOpRead(bracketOpRead), 
            bracketOpWrite(bracketOpWrite), 
            readString(readString), 
            writeString(writeString)
        {}

        std::string ToString() const;

    };
}