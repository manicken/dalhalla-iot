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
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"

namespace HAL_JSON {

    class Template : public Device {
    private:
        uint8_t pin = 0; // if pin would be used
    public:

        HALOperationResult read(HALValue& val);
        HALOperationResult write(const HALValue& val);
        HALOperationResult read(const HALReadStringRequestValue& val);
        HALOperationResult write(const HALWriteStringRequestValue& val);
        HALOperationResult read(const HALReadValueByCmd& val);
        HALOperationResult write(const HALWriteValueByCmd& val);
        ReadToHALValue_FuncType GetReadToHALValue_Function(ZeroCopyString& zcFuncName);
        ReadToHALValue_FuncType GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName);
        HALValue* GetValueDirectAccessPtr();
        /** called regulary from the main loop */
        void loop();
        /** called when all hal devices has been loaded */
        void begin();
        /** used to find sub/leaf devices @ "group devices" */
        Device* findDevice(UIDPath& path);

        /** Executes a device action that requires no parameters. */
        HALOperationResult exec();
        /** Executes a device action with a provided command string. */
        HALOperationResult exec(ZeroCopyString& cmd);

        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        Template(const JsonVariant &jsonObj, const char* type);



        String ToString() override;
    };
}