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

#include <Arduino.h> // Needed for String class

#include <ArduinoJson.h>

#include <DALHAL/Core/Types/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Core/Types/DALHAL_DeviceFunctionTable.h>

#include <DALHAL/Core/Reactive/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(REGO600_REGISTRY_ITEM)
#include "DALHAL_REGO600_Register_Reactive.h"
using REGO600register_DeviceBase = DALHAL::REGO600_Register_Reactive;
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

    namespace JsonSchema { namespace REGO600_Register { struct Extractors; } } // forward declaration
    
    class REGO600_Register : public REGO600register_DeviceBase {
        friend struct JsonSchema::REGO600_Register::Extractors; // allow access to private memebers of this class from the schema extractor

    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;

    private:
        static const DeviceFunctionTable FunctionTable;
        static const FunctionEntry<FunctionTypes::ReadToHALValue> readValueFunctions[];

        static HALOperationResult HALValue_primary_read(Device* device, HALValue &val);

    public: // member data
        ScriptVariable_ValueBase value;   // need to be public for the moment
        
    public: // member functions
        REGO600_Register(DeviceCreateContext& context);
        ~REGO600_Register() override = default;

        const Registry::DefineBase* GetRegistryDefine() override;

        HALValue* GetValueDirectAccessPtr() override;
      
        void PrintTo(StringBuilderStreamer& sbs) override;
        
    };

}