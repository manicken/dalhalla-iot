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

#include "../../Core/Device/DALHAL_Device.h"
#include "../DeviceRegistry/DALHAL_DeviceTypesRegistry.h"

namespace DALHAL {

    class ScriptArray : public Device {
    private:
        HALValue* values;
        int valueCount = 0;
        bool readOnly = false;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            Create,
            VerifyJSON
        };

        /** called when all hal devices has been loaded */
        void begin() override;

        HALOperationResult read(const HALReadStringRequestValue& val) override;

        HALOperationResult read(const HALValue& bracketSubscriptVal, HALValue& val) override;
        HALOperationResult write(const HALValue& bracketSubscriptVal, const HALValue& val) override;

        //static HALOperationResult BracketRead_Func(Device* device, const HALValue& bracketSubscriptVal, HALValue& val);
        //static HALOperationResult BracketWrite_Func(Device* device, const HALValue& bracketSubscriptVal, const HALValue& val);

        //BracketOpRead_FuncType GetBracketOpRead_Function(ZeroCopyString& zcFuncName) override;
        //BracketOpWrite_FuncType GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) override;

        
        ScriptArray(const JsonVariant &jsonObj, const char* type);

        String ToString() override;
    };
}