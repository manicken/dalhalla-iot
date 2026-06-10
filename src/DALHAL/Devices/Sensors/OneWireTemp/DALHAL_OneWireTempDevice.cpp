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

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>
#include "DALHAL_OneWireTempDevice_JSON_Schema.h"

namespace DALHAL {
    
    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase OneWireTempDeviceAtRoot::RegistryDefine = {
        OneWireTempDeviceAtRoot::Create,
        &JsonSchema::OneWireTempDeviceAtRoot::Root,
        DALHAL_REACTIVE_EVENT_TABLE(ONE_WIRE_TEMP_DEVICE)
    };
    
    /* override */
    const Registry::DefineBase* OneWireTempDeviceAtRoot::GetRegistryDefine() {
        return &RegistryDefine;
    }

    //   ██████  ███    ██ ███████     ██     ██ ██ ██████  ███████     ████████ ███████ ███    ███ ██████      ██████  ███████ ██    ██ ██  ██████ ███████ 
    //  ██    ██ ████   ██ ██          ██     ██ ██ ██   ██ ██             ██    ██      ████  ████ ██   ██     ██   ██ ██      ██    ██ ██ ██      ██      
    //  ██    ██ ██ ██  ██ █████       ██  █  ██ ██ ██████  █████          ██    █████   ██ ████ ██ ██████      ██   ██ █████   ██    ██ ██ ██      █████   
    //  ██    ██ ██  ██ ██ ██          ██ ███ ██ ██ ██   ██ ██             ██    ██      ██  ██  ██ ██          ██   ██ ██       ██  ██  ██ ██      ██      
    //   ██████  ██   ████ ███████      ███ ███  ██ ██   ██ ███████        ██    ███████ ██      ██ ██          ██████  ███████   ████   ██  ██████ ███████ 
    //                                                                                                                                                     

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase OneWireTempDevice::RegistryDefine = {
        OneWireTempDevice::Create,
        &JsonSchema::OneWireTempDevice::Root,
        DALHAL_REACTIVE_EVENT_TABLE(ONE_WIRE_TEMP_DEVICE)
    };

    /* override */
    const Registry::DefineBase* OneWireTempDevice::GetRegistryDefine() {
        return &OneWireTempDevice::RegistryDefine;
    }

    Device* OneWireTempDevice::Create(DeviceCreateContext& context) {
        return new OneWireTempDevice(context);
    }

    OneWireTempDevice::OneWireTempDevice(DeviceCreateContext& context) : OneWireTempDevice_DeviceBase(context.deviceType) {
        JsonSchema::OneWireTempDevice::Extractors::Apply(context, this);
#if HAS_REACTIVE_VALUE_CHANGE(SCRIPT_VARIABLE)
        value.setCallbacks(this, GenericValueCallback<OneWireTempDevice_DeviceBase>, nullptr);
#endif
    }

    OneWireTempDevice::~OneWireTempDevice() {
        
    }

    HALOperationResult OneWireTempDevice::read(HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) { return HALOperationResult::Success; }

        if (!dataValid) return HALOperationResult::DataNotReady;
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
            dataValid = true;
            //triggerEvent(); // actually a async event trigger
        }
    }
    
    void OneWireTempDevice::PrintTo(StringBuilderStreamer& sbs) {

        Device::PrintTo(sbs);
        
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("romId"));
        sbs.write_char('"');
        sbs.write_asHex(romid.bytes, 8, ':');
        sbs.write_char('"');

        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("format"));
        sbs.write_char('"');
        if (format == OneWireTempDeviceTempFormat::Celsius) { sbs.write('C'); }
        else if (format == OneWireTempDeviceTempFormat::Fahrenheit) { sbs.write('F'); }
        else { sbs.write(F("other")); }
        sbs.write_char('"');

        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("value"));
        value.toString(sbs);

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
            [this](){ readAll(); }
        )
    {
        JsonSchema::OneWireTempDeviceAtRoot::Extractors::Apply(context, this);
        uint32_t refreshMs = JsonSchema::CommonTime::refreshTimeGroupFieldsRequired.ExtractFrom(*(context.jsonObjItem)).toUInt();
        autoRefresh.SetRefreshTimeMs(refreshMs);
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

    void OneWireTempDeviceAtRoot::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pin"), pin);
        sbs.write_json_value_separator();
        OneWireTempDevice::PrintTo(sbs);
        sbs.write_json_value_separator();
        autoRefresh.PrintTo(sbs);
    }

}