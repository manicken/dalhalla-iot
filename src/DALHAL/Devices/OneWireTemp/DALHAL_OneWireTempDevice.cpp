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

#include "DALHAL_OneWireTempDevice.h"
#include "../../Support/ConvertHelper.h"

#include "../../Support/DALHAL_Logger.h"
#include "../../Core/Device/DALHAL_JSON_Config_Defines.h"
#include "../../Support/DALHAL_ArduinoJSON_ext.h"
#include "../../Core/Manager/DALHAL_GPIO_Manager.h"

namespace DALHAL {
    


    //   ██████  ███    ██ ███████     ██     ██ ██ ██████  ███████     ████████ ███████ ███    ███ ██████      ██████  ███████ ██    ██ ██  ██████ ███████ 
    //  ██    ██ ████   ██ ██          ██     ██ ██ ██   ██ ██             ██    ██      ████  ████ ██   ██     ██   ██ ██      ██    ██ ██ ██      ██      
    //  ██    ██ ██ ██  ██ █████       ██  █  ██ ██ ██████  █████          ██    █████   ██ ████ ██ ██████      ██   ██ █████   ██    ██ ██ ██      █████   
    //  ██    ██ ██  ██ ██ ██          ██ ███ ██ ██ ██   ██ ██             ██    ██      ██  ██  ██ ██          ██   ██ ██       ██  ██  ██ ██      ██      
    //   ██████  ██   ████ ███████      ███ ███  ██ ██   ██ ███████        ██    ███████ ██      ██ ██          ██████  ███████   ████   ██  ██████ ███████ 
    //                                                                                                                                                     

    bool OneWireTempDevice::VerifyJSON(const JsonVariant &jsonObj) {
        
        if (!ValidateJsonStringField(jsonObj, DALHAL_KEYNAME_UID)){ SET_ERR_LOC(DALHAL_ERROR_SOURCE_1WTD_VERIFY_JSON); return false; }
        if (!ValidateJsonStringField(jsonObj, DALHAL_KEYNAME_ONE_WIRE_ROMID)){ SET_ERR_LOC(DALHAL_ERROR_SOURCE_1WTD_VERIFY_JSON); return false; }
        
        const char* romIdStr = GetAsConstChar(jsonObj,DALHAL_KEYNAME_ONE_WIRE_ROMID);//].as<const char*>();
        if (strlen(romIdStr) == 0) { GlobalLogger.Error(F("OneWireTempDevice romId is zero lenght")); return false; }
        return Convert::HexToBytes(romIdStr, nullptr, 8);
    }

