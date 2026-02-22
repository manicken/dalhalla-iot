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

#include "HAL_JSON_REGO600.h"

namespace HAL_JSON {
    
    REGO600::REGO600(const JsonVariant &jsonObj, const char* type) : Device(type) {
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);

        rxPin = GetAsUINT8(jsonObj, HAL_JSON_KEYNAME_RXPIN);
        txPin = GetAsUINT8(jsonObj, HAL_JSON_KEYNAME_TXPIN);
        GPIO_manager::ReservePin(rxPin);
        GPIO_manager::ReservePin(txPin);

        JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS];
        int itemCount = items.size();
        bool* validItems = new bool[itemCount];
        registerItemCount = 0;
        // first pass count valid items
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) { validItems[i] = false; continue; } // comment item
            if (Device::DisabledInJson(item) == true) { validItems[i] = false; continue; } // disabled
#ifndef HAL_JSON_VALIDATE_JSON_STRICT
            if (REGO600register::VerifyJSON(item) == false) { validItems[i] = false; continue; }
#endif
            validItems[i] = true;
            registerItemCount++;
        }
        // second pass
        requestList = new (std::nothrow) Drivers::REGO600::Request*[registerItemCount]();
        registerItems = new (std::nothrow) Device*[registerItemCount]();
        int index = 0;
        for (int i=0;i<itemCount;i++) {
            if (validItems[i] == false) continue;
            const JsonVariant& item = items[i];
            REGO600register* itemReg = new REGO600register(item, nullptr);

            const char* regName = GetAsConstChar(item, "regname");
            const Drivers::REGO600::RegoLookupEntry* entry = Drivers::REGO600::SystemRegisterTableLockup(regName);
            if (regName == nullptr) {
                GlobalLogger.Error(F("regName not found"));
            }
            // here value is passed by ref so that REGO600 driver can access and change the value,
            // that makes REGO600register read function can then get the correct value
            const Drivers::REGO600::OpCodeInfo& info = Drivers::REGO600::getCmdInfo(0x02); // system registers only
            requestList[index] = new Drivers::REGO600::Request(
                info, 
                (entry != nullptr) ? *entry : Drivers::REGO600::ManualRawEntry, // should not be needed
                itemReg->value
            );
            registerItems[index] = itemReg;
            index++;
        }
        delete[] validItems;
        refreshTimeMs = ParseRefreshTimeMs(jsonObj, 0); // default to zero, note here REGO600 constructor will calculate minimum based on registerItemCount
        rego600 = new Drivers::REGO600(rxPin, txPin, requestList, registerItemCount, refreshTimeMs);
    }

    REGO600::~REGO600() {
        if (rego600 != nullptr)
            delete rego600;
        if (requestList != nullptr) { // if for example the allocation did fail
            for (int i=0;i<registerItemCount; i++) {
                delete requestList[i];
            }
            delete[] requestList;
        }
        if (registerItems != nullptr) { // if for example the allocation did fail
            for (int i=0;i<registerItemCount; i++) {
                delete registerItems[i];
            }
            delete[] registerItems;
        }
        pinMode(rxPin, INPUT); // input
        pinMode(txPin, INPUT); // input
    }
    void REGO600::begin() {
        rego600->begin(); // this will initialize a first request
    }
    void REGO600::loop() {
        rego600->loop();
    }

    bool REGO600::VerifyJSON(const JsonVariant &jsonObj) {
        if (!ValidateUINT8(jsonObj,HAL_JSON_KEYNAME_RXPIN)) return false;
        if (!ValidateUINT8(jsonObj,HAL_JSON_KEYNAME_TXPIN)) return false;
        uint8_t rxPin = GetAsUINT8(jsonObj, HAL_JSON_KEYNAME_RXPIN);
        if (!GPIO_manager::CheckIfPinAvailableAndReserve(rxPin, static_cast<uint8_t>(GPIO_manager::PinFunc::IN))) return false;
        uint8_t txPin = GetAsUINT8(jsonObj, HAL_JSON_KEYNAME_TXPIN);
        if (!GPIO_manager::CheckIfPinAvailableAndReserve(txPin, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT))) return false;
        if (!jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS)) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS)); return false; }
        
        if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
            return false;
        }
        const JsonArray& items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY("REGO600")); return false;}
        int itemCount = items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) continue; // comment item
            if (Device::DisabledInJson(item) == true) continue; // disabled
            if (REGO600register::VerifyJSON(item) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            validItemCount++;
        }
        if (validItemCount == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("REGO600")); return false; }
        return true;
    }

    Device* REGO600::Create(const JsonVariant &jsonObj, const char* type) {
        return new REGO600(jsonObj, type);
    }

    String REGO600::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += "\"items\":[";
        bool first = true;
        for (int i=0;i<registerItemCount;i++) {
            if (first == false) { ret += ","; }
            else
                first = false;
            ret += '{';
            ret += registerItems[i]->ToString();
            ret += ",\"opcode\":\"";
            ret += Convert::toHex((uint8_t)requestList[i]->info.opcode).c_str();
            ret += "\",\"addr\":\"";
            ret += Convert::toHex(requestList[i]->def.address).c_str();
            ret += "\",";
            ret += registerItems[i]->ToString();
            ret += "}";
        }
        ret += ']';
        return ret;
    }

    DeviceFindResult REGO600::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(registerItems, registerItemCount, path, this, outDevice);
    }

}