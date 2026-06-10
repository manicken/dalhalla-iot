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

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include "DALHAL_TX433_UnitTypeRegistry.h"

#include "DALHAL_TX433_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase TX433::RegistryDefine = {
        Create,
        &JsonSchema::TX433::Root,
        DALHAL_REACTIVE_EVENT_TABLE(TX433)
    };

    /* override */
    const Registry::DefineBase* TX433::GetRegistryDefine() {
        return &RegistryDefine;
    }

    Device* TX433::Create(DeviceCreateContext& context) {
        return new TX433(context);
    }

    TX433::TX433(DeviceCreateContext& context) : TX433_DeviceBase(context.deviceType) {
        JsonSchema::TX433::Extractors::Apply(context, this);
    }
    
    TX433::~TX433() {
        if (units != nullptr) {
            for (int i=0;i<unitCount;i++) {
                if (units[i] != nullptr) {
                    delete units[i];
                }
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
        std::string stdStrCmd = val.parameters.ToString();
        
        RF433::DecodeFromJSON(stdStrCmd); // TODO make this function take ZeroCopyString as argument, even thu it's copied internally
        // TODO better error check from DecodeFromJSON
#if HAS_REACTIVE_WRITE(TX433)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    void TX433::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
        
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pin"), pin);
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("units"));
        sbs.write_json_array_begin();

        for (int i=0;i<unitCount;i++) {
            if (i>0) { sbs.write_json_value_separator(); }
            
            sbs.write_json_object_begin();
            units[i]->PrintTo(sbs);
            sbs.write_json_object_end();
        }
        sbs.write_json_array_end();
    }

}