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

#include "HAL_JSON_REGO600register.h"

namespace HAL_JSON {
    
    REGO600register::REGO600register(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
        const char* valueTypeStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_REGO600_VALUE_TYPE);
        
        if (CharArray::equalsIgnoreCase(valueTypeStr, "float"))
            valueType = ValueType::FLOAT;
        else if (CharArray::equalsIgnoreCase(valueTypeStr, "uint"))
            valueType = ValueType::UINT;
        else if (CharArray::equalsIgnoreCase(valueTypeStr, "int"))
            valueType = ValueType::INT;
        else if (CharArray::equalsIgnoreCase(valueTypeStr, "bool"))
            valueType = ValueType::BOOL;
        //else this will never happen if VerifyJSON is used beforehand
        
        // special note here
        // HAL_JSON_REGO600_KEYNAME_OPCODE and HAL_JSON_REGO600_KEYNAME_ADDRESS
        // are extracted outside of this class instance as they are used elsewhere
    }

    bool REGO600register::VerifyJSON(const JsonVariant &jsonObj) {
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID) == false) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_REGO600_REG_VERIFY_JSON); return false; }
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_REGO600_OPCODE) == false) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_REGO600_REG_VERIFY_JSON); return false; }
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_REGO600_ADDRESS) == false) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_REGO600_REG_VERIFY_JSON); return false; }
        const char* opcodeStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_REGO600_OPCODE);
        if (!Convert::IsValidHexString(opcodeStr)) { GlobalLogger.Error(F("OC not valid hexstr: "), opcodeStr); SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_REGO600_REG_VERIFY_JSON); return false; }
        const char* addressStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_REGO600_ADDRESS);
        if (!Convert::IsValidHexString(addressStr)) { GlobalLogger.Error(F("ADDR not valid hexstr: "), addressStr); SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_REGO600_REG_VERIFY_JSON); return false; }

        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_REGO600_VALUE_TYPE) == false) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_REGO600_REG_VERIFY_JSON); return false; }
        const char* valueTypeStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_REGO600_VALUE_TYPE);
        bool validValueType = false;
        if (CharArray::equalsIgnoreCase(valueTypeStr, "float")) validValueType = true;
        else if (CharArray::equalsIgnoreCase(valueTypeStr, "uint")) validValueType = true;
        else if (CharArray::equalsIgnoreCase(valueTypeStr, "int")) validValueType = true;
        else if (CharArray::equalsIgnoreCase(valueTypeStr, "bool")) validValueType = true;
        if (validValueType == false) { GlobalLogger.Error(F("valueType invalid"), valueTypeStr); SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_REGO600_REG_VERIFY_JSON); return false; }
        return true;
    }

    Device* REGO600register::Create(const JsonVariant &jsonObj, const char* type) {
        return new REGO600register(jsonObj, type);
    }

    HALOperationResult REGO600register::read(HALValue& val) {
        if (valueType == ValueType::FLOAT)
            val = (float)((float)value / 10.0f);
        else if (valueType == ValueType::BOOL)
            val = (uint32_t)((value > 0)?1:0);
        else if (valueType == ValueType::UINT)
            val = value;
        else if (valueType == ValueType::INT)
            val = (int32_t)value;
        else {
            return HALOperationResult::ExecutionFailed; // should not happen
        }
        return HALOperationResult::Success;
    }

    String REGO600register::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += '"';
        ret += ",\"valuetype\":\"";
        ret += HAL_JSON::ToString(valueType);
        ret += '"';
        return ret;
    }

}