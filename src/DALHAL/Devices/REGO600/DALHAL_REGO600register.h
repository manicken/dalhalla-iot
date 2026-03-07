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

#include <ArduinoJson.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(REGO600_REGISTRY_ITEM)
#include "DALHAL_REGO600register_Reactive.h"
using REGO600register_DeviceBase = DALHAL::REGO600register_Reactive;
#else
using REGO600register_DeviceBase = DALHAL::Device;
#endif

#if HAS_REACTIVE_VALUE_CHANGE(SCRIPT_VARIABLE)
#include <DALHAL/Core/Types/DALHAL_ValueReactive.h>
using ScriptVariable_ValueBase = DALHAL::ReactiveHALValue;
#else
using ScriptVariable_ValueBase = DALHAL::HALValue;
#endif

namespace DALHAL {
    
    class REGO600register : public REGO600register_DeviceBase {
        
    public:
        
        ScriptVariable_ValueBase value;   // need to be public for the moment
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        REGO600register(const JsonVariant &jsonObj, const char* type);

        HALOperationResult read(HALValue& val) override;
        String ToString() override;
    };

}