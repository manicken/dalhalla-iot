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

#include "HAL_JSON_I2C_BUS.h"
#include "HAL_JSON_I2C_BUS_DeviceTypeReg.h"

namespace HAL_JSON {

    
    I2C_BUS::I2C_BUS(const JsonVariant &jsonObj, const char* type) : Device(type) {
        deviceCount = 0;
        devices = nullptr;

        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);

        sckpin = GetAsUINT8(jsonObj, "sckpin");
        sdapin = GetAsUINT8(jsonObj, "sdapin");
        freq = GetAsUINT32(jsonObj, "freq");
        if (freq < 100000) freq = 100000; // defaults to 100khz

        int busIndex = GetAsUINT8(jsonObj, "busindex");
#if defined(ESP32)
        if (busIndex == 1)
            wire = &Wire1;
        else
#endif
            wire = &Wire;

        wire->begin(sdapin, sckpin, freq);

        const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();

        int itemCount = items.size();
        bool* validItems = new bool[itemCount];
        // first pass count valid items
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant& item = items[i];
            if (IsConstChar(item) == true) { validItems[i] = false; continue; }// comment item
            if (Device::DisabledInJson(item) == true) { validItems[i] = false; continue; } // disabled
            if (ValidateJsonStringField(item, HAL_JSON_KEYNAME_TYPE) == false) { validItems[i] = false; continue; }
            
            const char* type = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
            
            const I2C_DeviceTypeDef* def = GetI2C_DeviceTypeDef(type);
            // no nullcheck is needed as ValidateJSON ensures that all types are correct
            if (def->Verify_JSON_Function(item) == false) { validItems[i] = false; continue; }
            validItemCount++;
            validItems[i] = true;
        }
        // second pass actually create the devices
        deviceCount = validItemCount;
        devices = new Device*[validItemCount]();
        int index = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (validItems[i] == false) continue;
            
            const char* type = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
            const I2C_DeviceTypeDef* def = GetI2C_DeviceTypeDef(type);
             // no nullcheck is needed as ValidateJSON ensures that all types are correct
            devices[index++] = def->Create_Function(item, "SSD1306", *wire);
        }
        delete[] validItems;
    }
    I2C_BUS::~I2C_BUS() {
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

    bool I2C_BUS::VerifyJSON(const JsonVariant &jsonObj) {
        if (!GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, "sckpin", static_cast<uint8_t>(GPIO_manager::PinMode::OUT))) {
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        if (!GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, "sdapin", static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN))) {
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS) == false) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY());
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        int itemCount = items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) continue; // comment item
            if (Device::DisabledInJson(item) == true) continue; // disabled
            if (ValidateJsonStringField(item, HAL_JSON_KEYNAME_TYPE) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            
            const char* type = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
            const I2C_DeviceTypeDef* def = GetI2C_DeviceTypeDef(type);
            if (def == nullptr) {
                GlobalLogger.Error(F("VerifyI2CDeviceJson - could not find type:"),type);
                SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
                return false;
            }
            
            if (def->Verify_JSON_Function == nullptr) {
                GlobalLogger.Error(F("VerifyI2CDeviceJson - Verify_JSON_Function nullptr:"),type);
                SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
                return false;
            }
            if (def->Create_Function == nullptr) {
                GlobalLogger.Error(F("VerifyI2CDeviceJson - Create_Function nullptr:"),type);
                SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
                return false;
            }
            if (def->Verify_JSON_Function(item) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            validItemCount++;

        }
        if (validItemCount == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("I2C_BUS"));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        return true;
    }

    Device* I2C_BUS::Create(const JsonVariant &jsonObj, const char* type) {
        return new I2C_BUS(jsonObj, type);
    }

    String I2C_BUS::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
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

    DeviceFindResult I2C_BUS::findDevice(UIDPath& path, Device*& outDevice) {
        //printf("\nI2C_BUS::findDevice: %s\n", path.ToString().c_str());
        return Device::findInArray(reinterpret_cast<Device**>(devices), deviceCount, path, this, outDevice);
    }

    void I2C_BUS::loop() {
        for (int i=0;i<deviceCount;i++) {
            devices[i]->loop();
        }
    }

    HALOperationResult I2C_BUS::read(const HALReadStringRequestValue& val) {
        ZeroCopyString zcStr = val.cmd; // make copy
        ZeroCopyString zcCmd = zcStr.SplitOffHead('/');
        if (zcCmd == "raw") { // this is more likely to be called
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
            val.out_value.reserve(2+bytesToRead*7/*-1+1*/); // ["0x00","0x01"] 2+ is the [] -1 is to remove the last , and +1 is the null char
            val.out_value = '[';
            bool first = true;
            uint8_t received = wire->requestFrom((uint8_t)addr, (uint8_t)bytesToRead);
            if (received == 0) return HALOperationResult::ExecutionFailed;
            for (uint8_t i = 0; i < received; ++i) {
                uint8_t byte = wire->read();
                if (first == false) val.out_value += ',';
                else first = false;
                val.out_value += "\"0x";
                val.out_value += Convert::toHex(byte);
                val.out_value += "\"";
            }
            val.out_value += ']';
            return HALOperationResult::Success;
        }
        else if (zcCmd == "list") {
            val.out_value = '{';
            bool first = true;
            for (uint8_t addr=1; addr<127; ++addr) {
                wire->beginTransmission(addr);
                if (wire->endTransmission() == 0) {
                    if (first == false) val.out_value += ',';
                    else if (first) first = false;
                    val.out_value += "\"0x";
                    val.out_value += Convert::toHex(addr);
                    val.out_value += "\":\"";
                    val.out_value += describeI2CAddress(addr);
                    val.out_value += "\"";
                }
            }
            val.out_value += '}';
            return HALOperationResult::Success;
        }
        return HALOperationResult::UnsupportedCommand;
    }

    HALOperationResult I2C_BUS::write(const HALWriteStringRequestValue& val) {
        ZeroCopyString zcStr = val.value; // make copy
        ZeroCopyString zcCmd = zcStr.SplitOffHead('/');
        if (zcCmd == "raw") {
            if (zcStr.IsEmpty()) return HALOperationResult::StringRequestParameterError;
            ZeroCopyString zcAddr = zcStr.SplitOffHead('/');
            if (zcStr.IsEmpty()) return HALOperationResult::StringRequestParameterError; // simple early check
            if (zcAddr.ValidUINT() == false) return HALOperationResult::StringRequestParameterError;
            ZeroCopyString zcByteCount = zcStr.SplitOffHead('/');
            if (zcByteCount.ValidUINT() == false) return HALOperationResult::StringRequestParameterError;
            uint32_t bytesToWrite = 0;
            zcByteCount.ConvertTo_uint32(bytesToWrite);
            if (bytesToWrite == 0) return HALOperationResult::StringRequestParameterError;
            
            int paramCount = zcStr.CountChar('/')+1; // +1 to make it easier/clearer
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
                val.result = 0x30 + res;
                return HALOperationResult::ExecutionFailed;
            }
            if (zcStr.IsEmpty() == false) {
                // this would mean that this is a write read request
                // and the current parameter is the number of bytes to read
                // currently a TODO feature
                // and the read function need to be DRY first
            }
            return HALOperationResult::Success;
        } else if (zcCmd == "speed") {
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
            return HALOperationResult::Success;
        }
        return HALOperationResult::UnsupportedCommand;
    }

}