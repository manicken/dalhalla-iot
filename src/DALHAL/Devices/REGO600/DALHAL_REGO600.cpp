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

#include "DALHAL_REGO600.h"

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Support/ConvertHelper.h>

#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include "DALHAL_REGO600_JSON_Schema.h"

namespace DALHAL {

    constexpr Registry::DefineBase REGO600::RegistryDefine = {
        Create,
        &JsonSchema::REGO600::Root,
        DALHAL_REACTIVE_EVENT_TABLE(REGO600)
    };
    
    REGO600::REGO600(DeviceCreateContext& context) : REGO600_DeviceBase(context.deviceType) {
        JsonSchema::REGO600::Extractors::Apply(context, this);
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
#if HAS_REACTIVE_BEGIN(REGO600)
        triggerBegin();
#endif
    }
    void REGO600::loop() {
        rego600->loop();
#if HAS_REACTIVE_CYCLE_COMPLETE(REGO600)
        if (rego600->RefreshLoopDone()) { // one hit flag check and clear if set
            triggerCycleComplete();
        }
#endif
    }

    Device* REGO600::Create(DeviceCreateContext& context) {
        return new REGO600(context);
    }

    String REGO600::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
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