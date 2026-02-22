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
#include "../../../Drivers/REGO600.h"

namespace HAL_JSON {
    
    REGO600register::REGO600register(const JsonVariant &jsonObj, const char* type) : Device(type) {
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
    }

    bool REGO600register::VerifyJSON(const JsonVariant &jsonObj) {
        bool anyError = false;
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID) == false) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_REGO600_REG_VERIFY_JSON); anyError = true; }
        if (ValidateJsonStringField(jsonObj, "regname") == false) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_REGO600_REG_VERIFY_JSON); anyError = true; }
        const char* regName = GetAsConstChar(jsonObj, "regname");
        const Drivers::REGO600::RegoLookupEntry* entry = Drivers::REGO600::SystemRegisterTableLockup(regName);
        if (entry == nullptr) {
            GlobalLogger.Error(F("regname not found"), regName);
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_REGO600_REG_VERIFY_JSON);
            anyError = true;
        }
        
        return anyError == false;
    }

    Device* REGO600register::Create(const JsonVariant &jsonObj, const char* type) {
        return new REGO600register(jsonObj, type);
    }

    HALOperationResult REGO600register::read(HALValue& val) {
        val = value;
        return HALOperationResult::Success;
    }

    String REGO600register::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += '"';
        return ret;
    }

}