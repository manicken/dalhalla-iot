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

#include "DALHAL_HA_Switch.h"

#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_DeviceDiscovery.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_CountingPubSubClient.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_Constants.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_CreateFunctionContext.h>

#include "DALHAL_HA_Switch_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase HA_Switch::RegistryDefine = {
        Create,
        &JsonSchema::HA_Switch::Root,
    };
    //volatile const void* keep_HA_Switch = &DALHAL::HA_Switch::RegistryDefine;

    const char* HA_Switch::PAYLOAD_OFF = "OFF";
    const char* HA_Switch::PAYLOAD_ON = "ON";

    void HA_Switch::SendDeviceDiscovery(PubSubClient& mqtt, const JsonVariant& jsonObj, TopicBasePath& topicBasePath) {
        const char* cmdTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::Command);
        PSC_JsonWriter::printf_str(mqtt, JSON(,"command_topic":"%s"), cmdTopicStr);
        PSC_JsonWriter::printf_str(mqtt, JSON(,"payload_on":"%s"), HA_Switch::PAYLOAD_ON);
        PSC_JsonWriter::printf_str(mqtt, JSON(,"payload_off":"%s"), HA_Switch::PAYLOAD_OFF);
        const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
        PSC_JsonWriter::printf_str(mqtt, JSON(,"state_topic":"%s"), stateTopicStr);
    }

    Device* HA_Switch::Create(DeviceCreateContext& context) {
        return new HA_Switch(static_cast<HA_CreateFunctionContext&>(context));
    }
    
    HA_Switch::HA_Switch(HA_CreateFunctionContext& context) : Device(context.deviceType), mqttClient(context.mqttClient) {
        JsonSchema::HA_Switch::Extractors::Apply(context, this);
    }
    
    HA_Switch::~HA_Switch() {
        delete cda;
    }

    String HA_Switch::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\"";
       
        return ret;
    }

    HALOperationResult HA_Switch::read(HALValue& val) {
        if (cda != nullptr) {
            return cda->ReadSimple(val);
        }
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult HA_Switch::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;


        const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
        if (val.toUInt() == 0) {
            mqttClient.publish(stateTopicStr, HA_Switch::PAYLOAD_OFF);
        } else {
            mqttClient.publish(stateTopicStr, HA_Switch::PAYLOAD_ON);
        }
        if (cda != nullptr) {
            return cda->WriteSimple(val);
        }
        return HALOperationResult::UnsupportedOperation;
    };

    HALOperationResult HA_Switch::exec(const ZeroCopyString& cmd) {
        if (momentary == false) {
            HALValue valState;
            HALOperationResult res = HALOperationResult::NotSet;
            const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
            if (cmd == HA_Switch::PAYLOAD_ON) {
                valState.set((uint32_t)1);
            } else if (cmd == HA_Switch::PAYLOAD_OFF) {
                valState.set((uint32_t)0);
            } else {
                return HALOperationResult::UnsupportedCommand; // or some error code
            }
            res = cda->WriteSimple(valState);
            if (res == HALOperationResult::Success) {
                Serial.println("switch exec OK");
                if (valState.toUInt() == 0)
                    mqttClient.publish(stateTopicStr, HA_Switch::PAYLOAD_OFF);
                else
                    mqttClient.publish(stateTopicStr, HA_Switch::PAYLOAD_ON);
            } else {
                Serial.println("switch exec fail");
            }
            return res;
        } else {
            const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
            mqttClient.publish(stateTopicStr, HA_Switch::PAYLOAD_OFF);
            return cda->Exec();
        }
    }

}