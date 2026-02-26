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

#include "HAL_JSON_PWMAnalogWrite.h"

#include "../../Support/HAL_JSON_Logger.h"
#include "../../Core/HAL_JSON_JSON_Config_Defines.h"
#include "../../Support/HAL_JSON_ArduinoJSON_ext.h"

#if defined(ESP8266)
    #define AnalogWriteResolution_func analogWriteResolution
    #define AnalogWriteFrequency_func analogWriteFreq;
#elif defined(ESP32)
    #define AnalogWriteResolution_func analogWriteResolution
    #define AnalogWriteFrequency_func analogWriteFrequency;
#endif

namespace HAL_JSON {
    
    // ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████      ██████ ███████  ██████  
    // ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██          ██      ██      ██       
    // ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████       ██      █████   ██   ███ 
    // ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██          ██      ██      ██    ██ 
    // ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████      ██████ ██       ██████  

    // Define static members somewhere in the cpp file (outside any function)
    uint8_t PWMAnalogWriteConfig::resolution = 0;
    uint32_t PWMAnalogWriteConfig::frequency = 0;

    Device* PWMAnalogWriteConfig::Create(const JsonVariant &jsonObj, const char* type) {
        return new PWMAnalogWriteConfig(jsonObj, type);
    }

    bool PWMAnalogWriteConfig::VerifyJSON(const JsonVariant &jsonObj) {
        if (IsUINT32(jsonObj, HAL_JSON_KEYNAME_PWM_CFG_FREQUENCY) == false) { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PWM_CFG_FREQUENCY)); return false; }
        if (IsUINT32(jsonObj, HAL_JSON_KEYNAME_PWM_CFG_RESOLUTION) == false) { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PWM_CFG_RESOLUTION)); return false; }
        return true;
    }

    PWMAnalogWriteConfig::PWMAnalogWriteConfig(const JsonVariant &jsonObj, const char* type) : Device(type) {
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);

        PWMAnalogWriteConfig::frequency = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PWM_CFG_FREQUENCY, 0);
        PWMAnalogWriteConfig::resolution = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PWM_CFG_RESOLUTION, 0);

        AnalogWriteResolution_func(PWMAnalogWriteConfig::resolution);
        AnalogWriteFrequency_func(PWMAnalogWriteConfig::frequency);
    }

    HALOperationResult PWMAnalogWriteConfig::write(const HALWriteStringRequestValue& value) {
        ZeroCopyString zcValue = value.value;
        ZeroCopyString zcCmd = zcValue.SplitOffHead('/');
        if (zcValue.IsEmpty()) { return HALOperationResult::InvalidArgument; }
        if (zcValue.ValidUINT() == false) { return HALOperationResult::WriteValueNotUintOrInt; }
        uint32_t uintValue = 0;
        zcValue.ConvertTo_uint32(uintValue);

        if (zcCmd == "freq") {
            PWMAnalogWriteConfig::frequency = uintValue;
            AnalogWriteFrequency_func(uintValue);
        } else if (zcCmd == "res") {
            PWMAnalogWriteConfig::resolution = uintValue;
            AnalogWriteResolution_func(uintValue);
        } else {
            return HALOperationResult::UnsupportedCommand;
        }
        return HALOperationResult::Success;
    }

    String PWMAnalogWriteConfig::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",\"freq\":";
        ret += std::to_string(PWMAnalogWriteConfig::frequency).c_str();
        ret += ",\"resolution\":";
        ret += std::to_string(PWMAnalogWriteConfig::resolution).c_str();
        return ret;
    }

    // ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████ 
    // ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██      
    // ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████   
    // ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██      
    // ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████ 

    Device* PWMAnalogWrite::Create(const JsonVariant &jsonObj, const char* type) {
        return new PWMAnalogWrite(jsonObj, type);
    }

    bool PWMAnalogWrite::VerifyJSON(const JsonVariant &jsonObj) {
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT));
    }

    PWMAnalogWrite::PWMAnalogWrite(const JsonVariant &jsonObj, const char* type) : Device(type) {
        pin = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PIN);// jsonObj[HAL_JSON_KEYNAME_PIN];// | 0;//.as<uint8_t>();
        uid = encodeUID(GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID));

        GPIO_manager::ReservePin(pin);

        pinMode(pin, OUTPUT); // output

    }

    PWMAnalogWrite::~PWMAnalogWrite() { pinMode(pin, INPUT); } // input

    HALOperationResult PWMAnalogWrite::read(HALValue &val) {
        //val.set(value); // just read back latest write
        val = value;
        return HALOperationResult::Success;
    }

    HALOperationResult PWMAnalogWrite::write(const HALValue &val) {
        if (val.getType() == HALValue::Type::TEST)  return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        //value = val.asUInt();
        value = val;
        if (inv_out)
            value = getInvValue(value);
        AnalogWriteResolution_func(PWMAnalogWriteConfig::resolution);
        AnalogWriteFrequency_func(PWMAnalogWriteConfig::frequency);
        analogWrite(pin, value);
        return HALOperationResult::Success;
    }

    String PWMAnalogWrite::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",";
        ret += DeviceConstStrings::value;//StartWithComma;
        ret += std::to_string(value).c_str();
        ret += ",\"inv_out\":";
        ret += std::to_string(inv_out).c_str();
        return ret;
    }

    uint32_t PWMAnalogWrite::getInvValue(uint32_t val)
    {
        if (PWMAnalogWriteConfig::resolution == 10)
            return 1023-val;
        else if (PWMAnalogWriteConfig::resolution == 8)
            return 255-val;

        return 255;
    }
	
}