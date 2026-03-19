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

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
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
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(DeviceCreateContext& context);

    private:
        uint8_t pin = 0; // if pin would be used

    public:
        _Template_(DeviceCreateContext& context);

        /** called when all hal devices has been loaded */
        void begin() override;
        /** called regulary from the main loop */
        void loop() override;
        /** used to find sub/leaf devices @ "group devices" */
        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;

        HALOperationResult read(HALValue& val) override;
        HALOperationResult write(const HALValue& val) override;
        HALOperationResult read(const HALValue& bracketSubscriptVal, HALValue& val) override;
        HALOperationResult write(const HALValue& bracketSubscriptVal, const HALValue& val) override;
        HALOperationResult read(const HALReadStringRequestValue& val) override;
        HALOperationResult write(const HALWriteStringRequestValue& val) override;
        HALOperationResult read(const HALReadValueByCmd& val) override;
        HALOperationResult write(const HALWriteValueByCmd& val) override;

        HALValue* GetValueDirectAccessPtr() override;
        ReadToHALValue_FuncType GetReadToHALValue_Function(ZeroCopyString& zcFuncName) override;
        WriteHALValue_FuncType GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) override;
        Exec_FuncType GetExec_Function(ZeroCopyString& zcFuncName) override;
        BracketOpRead_FuncType GetBracketOpRead_Function(ZeroCopyString& zcFuncName) override;
        BracketOpWrite_FuncType GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) override;
        
        /** Executes a device action that requires no parameters. */
        HALOperationResult exec() override ;
        /** Executes a device action with a provided command string. */
        HALOperationResult exec(const ZeroCopyString& cmd) override ;

        String ToString() override;
    };
}