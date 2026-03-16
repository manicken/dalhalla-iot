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

#pragma once


#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(SCRIPT_WRITEVAR)
#include "DALHAL_ScriptVariableWriteOnlyTest_Reactive.h"
using ScriptVariableWriteOnlyTest_DeviceBase = DALHAL::ScriptVariableWriteOnlyTest_Reactive;
#else
using ScriptVariableWriteOnlyTest_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    class ScriptVariableWriteOnlyTest : public ScriptVariableWriteOnlyTest_DeviceBase {
    private:
        HALValue value;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(DeviceCreateContext& context);
        static const Registry::DefineRoot RegistryDefine;/* = {
            Registry::UseRootUID::Mandatory,
            Create,
            VerifyJSON,
            DALHAL_REACTIVE_EVENT_TABLE(SCRIPT_WRITEVAR)
        };*/
        ScriptVariableWriteOnlyTest(DeviceCreateContext& context);
        HALOperationResult write(const HALValue& val) override;

        String ToString() override;
    };
}