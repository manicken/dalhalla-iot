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

#include "../../../Core/Device/DALHAL_Device.h"
#include "../../DeviceRegistry/DALHAL_DeviceTypesRegistry.h"

#include "../../../Config/DALHAL_ReactiveConfig.h"
#if USING_REACTIVE(TEMPLATE)
#include "DALHAL_ScriptVariableReactive.h"
#include "../../../Core/Types/DALHAL_ValueReactive.h"
using ScriptVariableDeviceBase = DALHAL::ScriptVariable_Reactive;
#else
using ScriptVariableDeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    class ScriptVariable : public ScriptVariableDeviceBase {
    private:
#if USING_REACTIVE(TEMPLATE)
        ReactiveHALValue value;
        static void ValueSetCallback(void* ctx);
#else
        HALValue value;
#endif

    public:
        
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            Create,
            VerifyJSON
        };
        ScriptVariable(const JsonVariant &jsonObj, const char* type);
        HALOperationResult read(HALValue& val) override;
        HALOperationResult write(const HALValue& val) override;
        HALValue* GetValueDirectAccessPtr() override;

        String ToString() override;
    };
}