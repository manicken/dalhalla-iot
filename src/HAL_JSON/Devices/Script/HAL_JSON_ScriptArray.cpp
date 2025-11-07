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

#include "HAL_JSON_ScriptArray.h"

namespace HAL_JSON {
    
    ScriptArray::ScriptArray(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        uid = encodeUID(GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID));
        const JsonArray jsonArray = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        int arraySize = jsonArray.size();
        valueCount = arraySize;
        values = new HALValue[arraySize];
        for (int i=0;i<arraySize;i++) {
            const JsonVariant& item = jsonArray[i];
            if (item.is<uint32_t>())
                values[i] = item.as<uint32_t>();
            else if (item.is<int32_t>())
                values[i] = item.as<int32_t>();
            else
                values[i] = item.as<float>();
        }
        if (jsonObj.containsKey("readonly")) {
            readOnly = jsonObj["readonly"];
        }
    }

    bool ScriptArray::VerifyJSON(const JsonVariant &jsonObj) {
        const JsonArray jsonArray = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        int arraySize = jsonArray.size();
        for (int i=0;i<arraySize;i++) {
            const JsonVariant& item = jsonArray[i];
            if ((item.is<uint32_t>() || item.is<int32_t>() || item.is<float>()) == false) {
                GlobalLogger.Error(F("invalid array value type at index:"), std::to_string(i).c_str() );
                GlobalLogger.setLastEntrySource("ScriptArray VJ");
                return false;
            }
        }
        return true;
    }

    Device* ScriptArray::Create(const JsonVariant &jsonObj, const char* type) {
        return new ScriptArray(jsonObj, type);
    }

    String ScriptArray::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        return ret;
    }
    // init array here
    void ScriptArray::begin() {

    }

    HALOperationResult ScriptArray::read(const HALReadStringRequestValue& val) {

        if (val.cmd == "valuelist") {
            std::string ret;
            ret += '[';
            for (int i=0;i<valueCount;i++) {
                ret += values[i].toString();
                if (i<valueCount-1) ret += ',';
            }
            ret += ']';
            val.out_value = ret;
        } else if (val.cmd.ValidNumber()) {
            int index = 0;
            if (val.cmd.ConvertTo_int32(index) == false) {
                val.out_value = "invalid index not a integer";
                return HALOperationResult::BracketOpSubscriptInvalid;
            }

            if (index < 0 || index >= valueCount) {
                val.out_value = "invalid index out of range";
                return HALOperationResult::BracketOpSubscriptOutOffRange;
            }

            val.out_value = values[index].toString();
        } else {
            val.out_value = "unknown command";
            return HALOperationResult::UnsupportedCommand;
        }
        return HALOperationResult::Success;
    }

    HALOperationResult ScriptArray::read(const HALValue& bracketSubscriptVal, HALValue& val) {
        int index = bracketSubscriptVal.asInt();
        if (index < 0 || index >= valueCount) {
            printf("\nScriptArray::read BracketOpSubscriptOutOffRange:%d\n", index);
            return HALOperationResult::BracketOpSubscriptOutOffRange;
        }
        val = values[index];
        return HALOperationResult::Success;
    }

    HALOperationResult ScriptArray::write(const HALValue& bracketSubscriptVal, const HALValue& val) {
        if (readOnly) return HALOperationResult::UnsupportedOperation;

        int index = bracketSubscriptVal.asInt();
        if (index < 0 || index >= valueCount) {
            printf("\nScriptArray::write BracketOpSubscriptOutOffRange:%d\n", index);
            return HALOperationResult::BracketOpSubscriptOutOffRange;
        }
        values[index] = val;
        return HALOperationResult::Success;
    }
/*
    HALOperationResult ScriptArray::BracketRead_Func(Device* device, const HALValue& bracketSubscriptVal, HALValue& val) {
        return static_cast<ScriptArray*>(device)->read(bracketSubscriptVal, val);
    }
    HALOperationResult ScriptArray::BracketWrite_Func(Device* device, const HALValue& bracketSubscriptVal, const HALValue& val) {
        return static_cast<ScriptArray*>(device)->write(bracketSubscriptVal, val);
    }
    
    Device::BracketOpRead_FuncType ScriptArray::GetBracketOpRead_Function(ZeroCopyString& zcFuncName) {
        return ScriptArray::BracketRead_Func; // functionname not used at the moment
    }

    Device::BracketOpWrite_FuncType ScriptArray::GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) {
        return ScriptArray::BracketWrite_Func; // functionname not used at the moment
    }
*/

}