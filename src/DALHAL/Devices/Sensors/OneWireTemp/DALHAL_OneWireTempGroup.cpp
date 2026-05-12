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

#include "DALHAL_OneWireTempGroup.h"

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>
#include "DALHAL_OneWireTempGroup_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase OneWireTempGroup::RegistryDefine = {
        Create,
        &JsonSchema::OneWireTempGroup::Root,
        DALHAL_REACTIVE_EVENT_TABLE(ONE_WIRE_TEMP_GROUP)
    };
    //volatile const void* keep_OneWireTempGroup = &DALHAL::OneWireTempGroup::RegistryDefine;

    Device* OneWireTempGroup::Create(DeviceCreateContext& context) {
        return new OneWireTempGroup(context);
    }

    OneWireTempGroup::OneWireTempGroup(DeviceCreateContext& context) : OneWireTempGroup_DeviceBase(context.deviceType),
        autoRefresh(
            [this]() { requestTemperatures(); },
            [this]() { readAll(); }
        ), 
        busses(nullptr)
    {
        JsonSchema::OneWireTempGroup::Extractors::Apply(context, this);
        uint32_t refreshMs = JsonSchema::CommonTime::refreshTimeGroupFieldsRequired.ExtractFrom(*(context.jsonObjItem)).toUInt();
        autoRefresh.SetRefreshTimeMs(refreshMs);
    }
    OneWireTempGroup::~OneWireTempGroup() {
        if (busses != nullptr) {
            for (int i=0;i<busCount;i++) {
                delete busses[i];
                busses[i] = nullptr;
            }
            delete[] busses;
            busses = nullptr;
        }
    }

    DeviceFindResult OneWireTempGroup::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(busses, busCount, path, this, outDevice);
    }

    HALOperationResult OneWireTempGroup::read(const HALReadStringRequestValue& val) {
        if (val.cmd.EqualsIC(F("getAllNewDevices"))) { // (as json) return a list of all new devices found for all busses (this will compare against the current ones and only print new ones)
            val.out_value = '[';
            for (int i=0;i<busCount;i++) {
                OneWireTempBus* bus = static_cast<OneWireTempBus*>(busses[i]); // cast for direct call not using vtable lockup
                if (bus == nullptr) continue;
                bus->read(val);
                if (i<busCount-1)
                    val.out_value += ',';
            }
            val.out_value += ']';

        }
        else if (val.cmd.EqualsIC(F("getAllNewDevicesWithTemp"))) {
            val.out_value = '[';
            for (int i=0;i<busCount;i++) {
                OneWireTempBus* bus = static_cast<OneWireTempBus*>(busses[i]); // cast for direct call not using vtable lockup
                if (bus == nullptr) continue;
                bus->read(val);
                if (i<busCount-1)
                    val.out_value += ',';
            }
            val.out_value += ']';

        }
        else if (val.cmd.EqualsIC(F("getAllDevices"))) { // (as json) return a complete list of all devices found for all busses
            val.out_value = '[';
            for (int i=0;i<busCount;i++) {
                OneWireTempBus* bus = static_cast<OneWireTempBus*>(busses[i]); // cast for direct call not using vtable lockup
                if (bus == nullptr) continue;
                bus->read(val);
                if (i<busCount-1)
                    val.out_value += ',';
            }
            val.out_value += ']';

        }
        else if (val.cmd.EqualsIC(F("getAllTemperatures"))) { // (as json) return a complete list of all temperatures each with it's uid as the keyname and the temp as the value
            val.out_value = '[';
            for (int i=0;i<busCount;i++) {
                OneWireTempBus* bus = static_cast<OneWireTempBus*>(busses[i]); // cast for direct call not using vtable lockup
                if (bus == nullptr) continue;
                bus->read(val);
                if (i<busCount-1)
                    val.out_value += ',';
            }
            val.out_value += ']';
            
        } else {
            std::string stdStrCmd = val.cmd.ToString();
            GlobalLogger.Warn(F("OneWireTempGroup::read - cmd not found: "), stdStrCmd.c_str()); // this can then be read by getting the last entry from logger
            //val.out_value = F("{\"error\":\"cmd not found\"}");
            return HALOperationResult::UnsupportedCommand;  // cmd not found
        }
#if HAS_REACTIVE_READ(ONE_WIRE_TEMP_GROUP)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }

    void OneWireTempGroup::requestTemperatures() {
        for (int i=0;i<busCount;i++) {
            OneWireTempBus* bus = static_cast<OneWireTempBus*>(busses[i]); // cast because need of non generic function
            bus->requestTemperatures();
        }
    }

    void OneWireTempGroup::readAll() {
        for (int i=0;i<busCount;i++) {
            OneWireTempBus* bus = static_cast<OneWireTempBus*>(busses[i]); // cast because need of non generic function
            bus->readAll();
        }
#if HAS_REACTIVE_CYCLE_COMPLETE(ONE_WIRE_TEMP_GROUP)
        triggerCycleComplete();
#endif
    }

    void OneWireTempGroup::loop() {
        autoRefresh.loop();
    }

    String OneWireTempGroup::ToString() {
        String ret;
        
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += '"'; ret += ',';
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += '"';
        ret += autoRefresh.ToString();
        ret += F(",\"busses\":[");
        for (int i=0;i<busCount;i++) {
            ret += '{';
            ret += busses[i]->ToString();
            ret += '}';
            if (i<busCount-1) ret += ',';
        }
        ret += ']';
        return ret;
    }
}