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

#include "DALHAL_I2C_Master.h"

#include <DALHAL/Devices/I2C_Master/_DevicesRegistry/DALHAL_I2C_Master_DevicesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Support/ConvertHelper.h>

#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>

#include "DALHAL_I2C_Master_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase I2C_Master::RegistryDefine = {
        &I2C_DeviceRegistry,
        Create,
        &JsonSchema::I2C_Master::Root,
        DALHAL_REACTIVE_EVENT_TABLE(I2C_MASTER)
    };
    
    /* override */
    const Registry::DefineBase* I2C_Master::GetRegistryDefine() {
        return &RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::ReadString> I2C_Master::readStringFunctions[] = {
        DALHAL_FUNCTION_ENTRY("raw", read_raw, "read raw data"),
        DALHAL_FUNCTION_ENTRY("list", list_devices, "list all devices found by using adress scan"),
    };

    constexpr FunctionEntry<FunctionTypes::WriteString> I2C_Master::writeStringFunctions[] = {
        DALHAL_FUNCTION_ENTRY("raw", write_raw, "write raw data"),
        DALHAL_FUNCTION_ENTRY("speed", set_speed, "set i2c speed"),
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable I2C_Master::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        EmptyFunctionTable<FunctionTypes::ReadToHALValue>, 
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        DALHAL_FUNCTION_TABLE_ENTRY(readStringFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeStringFunctions),
    };

    Device* I2C_Master::Create(DeviceCreateContext& context) {
        return new I2C_Master(context);
    }
    
    I2C_Master::I2C_Master(DeviceCreateContext& context) : I2C_Master_DeviceBase(context.deviceType) {
        JsonSchema::I2C_Master::Extractors::Apply(context, this);
    }

    I2C_Master::~I2C_Master() {
        if (devices != nullptr) {
            for (int i=0;i<deviceCount;i++) {
                delete devices[i];
                devices[i] = nullptr;
            }
            delete[] devices;
            devices = nullptr;
            deviceCount = 0;
        }
#if defined(ESP32)
        wire->end();
#endif
        pinMode(sckpin, INPUT);
        pinMode(sdapin, INPUT);
    }

    void I2C_Master::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("SDA PIN"), sdapin);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("SCK PIN"), sckpin);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("freq"), freq);
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("devices"));
        sbs.write_json_array_begin();
        
        for (int i=0;i<deviceCount;i++) {
            if (i > 0) { sbs.write_json_value_separator(); }
            sbs.write_json_object_begin();
            devices[i]->PrintTo(sbs);
            sbs.write_json_object_end();

        }
        sbs.write_json_array_end();
    }

    DeviceFindResult I2C_Master::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(devices, deviceCount, path, this, outDevice);
    }

    void I2C_Master::loop() {
        for (int i=0;i<deviceCount;i++) {
            devices[i]->loop();
        }
    }

    /* static */
    HALOperationResult I2C_Master::write_raw(Device* device, ZeroCopyString zcParams, StringBuilderStreamer& sbs) {
        if (zcParams.IsEmpty()) {
            return HALOperationResult::StringRequestParameterError;
        }
        ZeroCopyString zcAddr = zcParams.SplitOffHead('/');
        if (zcParams.IsEmpty()) return HALOperationResult::StringRequestParameterError; // simple early check
        if (zcAddr.ValidUINT() == false) { return HALOperationResult::StringRequestParameterError; }
        ZeroCopyString zcByteCount = zcParams.SplitOffHead('/');
        if (zcByteCount.ValidUINT() == false) { return HALOperationResult::StringRequestParameterError; }
        uint32_t bytesToWrite = 0;
        zcByteCount.ConvertTo_uint32(bytesToWrite);
        if (bytesToWrite == 0) { return HALOperationResult::StringRequestParameterError; }
        
        uint32_t paramCount = zcParams.CountChar('/')+1; // +1 to make it easier/clearer
        if (paramCount < bytesToWrite) { return HALOperationResult::StringRequestParameterError; }
        uint32_t addr = 0;
        zcAddr.ConvertTo_uint32(addr);
        I2C_Master& self = *static_cast<I2C_Master*>(device);
        self.wire->beginTransmission((uint8_t)addr);
        while (bytesToWrite--) {
            ZeroCopyString zcByte = zcParams.SplitOffHead('/');
            if (zcByte.ValidUINT() == false) {
                self.wire->endTransmission(true);
                return HALOperationResult::StringRequestParameterError;
            }
            uint32_t byteVal  = 0;
            zcByte.ConvertTo_uint32(byteVal );
            self.wire->write((uint8_t)byteVal );
        }
        uint8_t res = self.wire->endTransmission(true);
        if (res != 0) {
            //val.result.write((char)(0x30 + res));
            return HALOperationResult::ExecutionFailed;
        }
        return HALOperationResult::Success;
    }

    /* static */
    HALOperationResult I2C_Master::set_speed(Device* device, ZeroCopyString zcParams, StringBuilderStreamer& sbs) {
        if (zcParams.IsEmpty()) { 
            return HALOperationResult::StringRequestParameterError;
        }
        ZeroCopyString zcSpeed = zcParams.SplitOffHead('/');
        if (zcSpeed.IsEmpty()) { return HALOperationResult::StringRequestParameterError; }
        if (zcSpeed.ValidUINT() == false) { return HALOperationResult::StringRequestParameterError; }
        uint32_t speed = 0;
        zcSpeed.ConvertTo_uint32(speed);
        if (speed == 0) { return HALOperationResult::StringRequestParameterError; }
#if defined(ESP32)
        if (static_cast<I2C_Master*>(device)->wire->setClock(speed) == false) return HALOperationResult::ExecutionFailed;
#else
        static_cast<I2C_Master*>(device)->wire->setClock(speed);
#endif
        return HALOperationResult::Success;
    }

    /* static */
    HALOperationResult I2C_Master::read_raw(Device* device, ZeroCopyString zcParams, StringBuilderStreamer& sbs) {
        if (zcParams.IsEmpty()) { 
            return HALOperationResult::StringRequestParameterError;
        }
        ZeroCopyString zcAddr = zcParams.SplitOffHead('/');
        if (zcAddr.ValidUINT() == false) { return HALOperationResult::StringRequestParameterError; }
        uint32_t bytesToRead = 0;
        if (zcParams.IsEmpty()) {
            bytesToRead = 1;
        } else {
            ZeroCopyString zcByteCount = zcParams.SplitOffHead('/'); // make this safe in case there are additonal / parameters that should just be ignored
            // the following could default to one byte to read, 
            // but if it's specified with a additional parameter
            // it's best to return a error so that the user don't expect anything else
            if (zcByteCount.ValidUINT() == false) return HALOperationResult::StringRequestParameterError;
            zcByteCount.ConvertTo_uint32(bytesToRead);
        }
        uint32_t addr;
        zcAddr.ConvertTo_uint32(addr);
        sbs.write_jsonMemberStart(F("items"));
        sbs.write_json_array_begin();

        uint8_t received = static_cast<I2C_Master*>(device)->wire->requestFrom((uint8_t)addr, (uint8_t)bytesToRead);
        if (received == 0) return HALOperationResult::ExecutionFailed;
        for (uint8_t i = 0; i < received; ++i) {
            uint8_t byte = static_cast<I2C_Master*>(device)->wire->read();
            if (i > 0) { sbs.write_json_value_separator(); }

            sbs.write(F("\"0x"));
            sbs.write_asHex(byte);
            sbs.write_doublequote();
        }
        sbs.write_json_array_end();
        return HALOperationResult::Success;
    }

    /* static */
    HALOperationResult I2C_Master::list_devices(Device* device, ZeroCopyString zcParams, StringBuilderStreamer& sbs) {
        sbs.write_jsonMemberStart(F("items"));
        sbs.write_json_array_begin();
        I2C_Master& self = *static_cast<I2C_Master*>(device);
        for (uint8_t addr=1; addr<127; ++addr) {
            self.wire->beginTransmission(addr);
            if (self.wire->endTransmission() == 0) {
                if (addr > 1) { sbs.write_json_value_separator(); }
                sbs.write(F("\"0x"));
                sbs.write_asHex(addr);
                sbs.write_doublequote(); sbs.write_char(':'); sbs.write_doublequote();
                describeI2CAddress(addr, sbs);
                sbs.write_doublequote();
            }
        }
        sbs.write_json_array_end();
        return HALOperationResult::Success;
    }

}