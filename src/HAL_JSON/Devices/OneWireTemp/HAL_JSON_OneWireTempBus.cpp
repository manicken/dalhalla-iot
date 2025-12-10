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

#include "HAL_JSON_OneWireTempBus.h"

namespace HAL_JSON {

    

    bool OneWireTempBus::VerifyJSON(const JsonVariant &jsonObj) {
        
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS) == false) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS));
            return false;
        }
        if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
            return false;
        }
        const JsonArray& items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY("OneWireTempBus")); return false;}
        int itemCount = (int)items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) continue; // comment item
            if (Device::DisabledInJson(item) == true) continue; // disabled
            if (OneWireTempDevice::VerifyJSON(item) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            validItemCount++;
        }
        if (validItemCount == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("OneWireTempBus")); return false; }
        
        //if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN)); return false; }
        //if (jsonObj[HAL_JSON_KEYNAME_PIN].is<uint8_t>() == false)  { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PIN)); return false; }
        
        //uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>(); 
        //return GPIO_manager::CheckIfPinAvailableAndReserve(pin, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
    }

    OneWireTempBus::OneWireTempBus(const JsonVariant &jsonObj, const char* type) : Device(type) {
        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);//].as<const char*>();
        uid = encodeUID(uidStr);
        pin = GetAsUINT32(jsonObj,HAL_JSON_KEYNAME_PIN);//].as<uint8_t>();
        GPIO_manager::ReservePin(pin); // this is in most cases taken care of in OneWireTempBus::VerifyJSON but there are situations where it's needed

        oneWire = new OneWire(pin);
        dTemp = new DallasTemperature(oneWire);
        dTemp->setWaitForConversion(false);

        deviceCount = 0;
        const JsonArray& items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        int itemCount = items.size();
        bool* validDevices = new bool[itemCount];
        // first pass count valid devices
        for (int i=0;i<itemCount;i++) {
            const JsonVariant& item = items[i];
            if (IsConstChar(item) == true) { validDevices[i] = false; continue; } // comment item
            if (Device::DisabledInJson(item) == true) { validDevices[i] = false; continue; } // disabled
            bool valid = OneWireTempDevice::VerifyJSON(items[i]);
            validDevices[i] = valid;
            if (valid == false) continue;
            deviceCount++;
        }
        devices = new (std::nothrow) Device*[deviceCount]();
        uint32_t index = 0;
        for (int i=0;i<itemCount;i++) {
            if (validDevices[i] == false) continue;
            devices[index++] = new OneWireTempDevice(static_cast<const JsonVariant&>(items[i]), type); // here type is not used so we just take it from bus
        }
        delete[] validDevices;
    }

    OneWireTempBus::~OneWireTempBus() {
        if (devices != nullptr) {
            for (int i=0;i<deviceCount;i++) {
                delete devices[i];
                devices[i] = nullptr;
            }
            delete[] devices;
            devices = nullptr;
        }
        delete dTemp;
        delete oneWire;
        
        pinMode(pin, INPUT); // "free" the pin
    }

    void OneWireTempBus::requestTemperatures()
    {
        dTemp->requestTemperatures();
    }

    void OneWireTempBus::readAll()
    {
        for (int i=0;i<deviceCount;i++) {
            OneWireTempDevice* device = static_cast<OneWireTempDevice*>(devices[i]); // cast for fast exec not using vtable lockup
            device->read(*dTemp);
        }
    }

    DeviceFindResult OneWireTempBus::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(devices, deviceCount, path, this, outDevice);
    }

    HALOperationResult OneWireTempBus::read(const HALReadStringRequestValue& val) {
        if (val.cmd == "getAllNewDevices") { // (as json) return a list of all new devices found for all busses (this will compare against the current ones and only print new ones)
            val.out_value += getAllDevices(false, true);
            return HALOperationResult::Success;
        }
        else if (val.cmd == "getAllNewDevicesWithTemp") { // (as json) return a list of all new devices found for all busses (this will compare against the current ones and only print new ones)
            val.out_value += getAllDevices(true, true);
            return HALOperationResult::Success;
        }
        else if (val.cmd == "getAllDevices") { // (as json) return a complete list of all devices found for all busses
            val.out_value += getAllDevices(false, false);
            return HALOperationResult::Success;
        }
        else if (val.cmd == "getAllTemperatures") { // (as json) return a complete list of all temperatures each with it's uid as the keyname and the temp as the value
            val.out_value += getAllDevices(true, false);
            return HALOperationResult::Success;
        }
        std::string stdStrCmd = val.cmd.ToString();
        GlobalLogger.Warn(F("OneWireTempBus::read - cmd not found: "), stdStrCmd.c_str()); // this can then be read by getting the last entry from logger
        return HALOperationResult::UnsupportedCommand;
    }

    bool OneWireTempBus::haveDeviceWithRomID(OneWireAddress addr) {
        if (deviceCount == 0 || devices == nullptr) return false;
        for (int i=0;i<deviceCount;i++) {
            
            OneWireTempDevice* device = static_cast<OneWireTempDevice*>(devices[i]); // cast for special access
            if (device->romid.id == addr.id) return true;
        }
        return false;
    }

    std::string OneWireTempBus::getAllDevices(bool printTemp, bool onlyNewDevices) {
        //byte i = 0;
        uint8_t done = 0;
        OneWireAddress addr;
        std::string returnStr;
        char hexString[3];
        bool first = true;

        returnStr.append("{\"pin\":");
        returnStr += std::to_string(pin);
        returnStr.append(",\"items\":[");
        if (printTemp) {
            dTemp->setWaitForConversion(true);
            dTemp->requestTemperatures();
            dTemp->setWaitForConversion(false);
        }
        
        oneWire->reset_search();
        while(!done)
        {
            if (oneWire->search(addr.bytes) != 1)
            {
                
                oneWire->reset_search();
                done = 1;
            }
            else
            {
                if (onlyNewDevices && haveDeviceWithRomID(addr)==true) continue;

                if (!first) {
                    returnStr.append(",");
                }
                if (printTemp) {
                    returnStr.append("{\"romId\":");
                }
                returnStr.append("\"");
                for(int i = 0; i < 7; i++) 
                {
                    sprintf(hexString, "%02X", addr.bytes[i]);
                    returnStr.append(hexString);
                    returnStr.append(":");
                }
                sprintf(hexString, "%02X", addr.bytes[7]);
                returnStr.append(hexString);
                returnStr.append("\"");
                if (printTemp) {
                    returnStr.append(",\"tempC\":");
                    returnStr += std::to_string(dTemp->getTempC(addr.bytes));
                    returnStr.append(",\"tempF\":");
                    returnStr += std::to_string(dTemp->getTempF(addr.bytes));
                    returnStr.append("}");
                }
                first = false;
            }
        }
        
        returnStr.append("]}");
        return returnStr;
    }

    String OneWireTempBus::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
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

    OneWireTempBusAtRoot::OneWireTempBusAtRoot(const JsonVariant &jsonObj, const char* type) 
    : OneWireTempBus(jsonObj, type),
        autoRefresh(
              [this]() { requestTemperatures(); },
              [this]() { readAll(); },
              ParseRefreshTimeMs(jsonObj,HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS)
        )
    {
        
    }

    OneWireTempBusAtRoot::~OneWireTempBusAtRoot() {
        
    }

    void OneWireTempBusAtRoot::loop() {
        autoRefresh.loop();
    }

    String OneWireTempBusAtRoot::ToString() {
        String ret;
        //ret += DeviceConstStrings::uid;
        //ret += decodeUID(uid).c_str();
        //ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += OneWireTempBus::ToString();
        ret += autoRefresh.ToString();
        
        return ret;
    }

    Device* OneWireTempBusAtRoot::Create(const JsonVariant &jsonObj, const char* type) {
        return new OneWireTempBusAtRoot(jsonObj, type);
    }
}