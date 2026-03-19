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

#include "DALHAL_OneWireTempDevice.h"

#include <DALHAL/Support/ConvertHelper.h>

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include "DALHAL_OneWireTempDevice_JSON_Scheme.h"

namespace DALHAL {
    
    constexpr Registry::DefineBase OneWireTempDeviceAtRoot::RegistryDefine = {
        Create,
        &JsonSchema::OneWireTempDeviceAtRoot,
        DALHAL_REACTIVE_EVENT_TABLE(ONE_WIRE_TEMP_DEVICE)
    };

    //   ██████  ███    ██ ███████     ██     ██ ██ ██████  ███████     ████████ ███████ ███    ███ ██████      ██████  ███████ ██    ██ ██  ██████ ███████ 
    //  ██    ██ ████   ██ ██          ██     ██ ██ ██   ██ ██             ██    ██      ████  ████ ██   ██     ██   ██ ██      ██    ██ ██ ██      ██      
    //  ██    ██ ██ ██  ██ █████       ██  █  ██ ██ ██████  █████          ██    █████   ██ ████ ██ ██████      ██   ██ █████   ██    ██ ██ ██      █████   
    //  ██    ██ ██  ██ ██ ██          ██ ███ ██ ██ ██   ██ ██             ██    ██      ██  ██  ██ ██          ██   ██ ██       ██  ██  ██ ██      ██      
    //   ██████  ██   ████ ███████      ███ ███  ██ ██   ██ ███████        ██    ███████ ██      ██ ██          ██████  ███████   ████   ██  ██████ ███████ 
    //                                                                                                                                                     

    OneWireTempDevice::OneWireTempDevice(DeviceCreateContext& context) : OneWireTempDevice_DeviceBase(context.deviceType) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
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
#if HAS_REACTIVE_VALUE_CHANGE(SCRIPT_VARIABLE)
        value.setCallbacks(this, GenericValueCallback<OneWireTempDevice_DeviceBase>, nullptr);
#endif
    }

    OneWireTempDevice::~OneWireTempDevice() {
        
    }

    HALOperationResult OneWireTempDevice::read(HALValue& val) {
        val = value;
#if HAS_REACTIVE_READ(ONE_WIRE_TEMP_DEVICE)
        triggerRead();
#endif
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
            //triggerEvent(); // actually a async event trigger
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
        ret += value.toString().c_str();
        //ret += "\"";
        return ret;
    }

    //   ██     ██     ██ ██ ██████  ███████     ████████ ███████ ███    ███ ██████      ██████  ███████ ██    ██ ██  ██████ ███████      ██████      ██████   ██████   ██████  ████████ 
    //  ███     ██     ██ ██ ██   ██ ██             ██    ██      ████  ████ ██   ██     ██   ██ ██      ██    ██ ██ ██      ██          ██    ██     ██   ██ ██    ██ ██    ██    ██    
    //   ██     ██  █  ██ ██ ██████  █████          ██    █████   ██ ████ ██ ██████      ██   ██ █████   ██    ██ ██ ██      █████       ██ ██ ██     ██████  ██    ██ ██    ██    ██    
    //   ██     ██ ███ ██ ██ ██   ██ ██             ██    ██      ██  ██  ██ ██          ██   ██ ██       ██  ██  ██ ██      ██          ██ ██ ██     ██   ██ ██    ██ ██    ██    ██    
    //   ██      ███ ███  ██ ██   ██ ███████        ██    ███████ ██      ██ ██          ██████  ███████   ████   ██  ██████ ███████      █ ████      ██   ██  ██████   ██████     ██    

    Device* OneWireTempDeviceAtRoot::Create(DeviceCreateContext& context) {
        return new OneWireTempDeviceAtRoot(context);
    }

    OneWireTempDeviceAtRoot::OneWireTempDeviceAtRoot(DeviceCreateContext& context) 
        : OneWireTempDevice(context), 
          autoRefresh(
            [this](){ requestTemperatures(); },
            [this](){ readAll(); },
            ParseRefreshTimeMs(*(context.jsonObjItem),DALHAL_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS)
        )
    {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
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
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
    HALOperationResult OneWireTempDeviceAtRoot::write(const HALValue& val) {
        dTemp->setTempC(val); // only in simulator
        return HALOperationResult::Success;
    }
#endif

    void OneWireTempDeviceAtRoot::loop() {
        autoRefresh.loop();
    }

    String OneWireTempDeviceAtRoot::ToString() {
        String ret;
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\",";
        ret += OneWireTempDevice::ToString();
        ret += autoRefresh.ToString();
        return ret;
    }

}