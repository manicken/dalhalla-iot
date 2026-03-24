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

#include "DALHAL_TX433.h"

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Base.h>
#include "DALHAL_TX433_UnitTypeRegistry.h"

#include "DALHAL_TX433_JSON_Scheme.h"

namespace DALHAL {

    constexpr Registry::DefineBase TX433::RegistryDefine = {
        Create,
        &JsonSchema::TX433,
        DALHAL_REACTIVE_EVENT_TABLE(TX433)
    };

    Device* TX433::Create(DeviceCreateContext& context) {
        return new TX433(context);
    }
    TX433::TX433(DeviceCreateContext& context) : TX433_DeviceBase(context.deviceType) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
        const char* uidStr = jsonObj[DALHAL_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
        pin = GetAsUINT32(jsonObj,DALHAL_KEYNAME_PIN);//].as<uint8_t>();
        GPIO_manager::ReservePin(pin); // in case we forgot to do it somewhere
        if (jsonObj.containsKey(DALHAL_KEYNAME_TX433_UNITS) && jsonObj[DALHAL_KEYNAME_TX433_UNITS].is<JsonArray>()) {
            JsonArray _units = jsonObj[DALHAL_KEYNAME_TX433_UNITS].as<JsonArray>();
            int _unitCount = _units.size();
            bool* validUnits = new bool[_unitCount];
            unitCount = 0;
            // first pass count valid units(devices)
            for (int i=0;i<_unitCount;i++) {
                const JsonVariant& unit = _units[i];
                if (IsConstChar(unit) == true) { validUnits[i] = false;  continue; }  // comment item
                if (Device::DisabledInJson(unit) == true) { validUnits[i] = false;  continue; } // disabled
                validUnits[i] = true; // allways valid in strict mode
                unitCount++;
            }
            // second pass create units(devices)
            units = new Device*[unitCount](); /*TX433unit*/
            uint32_t index = 0;
            TX433_Unit_CreateFunctionContext createContext(pin);
            for (int i=0;i<_unitCount;i++) {
                if (validUnits[i] == false) continue;
                const JsonVariant& item = _units[i];
                createContext.jsonObjItem = &item;
                const char* type_cStr = item[DALHAL_COMMON_CFG_NAME_TYPE].as<const char*>();
                const Registry::Item& regItem = Registry::GetItem(TX433_UnitTypeRegistry, type_cStr);
                // here it's safe to use regItem as JSON validation ensure that device type exists
                createContext.deviceType = regItem.typeName;

                units[index++] = regItem.def->Create_Function(createContext);
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

    DeviceFindResult TX433::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(units, unitCount, path, this, outDevice);
    }

    HALOperationResult TX433::write(const HALWriteStringRequestValue &val) {
        RF433::init(pin); // this only sets the pin and set the pin to output
        std::string stdStrCmd = val.value.ToString();
        
        RF433::DecodeFromJSON(stdStrCmd); // TODO make this function take ZeroCopyString as argument, even thu it's copied internally
        // TODO better error check from DecodeFromJSON
#if HAS_REACTIVE_WRITE(TX433)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    String TX433::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
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