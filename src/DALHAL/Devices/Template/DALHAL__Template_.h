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

#pragma once

#define DALHAL_DEVICE__TEMPLATE_

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>

#include <DALHAL/Core/Types/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Core/Types/DALHAL_DeviceFunctionTable.h>

#include <DALHAL/Core/Reactive/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(_TEMPLATE_)
#include "DALHAL__Template__Reactive.h"
using _Template__DeviceBase = DALHAL::_Template__Reactive;
#else
using _Template__DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {
    class _Template_ : public _Template__DeviceBase {
    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static Device* Create(DeviceCreateContext& context);

    private:
        static const DeviceFunctionTable FunctionTable;
        static const DeviceFunctionTable FunctionTable2;
        static const FunctionEntry<FunctionTypes::Exec> execFunctions[];
        static const FunctionEntry<FunctionTypes::ReadToHALValue> readValueFunctions[];
        static const FunctionEntry<FunctionTypes::WriteHALValue> writeValueFunctions[];
        static const FunctionEntry<FunctionTypes::BracketOpRead> bracketOpReadFunctions[];
        static const FunctionEntry<FunctionTypes::BracketOpWrite> bracketOpWriteFunctions[];
        static const FunctionEntry<FunctionTypes::ReadString> readStringFunctions[];
        static const FunctionEntry<FunctionTypes::WriteString> writeStringFunctions[];

        static HALOperationResult exec_Template_Function(Device* device);
        static HALOperationResult readValue_Template_Function(Device* device, HALValue& val);
        static HALOperationResult writeValue_Template_Function(Device* device, const HALValue& val);
        static HALOperationResult bracketOpRead_Template_Function(Device* device, const HALValue& subscriptValue, HALValue& outValue);
        static HALOperationResult bracketOpWrite_Template_Function(Device* device, const HALValue& subscriptValue, const HALValue& inValue);
        static HALOperationResult readString_Template_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs);
        static HALOperationResult writeString_Template_Function(Device* device, const ZeroCopyString& zcStrParameters, StringBuilderStreamer& sbs);

    private:
        uint8_t pin = 0; // if pin would be used

    public:
        _Template_(DeviceCreateContext& context);
        ~_Template_() override = default;

        const Registry::DefineBase* GetRegistryDefine() override;

        /** called when all hal devices has been loaded */
        void begin() override;
        /** called regulary from the main loop */
        void loop() override;
        /** used to find sub/leaf devices @ "group devices" */
        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;

        HALValue* GetValueDirectAccessPtr() override;
                
        void PrintTo(StringBuilderStreamer& sbs) override;
        
    };
}