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

namespace DALHAL {

    // forward declarations
    class Device; 
    class HALValue;
    enum class HALOperationResult;
    struct HALReadStringRequestValue;
    struct HALWriteStringRequestValue;
    struct ZeroCopyString;

    template<typename Fn>
    struct FunctionEntry {
        const char* name;
        Fn fn;
        const char* help;
        

        constexpr FunctionEntry(const char* name, Fn fn, const char* help) : name(name), fn(fn), help(help) {}
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
    static std::string GetDeviceFunctions(const FunctionTable_t<Fn>& funcTable) {
        std::string ret;
        ret += '[';
        for (size_t i = 0; i<funcTable.count; ++i) {
            if (i>0) { ret += ','; }
            ret += "{\"name\":\""; ret += funcTable.items[i].name?funcTable.items[i].name:""; ret += '"'; // yes name can be nullptr or empty as that signal to use the standard read/write function if available
            ret += ",\"help\":\""; ret += funcTable.items[i].help?funcTable.items[i].help:""; ret += "\"}";
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
        using ReadString_FuncType = HALOperationResult (*)(Device* device, const HALReadStringRequestValue& val);
        using WriteString_FuncType = HALOperationResult (*)(Device* device, const HALWriteStringRequestValue& val);

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