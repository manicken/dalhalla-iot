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

#include "HAL_JSON_TX433.h"

namespace HAL_JSON {

    bool TX433::VerifyJSON(const JsonVariant &jsonObj) {
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID) == false) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_TX433_VERIFY_JSON); return false; }

        if (jsonObj.containsKey(HAL_JSON_KEYNAME_TX433_UNITS) && jsonObj[HAL_JSON_KEYNAME_TX433_UNITS].is<JsonArray>()) {
            JsonArray units = jsonObj[HAL_JSON_KEYNAME_TX433_UNITS].as<JsonArray>();
            int unitCount = units.size();
            for (int i=0;i<unitCount;i++) {
                const JsonVariant& unit = units[i];
                if (IsConstChar(unit) == true) continue; // comment item
                if (Device::DisabledInJson(unit) == true) continue; // disabled
                if (TX433unit::VerifyJSON(unit) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            }
        }
        // this is a check only to verify that the pin cfg exist
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT)));
    }

    Device* TX433::Create(const JsonVariant &jsonObj, const char* type) {
        return new TX433(jsonObj, type);
    }
    TX433::TX433(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::Many,type) {
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
        pin = GetAsUINT32(jsonObj,HAL_JSON_KEYNAME_PIN);//].as<uint8_t>();
        GPIO_manager::ReservePin(pin); // in case we forgot to do it somewhere
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_TX433_UNITS) && jsonObj[HAL_JSON_KEYNAME_TX433_UNITS].is<JsonArray>()) {
            JsonArray _units = jsonObj[HAL_JSON_KEYNAME_TX433_UNITS].as<JsonArray>();
            int _unitCount = _units.size();
            bool* validUnits = new bool[_unitCount];
            unitCount = 0;
            // first pass count valid units(devices)
            for (int i=0;i<_unitCount;i++) {
                const JsonVariant& unit = _units[i];
                if (IsConstChar(unit) == true) { validUnits[i] = false;  continue; }  // comment item
                if (Device::DisabledInJson(unit) == true) { validUnits[i] = false;  continue; } // disabled
                bool valid = TX433unit::VerifyJSON(unit);
                validUnits[i] = valid;
                if (valid == false) continue;
                unitCount++;
            }
            // second pass create units(devices)
            units = new (std::nothrow) TX433unit*[unitCount]();
            uint32_t index = 0;
            for (int i=0;i<_unitCount;i++) {
                if (validUnits[i] == false) continue;
                const JsonVariant& unit = _units[i];
                units[index++] = new TX433unit(unit, nullptr, pin); // here type is not used so we just take it from current to avoid creating new const strings, or use nullstr do also work
            }
            delete[] validUnits;
        }


    }
    TX433::~TX433() {
        if (units != nullptr) {
            for (int i=0;i<unitCount;i++) {
                delete units[i];
                units[i] = nullptr;
            }
            delete[] units;
            units = nullptr;
        }
        pinMode(pin, INPUT); // reset to input so other devices can safely use it
    }

    Device* TX433::findDevice(UIDPath& path) {
        if (units == nullptr) return nullptr;
        else if (unitCount == 0) return nullptr;

        HAL_UID uidToFind = path.peekNextUID();
        if (uidToFind.Invalid()) { GlobalLogger.Error(F("TX433::findDevice - uidToFind is Invalid")); return nullptr; } // early break
        
        
        for (int i=0;i<unitCount;i++) {
            TX433unit* unit = units[i];
            if (!unit) continue; // absolute failsafe
            if (unit->uid == uidToFind) return unit;
        }
        return nullptr;
    }

    HALOperationResult TX433::write(const HALWriteStringRequestValue &val) {
        RF433::init(pin); // this only sets the pin and set the pin to output
        std::string stdStrCmd = val.value.ToString();
        RF433::DecodeFromJSON(stdStrCmd); // TODO make this function take ZeroCopyString as argument, even thu it's copied internally
        // TODO better error check from DecodeFromJSON
        return HALOperationResult::Success;
    }

    String TX433::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",\"units\":[";
        bool first = true;
        for (int i=0;i<unitCount;i++) {
            if (first == false)
                ret += ",";
            else
                first = false;
            ret += "{";
            ret += units[i]->ToString();
            ret += "}";            
        }
        ret += "]";
        return ret;
    }

}