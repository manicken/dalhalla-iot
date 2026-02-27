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

#include "DALHAL_PCF8574x.h"

#include "../../Support/DALHAL_Logger.h"
#include "../../Core/Device/DALHAL_JSON_Config_Defines.h"
#include "../../Support/DALHAL_ArduinoJSON_ext.h"

#include "../../Support/ConvertHelper.h"


namespace DALHAL {

    bool PCF8574x::HasAddress(uint8_t addr) {
        return (addr >= 0x20 && addr <= 0x27) || // PCF8574
               (addr >= 0x38 && addr <= 0x3f);   // PCF8574A
    }
    
    PCF8574x::PCF8574x(const JsonVariant &jsonObj, const char* type, TwoWire& wire) : Device(type), wire(&wire) {
        const char* uidStr = GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID);
        uid = encodeUID(uidStr);
        const char* addrStr = GetAsConstChar(jsonObj, "addr");
        addr = static_cast<uint8_t>(std::strtoul(addrStr, nullptr, 16));

    }

    bool PCF8574x::VerifyJSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj, DALHAL_KEYNAME_UID)){ 
            SET_ERR_LOC(DALHAL_ERROR_SOURCE_DISPLAY_SSD1306_VERIFY_JSON);
            return false;
        }
        if (false == ValidateJsonStringField(jsonObj, "addr")) {
            SET_ERR_LOC(DALHAL_ERROR_SOURCE_DISPLAY_SSD1306_VERIFY_JSON);
            return false;
        }
        return true;
    }

    Device* PCF8574x::Create(const JsonVariant &jsonObj, const char* type, TwoWire& wire) {
        return new PCF8574x(jsonObj, type, wire);
    }

    HALOperationResult PCF8574x::read(HALValue& val) {
        uint8_t received = wire->requestFrom(addr, (uint8_t)1);
        if (received == 0) return HALOperationResult::ExecutionFailed;
        val = (uint32_t)wire->read();
        return HALOperationResult::Success;
    }
    HALOperationResult PCF8574x::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        wire->beginTransmission(addr);
        wire->write(val.asUInt());
        uint8_t res = wire->endTransmission(true);
        if (res != 0) {
            // todo maybe log to global logger
            return HALOperationResult::ExecutionFailed;
        }
        return HALOperationResult::Success;
    }

    String PCF8574x::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += "\"addr\":\"0x";
        ret += Convert::toHex(addr).c_str();
        ret += "\"";
        return ret;
    }

}