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

#include "DALHAL_REST_Cmd.h"
#if defined(ESP32)
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#endif
#include <WiFiClient.h>

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include "DALHAL_REST_Cmd_JSON_Schema.h"
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h> // for DALHAL_COMMON_CFG_NAME_UID

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase REST_Cmd::RegistryDefine = {
        Create,
        &JsonSchema::REST_Cmd::Root,
        nullptr // no events available on obsolete device
    };
    //volatile const void* keep_REST_Cmd = &DALHAL::REST_Cmd::RegistryDefine;

    Device* REST_Cmd::Create(DeviceCreateContext& context) {
        return new REST_Cmd(context);
    }

    REST_Cmd::REST_Cmd(DeviceCreateContext& context) : DALHAL::Device(context.deviceType)
    {
        JsonSchema::REST_Cmd::Extractors::Apply(context, this);
    }

    DALHAL::HALOperationResult REST_Cmd::exec() {
        int getCode = 0;
        HTTPClient client;
        WiFiClient wifiClient;
        if (client.begin(wifiClient, remoteUrl)) {
            getCode = client.GET();
        }

        if (getCode != 200) {
#if defined(_WIN32) || defined(__linux__)
            printf("\n[REST_Cmd] Failed to exec (%d): %s\n", getCode, remoteUrl.c_str());
#endif
            return HALOperationResult::ExecutionFailed;
        }
        String resp = client.getString();
        ZeroCopyString zcStrData(resp.c_str());
        if (zcStrData.FindString("\"error\"") != nullptr) {
            return HALOperationResult::ExecutionFailed;
        }
        return DALHAL::HALOperationResult::Success;
    }
}