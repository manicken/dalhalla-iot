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

#include <string>
#include <ArduinoJson.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(SCRIPT_ARRAY)
#include "DALHAL_ScriptArray_Reactive.h"
using ScriptArray_DeviceBase = DALHAL::ScriptArray_Reactive;
#else
using ScriptArray_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    namespace JsonSchema { namespace ScriptArray { struct Extractors; } } // forward declaration

    class ScriptArray : public ScriptArray_DeviceBase {
        friend struct JsonSchema::ScriptArray::Extractors; // allow access to private memebers of this class from the schema extractor

    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static Device* Create(DeviceCreateContext& context);

    private:
        static HALOperationResult BracketRead_Func(Device* device, const HALValue& bracketSubscriptVal, HALValue& val);
        static HALOperationResult BracketWrite_Func(Device* device, const HALValue& bracketSubscriptVal, const HALValue& val);

        HALValue* values = nullptr;
        int valueCount = 0;
        bool readOnly = false;

    public:
        ScriptArray(DeviceCreateContext& context);
        ~ScriptArray() override;

        void begin() override;

        HALOperationResult read(const HALReadStringRequestValue& val) override;

        HALOperationResult read(const HALValue& bracketSubscriptVal, HALValue& val) override;
        HALOperationResult write(const HALValue& bracketSubscriptVal, const HALValue& val) override;

        BracketOpRead_FuncType GetBracketOpRead_Function(ZeroCopyString& zcFuncName) override;
        BracketOpWrite_FuncType GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) override;

        String ToString() override;
    };
}