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
        Create,
        &JsonSchema::OneWireTempBusAtRoot::Root,
        DALHAL_REACTIVE_EVENT_TABLE(ONE_WIRE_TEMP_BUS)
    };
    //volatile const void* keep_OneWireTempBusAtRoot = &DALHAL::OneWireTempBusAtRoot::RegistryDefine;

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

    HALOperationResult OneWireTempBus::read(const HALReadStringRequestValue& val) {
        if (val.cmd.EqualsIC(F("getAllNewDevices"))) { // (as json) return a list of all new devices found for all busses (this will compare against the current ones and only print new ones)
            getAllDevices(false, true, val.sbs);
        }
        else if (val.cmd.EqualsIC(F("getAllNewDevicesWithTemp"))) { // (as json) return a list of all new devices found for all busses (this will compare against the current ones and only print new ones)
            getAllDevices(true, true, val.sbs);
        }
        else if (val.cmd.EqualsIC(F("getAllDevices"))) { // (as json) return a complete list of all devices found for all busses
            getAllDevices(false, false, val.sbs);
        }
        else if (val.cmd.EqualsIC(F("getAllTemperatures"))) { // (as json) return a complete list of all temperatures each with it's uid as the keyname and the temp as the value
            getAllDevices(true, false, val.sbs);
        } else {
            std::string stdStrCmd = val.cmd.ToString();
            GlobalLogger.Warn(F("OneWireTempBus::read - cmd not found: "), stdStrCmd.c_str()); // this can then be read by getting the last entry from logger
            return HALOperationResult::UnsupportedCommand;
        }
#if HAS_REACTIVE_READ(ONE_WIRE_TEMP_BUS)
        triggerRead();
#endif
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
        sbs.write('{');
        sbs.write_jsonNumber(F("pin"), pin);
        sbs.write(',');
        sbs.write_jsonKey(F("items"));
        sbs.write('[');
        
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
                    sbs.write(',');
                } else {
                    first = false;
                }
       
                sbs.write('{');

                sbs.write_jsonKey(F("romId"));
                sbs.write('"');
                for(int i = 0; i < 8; i++) 
                {
                    if (i>0) { sbs.write(':'); }
                    sbs.write_asHex(addr.bytes[i]);
                }
                sbs.write('"');
                
                if (printTemp) {
                    sbs.write(','); sbs.write_jsonNumber(F("tempC"), dTemp->getTempC(addr.bytes));
                    sbs.write(','); sbs.write_jsonNumber(F("tempF"), dTemp->getTempF(addr.bytes));
                }
                sbs.write('}');
            }
        }
        sbs.write(']');
        sbs.write('}');
    }

    String OneWireTempBus::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += '"';
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += F(",\"devices\":[");
        for (int i=0;i<deviceCount;i++) {
            ret += '{';
            ret += devices[i]->ToString();
            ret += '}'; 
            if (i<deviceCount-1) ret += ',';
        }
        ret += ']';
        return ret;
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

    String OneWireTempBusAtRoot::ToString() {
        String ret;
        //ret += DeviceConstStrings::uid;
        //ret += decodeUID(uid).c_str();
        //ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\",";
        ret += OneWireTempBus::ToString();
        ret += autoRefresh.ToString();
        
        return ret;
    }

    Device* OneWireTempBusAtRoot::Create(DeviceCreateContext& context) {
        return new OneWireTempBusAtRoot(context);
    }
}