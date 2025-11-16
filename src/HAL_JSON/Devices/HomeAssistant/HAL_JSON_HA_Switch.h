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
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"

#include <WiFiClient.h>
#include <PubSubClient.h>

namespace HAL_JSON {

    class Switch : public Device {
    private:
        PubSubClient& mqttClient;
    public:
        static void SendDeviceDiscovery(PubSubClient& mqttClient, const JsonVariant &jsonObj, const JsonVariant &jsonObjGlobal);

        HALOperationResult read(HALValue& val) override;
        HALOperationResult write(const HALValue& val) override;
        HALOperationResult read(const HALValue& bracketSubscriptVal, HALValue& val) override;
        HALOperationResult write(const HALValue& bracketSubscriptVal, const HALValue& val) override;
        HALOperationResult read(const HALReadStringRequestValue& val) override;
        HALOperationResult write(const HALWriteStringRequestValue& val) override;
        HALOperationResult read(const HALReadValueByCmd& val) override;
        HALOperationResult write(const HALWriteValueByCmd& val) override;
        ReadToHALValue_FuncType GetReadToHALValue_Function(ZeroCopyString& zcFuncName) override;
        WriteHALValue_FuncType GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) override;
        Exec_FuncType GetExec_Function(ZeroCopyString& zcFuncName) override;

        BracketOpRead_FuncType GetBracketOpRead_Function(ZeroCopyString& zcFuncName) override;
        BracketOpWrite_FuncType GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) override;

        HALValue* GetValueDirectAccessPtr() override;
        /** called regulary from the main loop */
        void loop() override;
        /** called when all hal devices has been loaded */
        void begin() override;
        /** used to find sub/leaf devices @ "group devices" */
        Device* findDevice(UIDPath& path) override;

        /** Executes a device action that requires no parameters. */
        HALOperationResult exec() override ;
        /** Executes a device action with a provided command string. */
        HALOperationResult exec(ZeroCopyString& cmd) override ;

        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient);
        Switch(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient);
        ~Switch();


        String ToString() override;
    };
}