    OneWireTempDevice::OneWireTempDevice(const JsonVariant &jsonObj, const char* type) : SimpleEventDevice(type) {
        const char* uidStr = GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID);//].as<const char*>();
        uid = encodeUID(uidStr);
        const char* romIdStr = GetAsConstChar(jsonObj,DALHAL_KEYNAME_ONE_WIRE_ROMID);//].as<const char*>();
        Convert::HexToBytes(romIdStr, romid.bytes, 8);
        // optional settings
        if (jsonObj.containsKey(DALHAL_KEYNAME_ONE_WIRE_TEMPFORMAT) && ValidateJsonStringField_noContains(jsonObj, DALHAL_KEYNAME_ONE_WIRE_TEMPFORMAT)) {
            const char* tempFormatStr = GetAsConstChar(jsonObj, DALHAL_KEYNAME_ONE_WIRE_TEMPFORMAT);//].as<const char*>();
            if (tempFormatStr[0] == 'c' || tempFormatStr[0] == 'C')
                format = OneWireTempDeviceTempFormat::Celsius;
            else if (tempFormatStr[0] == 'f' || tempFormatStr[0] == 'F')
                format = OneWireTempDeviceTempFormat::Fahrenheit;
            // else the default value is used (defined in .h file)
        }
        
    }

    OneWireTempDevice::~OneWireTempDevice() {
        
    }

    HALOperationResult OneWireTempDevice::read(HALValue& val) {
        val = value;
        return HALOperationResult::Success;
    }

    void OneWireTempDevice::read(DallasTemperature& dTemp) {
        float tempVal = 0;
        bool updateVal = false;
        if (format == OneWireTempDeviceTempFormat::Celsius) {
            tempVal = dTemp.getTempC(romid.bytes);
            updateVal = (tempVal != DEVICE_DISCONNECTED_C);
        }
        else if (format == OneWireTempDeviceTempFormat::Fahrenheit) {
            tempVal = dTemp.getTempF(romid.bytes);
            updateVal = (tempVal != DEVICE_DISCONNECTED_F);
        }
        if (updateVal) {
            value = tempVal;
            triggerEvent(); // actually a async event trigger
        }
    }
    
    String OneWireTempDevice::ToString() {
        String ret;
        ret.reserve(128);
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",\"romid\":\"";
        ret += String(Convert::ByteArrayToString(romid.bytes, 8).c_str());
        ret += "\",\"format\":";
        if (format == OneWireTempDeviceTempFormat::Celsius) ret += "\"C\"";
        else if (format == OneWireTempDeviceTempFormat::Fahrenheit) ret += "\"F\"";
        else ret += "\"other\"";
        ret += ",";
        ret += DeviceConstStrings::value;//StartWithComma;
        ret += std::to_string(value).c_str();
        //ret += "\"";
        return ret;
    }

    //   ██     ██     ██ ██ ██████  ███████     ████████ ███████ ███    ███ ██████      ██████  ███████ ██    ██ ██  ██████ ███████      ██████      ██████   ██████   ██████  ████████ 
    //  ███     ██     ██ ██ ██   ██ ██             ██    ██      ████  ████ ██   ██     ██   ██ ██      ██    ██ ██ ██      ██          ██    ██     ██   ██ ██    ██ ██    ██    ██    
    //   ██     ██  █  ██ ██ ██████  █████          ██    █████   ██ ████ ██ ██████      ██   ██ █████   ██    ██ ██ ██      █████       ██ ██ ██     ██████  ██    ██ ██    ██    ██    
    //   ██     ██ ███ ██ ██ ██   ██ ██             ██    ██      ██  ██  ██ ██          ██   ██ ██       ██  ██  ██ ██      ██          ██ ██ ██     ██   ██ ██    ██ ██    ██    ██    
    //   ██      ███ ███  ██ ██   ██ ███████        ██    ███████ ██      ██ ██          ██████  ███████   ████   ██  ██████ ███████      █ ████      ██   ██  ██████   ██████     ██    

    Device* OneWireTempDeviceAtRoot::Create(const JsonVariant& jsonObj, const char* type) {
        return new OneWireTempDeviceAtRoot(jsonObj, type);
    }

    bool OneWireTempDeviceAtRoot::VerifyJSON(const JsonVariant &jsonObj) {
        if (OneWireTempDevice::VerifyJSON(jsonObj) == false) return false;
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinFunc::OUT) | static_cast<uint8_t>(GPIO_manager::PinFunc::IN)));
    }

    OneWireTempDeviceAtRoot::OneWireTempDeviceAtRoot(const JsonVariant &jsonObj, const char* type) 
        : OneWireTempDevice(jsonObj, type), 
          autoRefresh(
            [this](){ requestTemperatures(); },
            [this](){ readAll(); },
            ParseRefreshTimeMs(jsonObj,DALHAL_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS)
        )
    {
        pin = GetAsUINT32(jsonObj,DALHAL_KEYNAME_PIN);//].as<uint8_t>();
        GPIO_manager::ReservePin(pin); // this is in most cases taken care of in OneWireTempBus::VerifyJSON but there are situations where it's needed

        oneWire = new OneWire(pin);
        dTemp = new DallasTemperature(oneWire);
        dTemp->setWaitForConversion(false);
    }

    OneWireTempDeviceAtRoot::~OneWireTempDeviceAtRoot() {
        delete dTemp;
        delete oneWire;
        pinMode(pin, INPUT); // "free" the pin
    }

    void OneWireTempDeviceAtRoot::requestTemperatures() {
        dTemp->requestTemperatures();
    }

    void OneWireTempDeviceAtRoot::readAll() {
        read(*dTemp);
    }

    HALOperationResult OneWireTempDeviceAtRoot::write(const HALValue& val) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
        dTemp->setTempC(val); // only in simulator
        return HALOperationResult::Success;
#endif
        return HALOperationResult::UnsupportedOperation;
    }

    void OneWireTempDeviceAtRoot::loop() {
        autoRefresh.loop();
    }

    String OneWireTempDeviceAtRoot::ToString() {
        String ret;
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += OneWireTempDevice::ToString();
        ret += autoRefresh.ToString();
        return ret;
    }

}