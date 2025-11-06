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

#include "HAL_JSON_CachedDeviceRead.h"
#include "ScriptEngine/HAL_JSON_SCRIPT_ENGINE_Support.h"

namespace HAL_JSON {

    HALOperationResult CachedDeviceRead::ReadSimple(HALValue& val) {
        return handler(context, val);
    }

    struct FuncContext {
        Device* device;
        Device::ReadToHALValue_FuncType func;
    };

    struct BracketContext {
        Device* device;
        Device::BracketOpRead_FuncType bracketFunc;
        CachedDeviceRead* subscriptOperand; // owned
        ~BracketContext() {
            if (subscriptOperand)
                delete subscriptOperand; 
        }
    };

    HALOperationResult CachedDeviceRead::Handler_Direct(void* ctx, HALValue& val) {
        HALValue* valPtr = static_cast<HALValue*>(ctx);
        val = *valPtr;
        return HALOperationResult::Success;
    }

    HALOperationResult CachedDeviceRead::Handler_Device(void* ctx, HALValue& val) {
        Device* device = static_cast<Device*>(ctx);
        return device->read(val);
    }

    HALOperationResult CachedDeviceRead::Handler_Func(void* ctx, HALValue& val) {
        FuncContext* c = static_cast<FuncContext*>(ctx);
        return c->func(c->device, val);
    }

    HALOperationResult CachedDeviceRead::Handler_Bracket(void* ctx, HALValue& val) {
        BracketContext* c = static_cast<BracketContext*>(ctx);
        HALValue subVal;
        HALOperationResult res = c->subscriptOperand->ReadSimple(subVal);
        if (res != HALOperationResult::Success) return res;

        if (c->bracketFunc)
            return c->bracketFunc(c->device, subVal, val);
        else
            return c->device->read(subVal, val);
    }

    CachedDeviceRead::CachedDeviceRead(ZeroCopyString zcStrUidPathAndFuncName) 
        : context(nullptr), handler(nullptr), deleter(nullptr) 
    {
        const char* bracketPos = zcStrUidPathAndFuncName.FindChar('[');
        if (bracketPos) {
            // Bracket read
            ZeroCopyString inner(bracketPos + 1, zcStrUidPathAndFuncName.end - 1);
            CachedDeviceRead* subOperand = new CachedDeviceRead(inner);
            zcStrUidPathAndFuncName.end = bracketPos;

            UIDPath uid(zcStrUidPathAndFuncName);
            Device* device = Manager::findDevice(uid);
            auto* ctx = new BracketContext{device, device->GetBracketOpRead_Function(zcStrUidPathAndFuncName), subOperand};

            context = ctx;
            handler = &CachedDeviceRead::Handler_Bracket;
            deleter = ScriptEngine::DeleteAs<BracketContext>;
            return;
        }

        // Funcname or plain read
        ZeroCopyString operandName = zcStrUidPathAndFuncName.SplitOffHead('#');
        ZeroCopyString& funcName = zcStrUidPathAndFuncName;
        UIDPath uid(operandName);
        Device* device = Manager::findDevice(uid);
        if (!device) { 
            handler = [](void*, HALValue&) { return HALOperationResult::ExecutionFailed; };
            return; 
        }

        // Func read
        Device::ReadToHALValue_FuncType readFunc = device->GetReadToHALValue_Function(funcName);
        if (readFunc) {
            auto* ctx = new FuncContext{device, readFunc};
            context = ctx;
            handler = &CachedDeviceRead::Handler_Func;
            deleter = ScriptEngine::DeleteAs<FuncContext>;
            return;
        }

        // Direct access
        deleter = nullptr; // non owning
        HALValue* valPtr = device->GetValueDirectAccessPtr();
        if (valPtr) {
            context = valPtr;
            handler = &CachedDeviceRead::Handler_Direct;
            return;
        }

        // Fallback device read
        context = device;
        handler = &CachedDeviceRead::Handler_Device;
    }

}