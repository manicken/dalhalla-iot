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


#include <Arduino.h>

#include <ArduinoJson.h>
#include <stdlib.h>

#include "DALHAL_Device.h"
#include "../Types/DALHAL_Value.h"
#include "../Types/DALHAL_ZeroCopyString.h"

namespace DALHAL {

    class CachedDeviceRead {
    public:
        using ReadHandler = HALOperationResult(*)(void* context, HALValue& val);

    private:
        void* context;                  // context for handler (device, HALValue*, or subread)
        ReadHandler handler;            // function to call for reading
        void (*deleter)(void* context); // optional deleter for owned context
        bool SetBracketAccess(const char* bracketPos, ZeroCopyString zcStrUidPathAndFuncName);
    public:
        // Construct from UID/path string
        //CachedDeviceRead(const char* uidPathAndFuncName);
        CachedDeviceRead();
        //CachedDeviceRead(ZeroCopyString zcStrUidPathAndFuncName); prevent usage that could lead to uncertain states
        /** emit errors inside so no reporting is needed here unless one need to specific */
        bool Set(ZeroCopyString zcStrUidPathAndFuncName);

        // Execute the read
        HALOperationResult ReadSimple(HALValue& val);

        ~CachedDeviceRead();

    private:
        // Handlers
        static HALOperationResult Handler_Invalid(void* ctx, HALValue& val);
        static HALOperationResult Handler_Direct(void* context, HALValue& val);
        static HALOperationResult Handler_Device(void* context, HALValue& val);
        static HALOperationResult Handler_Func(void* context, HALValue& val);
        static HALOperationResult Handler_Bracket(void* context, HALValue& val);
    };

    
}