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

#include "DALHAL_HA_Button.h"
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_DeviceDiscovery.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_PubSubClient_JsonWriter.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_CountingPubSubClient.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_Constants.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_CreateFunctionContext.h>

#include "DALHAL_HA_Button_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase HA_Button::RegistryDefine = {
        Create,
        &JsonSchema::HA_Button::Root,
    };
    //volatile const void* keep_HA_Button = &DALHAL::HA_Button::RegistryDefine;

    void HA_Button::SendDeviceDiscovery(PubSubClient& mqtt, TopicBasePath& topicBasePath) {
        const char* cmdTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::Command);
        PSC_JsonWriter::printf_str(mqtt, JSON(,"command_topic":"%s"), cmdTopicStr);
    }

    Device* HA_Button::Create(DeviceCreateContext& context) {
        return new HA_Button(static_cast<HA_CreateFunctionContext&>(context));
    }
    
    HA_Button::HA_Button(HA_CreateFunctionContext& context) : Device(context.deviceType), mqttClient(context.mqttClient)  {
        JsonSchema::HA_Button::Extractors::Apply(context, this);
    }

    HA_Button::~HA_Button() {
        delete cda;
    }

    String HA_Button::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\"";
       
        return ret;
    }

    HALOperationResult HA_Button::exec(const ZeroCopyString& cmd) {
        return cda->Exec();
    }

    HALOperationResult HA_Button::exec() {
        return cda->Exec();
    }

}