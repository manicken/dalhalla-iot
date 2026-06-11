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

#include "DALHAL_OneWireTempBus.h"

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>
#include "DALHAL_OneWireTempBus_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase OneWireTempBusAtRoot::RegistryDefine = {
        OneWireTempBusAtRoot::Create,
        &JsonSchema::OneWireTempBusAtRoot::Root,
        DALHAL_REACTIVE_EVENT_TABLE(ONE_WIRE_TEMP_BUS),
        &OneWireTempBus::FunctionTable
    };
    
    /* override */
    const Registry::DefineBase* OneWireTempBusAtRoot::GetRegistryDefine() {
        return &RegistryDefine;
    }


    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::ReadString> OneWireTempBus::readStringFunctions[] = {
        {"getAllNewDevices", &OneWireTempBus::readString_getAllNewDevices_Function, "get all new devices present on the bus according to the current cfg"},
        {"getAllNewDevicesWithTemp", &OneWireTempBus::readString_getAllNewDevicesWithTemp_Function, "get all new devices with current temperature present on the bus according to the current cfg"},
        {"getAllDevices", &OneWireTempBus::readString_getAllDevices_Function, "get all devices (even new) present on the bus"},
        {"getAllTemperatures", &OneWireTempBus::readString_getAllTemperatures_Function, "get all devices (even new) present on the bus with temperature printed"}
    };

    constexpr DeviceFunctionTable OneWireTempBus::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,

        EmptyFunctionTable<FunctionTypes::ReadToHALValue>,
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,

        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,

        {readStringFunctions, sizeof(readStringFunctions) / sizeof(readStringFunctions[0])},
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase OneWireTempBus::RegistryDefine = {
        OneWireTempBus::Create,
        &JsonSchema::OneWireTempBus::Root,
        DALHAL_REACTIVE_EVENT_TABLE(ONE_WIRE_TEMP_BUS),
        &OneWireTempBus::FunctionTable
    };

    /* override */
    const Registry::DefineBase* OneWireTempBus::GetRegistryDefine() {
        return &OneWireTempBus::RegistryDefine;
    }

    Device* OneWireTempBus::Create(DeviceCreateContext& context) {
        return new OneWireTempBus(context);
    }

    OneWireTempBus::OneWireTempBus(DeviceCreateContext& context) : OneWireTempBus_DeviceBase(context.deviceType) {
        JsonSchema::OneWireTempBus::Extractors::Apply(context, this);
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
#if HAS_REACTIVE_CYCLE_COMPLETE(ONE_WIRE_TEMP_BUS)
        triggerCycleComplete();
#endif
    }

    DeviceFindResult OneWireTempBus::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(devices, deviceCount, path, this, outDevice);
    }

    HALOperationResult OneWireTempBus::readString_getAllNewDevices_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        static_cast<OneWireTempBus*>(device)->getAllDevices(false, true, sbs);
        return HALOperationResult::Success;
    }
    HALOperationResult OneWireTempBus::readString_getAllNewDevicesWithTemp_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        static_cast<OneWireTempBus*>(device)->getAllDevices(true, true, sbs);
        return HALOperationResult::Success;
    }
    HALOperationResult OneWireTempBus::readString_getAllDevices_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        static_cast<OneWireTempBus*>(device)->getAllDevices(false, false, sbs);
        return HALOperationResult::Success;
    }
    HALOperationResult OneWireTempBus::readString_getAllTemperatures_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs) {
        static_cast<OneWireTempBus*>(device)->getAllDevices(true, false, sbs);
        return HALOperationResult::Success;
    }

    bool OneWireTempBus::haveDeviceWithRomID(OneWireAddress addr) {
        if (deviceCount == 0 || devices == nullptr) return false;
        for (int i=0;i<deviceCount;i++) {
            
            OneWireTempDevice* device = static_cast<OneWireTempDevice*>(devices[i]); // cast for special access
            if (device->romid.id == addr.id) return true;
        }
        return false;
    }

    void OneWireTempBus::getAllDevices(bool printTemp, bool onlyNewDevices, StringBuilderStreamer& sbs) {
        uint8_t done = 0;
        OneWireAddress addr;
        bool first = true;
        sbs.write_json_object_begin();
        sbs.write_jsonNumber(F("pin"), pin);
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("items"));
        sbs.write_json_array_begin();
        
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
                    sbs.write_json_value_separator();
                } else {
                    first = false;
                }
       
                sbs.write_json_object_begin();

                sbs.write_jsonMemberStart(F("romId"));
                sbs.write_char('"');
                sbs.write_asHex(addr.bytes, 8, ':');
                sbs.write_char('"');
                
                if (printTemp) {
                    sbs.write_json_value_separator(); sbs.write_jsonNumber(F("tempC"), dTemp->getTempC(addr.bytes));
                    sbs.write_json_value_separator(); sbs.write_jsonNumber(F("tempF"), dTemp->getTempF(addr.bytes));
                }
                sbs.write_json_object_end();
            }
        }
        sbs.write_json_array_end();
        sbs.write_json_object_end();
    }

    void OneWireTempBus::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pin"), pin);
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

    OneWireTempBusAtRoot::OneWireTempBusAtRoot(DeviceCreateContext& context) 
    : OneWireTempBus(context),
        autoRefresh(
              [this]() { requestTemperatures(); },
              [this]() { readAll(); }
        )
    {
        JsonSchema::OneWireTempBusAtRoot::Extractors::Apply(context, this);
        uint32_t refreshMs = JsonSchema::CommonTime::refreshTimeGroupFieldsRequired.ExtractFrom(*(context.jsonObjItem)).toUInt();
        Serial1.printf("\r\nrefreshtimeMs:%" PRIu32 "\r\n", refreshMs);
        autoRefresh.SetRefreshTimeMs(refreshMs);
    }

    OneWireTempBusAtRoot::~OneWireTempBusAtRoot() {
        
    }

    void OneWireTempBusAtRoot::loop() {
        autoRefresh.loop();
    }

    void OneWireTempBusAtRoot::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        OneWireTempBus::PrintTo(sbs);
        sbs.write_json_value_separator();
        autoRefresh.PrintTo(sbs);

    }

    Device* OneWireTempBusAtRoot::Create(DeviceCreateContext& context) {
        return new OneWireTempBusAtRoot(context);
    }
}