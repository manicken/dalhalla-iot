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
        Create,
        &JsonSchema::I2C_Master::Root,
        DALHAL_REACTIVE_EVENT_TABLE(I2C_MASTER)
    };
    //volatile const void* keep_I2C_Master = &DALHAL::I2C_Master::RegistryDefine;

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

    String I2C_Master::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += ",\"SDA PIN\":";
        ret += std::to_string(sdapin).c_str();
        ret += ",\"SCK PIN\":";
        ret += std::to_string(sckpin).c_str();
        ret += ",\"freq\":";
        ret += std::to_string(freq).c_str();
        ret += ",\"devices\":[";
        for (int i=0;i<deviceCount;i++) {
            ret += "{";
            ret += devices[i]->ToString();
            ret += "}";
            if (i<deviceCount-1) ret += ",";
        }
        ret += "]";
        return ret;
    }

    DeviceFindResult I2C_Master::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(devices, deviceCount, path, this, outDevice);
    }

    void I2C_Master::loop() {
        for (int i=0;i<deviceCount;i++) {
            devices[i]->loop();
        }
    }

    HALOperationResult I2C_Master::read(const HALReadStringRequestValue& val) {
        StringBuilderStreamer& sbs = val.sbs;
        ZeroCopyString zcStr = val.cmd; // make copy
        ZeroCopyString zcCmd = zcStr.SplitOffHead('/');
        if (zcCmd.EqualsIC(F("raw"))) { // this is more likely to be called
            if (zcStr.IsEmpty()) return HALOperationResult::StringRequestParameterError;
            ZeroCopyString zcAddr = zcStr.SplitOffHead('/');
            if (zcAddr.ValidUINT() == false) return HALOperationResult::StringRequestParameterError;
            uint32_t bytesToRead = 0;
            if (zcStr.IsEmpty()) bytesToRead = 1;
            else {
                ZeroCopyString zcByteCount = zcStr.SplitOffHead('/'); // make this safe in case there are additonal / parameters that should just be ignored
                // the following could default to one byte to read, 
                // but if it's specified with a additional parameter
                // it's best to return a error so that the user don't expect anything else
                if (zcByteCount.ValidUINT() == false) return HALOperationResult::StringRequestParameterError;
                zcByteCount.ConvertTo_uint32(bytesToRead);
            }
            uint32_t addr;
            zcAddr.ConvertTo_uint32(addr);
            
            sbs.write('[');

            uint8_t received = wire->requestFrom((uint8_t)addr, (uint8_t)bytesToRead);
            if (received == 0) return HALOperationResult::ExecutionFailed;
            for (uint8_t i = 0; i < received; ++i) {
                uint8_t byte = wire->read();
                if (i > 0) { sbs.write(','); }

                sbs.write(F("\"0x"));
                sbs.write_asHex(byte);
                sbs.write('"');
            }
            sbs.write(']');
        }
        else if (zcCmd.EqualsIC(F("list"))) {
            sbs.write('{');
            bool first = true;
            for (uint8_t addr=1; addr<127; ++addr) {
                wire->beginTransmission(addr);
                if (wire->endTransmission() == 0) {
                    if (addr > 1) { sbs.write(','); }
                    sbs.write(F("\"0x"));
                    sbs.write_asHex(addr);
                    sbs.write('"'); sbs.write(':'); sbs.write('"');
                    describeI2CAddress(addr, sbs);
                    sbs.write('"');
                }
            }
            sbs.write('}');
            
        } else {
            return HALOperationResult::UnsupportedCommand;
        }
#if HAS_REACTIVE_READ(I2C_MASTER)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult I2C_Master::write(const HALWriteStringRequestValue& val) {
        ZeroCopyString zcStr = val.value; // make copy
        ZeroCopyString zcCmd = zcStr.SplitOffHead('/');
        if (zcCmd.EqualsIC(F("raw"))) {
            if (zcStr.IsEmpty()) return HALOperationResult::StringRequestParameterError;
            ZeroCopyString zcAddr = zcStr.SplitOffHead('/');
            if (zcStr.IsEmpty()) return HALOperationResult::StringRequestParameterError; // simple early check
            if (zcAddr.ValidUINT() == false) return HALOperationResult::StringRequestParameterError;
            ZeroCopyString zcByteCount = zcStr.SplitOffHead('/');
            if (zcByteCount.ValidUINT() == false) return HALOperationResult::StringRequestParameterError;
            uint32_t bytesToWrite = 0;
            zcByteCount.ConvertTo_uint32(bytesToWrite);
            if (bytesToWrite == 0) return HALOperationResult::StringRequestParameterError;
            
            uint32_t paramCount = zcStr.CountChar('/')+1; // +1 to make it easier/clearer
            if (paramCount < bytesToWrite) return HALOperationResult::StringRequestParameterError;
            uint32_t addr = 0;
            zcAddr.ConvertTo_uint32(addr);
            wire->beginTransmission((uint8_t)addr);
            while (bytesToWrite--) {
                ZeroCopyString zcByte = zcStr.SplitOffHead('/');
                if (zcByte.ValidUINT() == false) {
                    wire->endTransmission(true);
                    return HALOperationResult::StringRequestParameterError;
                }
                uint32_t byteVal  = 0;
                zcByte.ConvertTo_uint32(byteVal );
                wire->write((uint8_t)byteVal );
            }
            uint8_t res = wire->endTransmission(true);
            if (res != 0) {
                val.result.write((char)(0x30 + res));
                return HALOperationResult::ExecutionFailed;
            }
            if (zcStr.IsEmpty() == false) {
                // this would mean that this is a write read request
                // and the current parameter is the number of bytes to read
                // currently a TODO feature
                // and the read function need to be DRY first
            }
        } else if (zcCmd.EqualsIC(F("speed"))) {
            if (zcStr.IsEmpty()) return HALOperationResult::StringRequestParameterError;
            ZeroCopyString zcSpeed = zcStr.SplitOffHead('/');
            if (zcSpeed.IsEmpty()) return HALOperationResult::StringRequestParameterError;
            if (zcSpeed.ValidUINT() == false) return HALOperationResult::StringRequestParameterError;
            uint32_t speed = 0;
            zcSpeed.ConvertTo_uint32(speed);
            if (speed == 0) return HALOperationResult::StringRequestParameterError;
#if defined(ESP32)
            if (wire->setClock(speed) == false) return HALOperationResult::ExecutionFailed;
#else
            wire->setClock(speed);
#endif
        } else {
            return HALOperationResult::UnsupportedCommand;
        }
#if HAS_REACTIVE_WRITE(I2C_MASTER)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

}