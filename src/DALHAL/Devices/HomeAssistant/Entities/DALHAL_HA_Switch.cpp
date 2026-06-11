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
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_PubSubClient_JsonWriter.h>
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
    
    /* override */
    const Registry::DefineBase* HA_Switch::GetRegistryDefine() {
        return &RegistryDefine;
    }

    #define DALHAL_HA_SWITCH_PAYLOAD_OFF "OFF"
    #define DALHAL_HA_SWITCH_PAYLOAD_ON "ON"
    const char* HA_Switch::PAYLOAD_OFF = DALHAL_HA_SWITCH_PAYLOAD_OFF;
    const char* HA_Switch::PAYLOAD_ON = DALHAL_HA_SWITCH_PAYLOAD_ON;

    void HA_Switch::SendDeviceDiscovery(PubSubClient& mqtt, const HA_DD_Context& ctx) {
        HA_DeviceDiscovery::SendCommandTopicCfg(mqtt, ctx); // adds , before

        PSC_JsonWriter::printf_str(mqtt, ",\n" JSON("payload_on":"%s"), HA_Switch::PAYLOAD_ON);
        PSC_JsonWriter::printf_str(mqtt, ",\n" JSON("payload_off":"%s"), HA_Switch::PAYLOAD_OFF);

        HA_DeviceDiscovery::SendStateTopicCfg(mqtt, ctx); // adds , before
    }

    Device* HA_Switch::Create(DeviceCreateContext& context) {
        return new HA_Switch(static_cast<HA_CreateFunctionContext&>(context));
    }
    
    HA_Switch::HA_Switch(HA_CreateFunctionContext& context) : HA_DeviceEntity(context) {
        JsonSchema::HA_Switch::Extractors::Apply(context, this);
    }
    
    HA_Switch::~HA_Switch() {
        delete cda;
    }

    void HA_Switch::PrintTo(StringBuilderStreamer& sbs) {
        //String ret = Device::ToString();
        Device::PrintTo(sbs);
        //return ret;
    }

    HALOperationResult HA_Switch::read(HALValue& val) {
        if (cda != nullptr) {
            return cda->ReadSimple(val);
        }
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult HA_Switch::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (!val.isBoolCompatible()) {
            GlobalLogger.Error(F("HA_Switch::write !val.isBoolCompatible(): "), val.typeToString());
            return HALOperationResult::WriteValueNaN;
        }


        bool state = val.toBool();
        uint32_t state_cStr_Length = (state ? sizeof(DALHAL_HA_SWITCH_PAYLOAD_ON) : sizeof(DALHAL_HA_SWITCH_PAYLOAD_OFF)) - 1;
        const char* state_cStr = state ? DALHAL_HA_SWITCH_PAYLOAD_ON : DALHAL_HA_SWITCH_PAYLOAD_OFF;
        bool success = HA_DeviceDiscovery::SendState(mqttClient, hass_uid.c_str(), state_cStr, state_cStr_Length);

        if (success == false) { 
            return HALOperationResult::ExecutionFailed;
        }
        
        if (cda != nullptr) {
            return cda->WriteSimple(val);
        }
        return HALOperationResult::UnsupportedOperation;
    };

    HALOperationResult HA_Switch::ha_apply(const ZeroCopyString& zcVal) {
        if (momentary == false) {
            HALValue valState;
            HALOperationResult res = HALOperationResult::NotSet;
            //const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
            if (zcVal.Equals(HA_Switch::PAYLOAD_ON)) {
                valState.set(true);
            } else if (zcVal.Equals(HA_Switch::PAYLOAD_OFF)) {
                valState.set(false);
            } else {
                return HALOperationResult::UnsupportedCommand; // or some error code
            }
            res = cda->WriteSimple(valState);
            if (res == HALOperationResult::Success) {
                Serial.println("switch exec OK");

                bool state = valState.toBool();
                uint32_t state_cStr_Length = (state ? sizeof(DALHAL_HA_SWITCH_PAYLOAD_ON) : sizeof(DALHAL_HA_SWITCH_PAYLOAD_OFF)) - 1;
                const char* state_cStr = state ? DALHAL_HA_SWITCH_PAYLOAD_ON : DALHAL_HA_SWITCH_PAYLOAD_OFF;
                bool success = HA_DeviceDiscovery::SendState(mqttClient, hass_uid.c_str(), state_cStr, state_cStr_Length);
                if (success == false) { 
                    return HALOperationResult::ExecutionFailed;
                }
            } else {
                Serial.println("switch exec fail");
            }
            return res;
        } else {

            bool success = HA_DeviceDiscovery::SendState(mqttClient, hass_uid.c_str(), DALHAL_HA_SWITCH_PAYLOAD_OFF, sizeof(DALHAL_HA_SWITCH_PAYLOAD_OFF)-1);
            if (success == false) { 
                return HALOperationResult::ExecutionFailed;
            }
            return cda->Exec();
        }
    }

}