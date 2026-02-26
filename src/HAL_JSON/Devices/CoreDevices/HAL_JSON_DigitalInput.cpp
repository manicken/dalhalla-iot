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

#include "HAL_JSON_DigitalInput.h"

#include "../../Support/HAL_JSON_Logger.h"
#include "../../Core/HAL_JSON_JSON_Config_Defines.h"
#include "../../Support/HAL_JSON_ArduinoJSON_ext.h"

namespace HAL_JSON {
    
    Device* DigitalInput::Create(const JsonVariant &jsonObj, const char* type) {
        return new DigitalInput(jsonObj, type);
    }

    bool DigitalInput::VerifyJSON(const JsonVariant &jsonObj) {
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, static_cast<uint8_t>(GPIO_manager::PinFunc::IN));
    }

    DigitalInput::DigitalInput(const JsonVariant &jsonObj, const char* type) : Device(type) {
        pin = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PIN);// jsonObj[HAL_JSON_KEYNAME_PIN];// | 0;//.as<uint8_t>();
        uid = encodeUID(GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID));
        //pin = jsonObj[HAL_JSON_KEYNAME_PIN];//.as<uint8_t>();
        GPIO_manager::ReservePin(pin);
        
        //const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID];//.as<const char*>();
        //uid = encodeUID(uidStr);

        pinMode(pin, INPUT); // input
    }

    HALOperationResult DigitalInput::read(HALValue &val) {
        //val.set((uint32_t)digitalRead(pin));
        val = (uint32_t)digitalRead(pin);
        return HALOperationResult::Success;
    }

    String DigitalInput::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",";
        ret += DeviceConstStrings::value;//StartWithComma;
        ret += std::to_string(digitalRead(pin)).c_str();
        return ret;
    }
	
}