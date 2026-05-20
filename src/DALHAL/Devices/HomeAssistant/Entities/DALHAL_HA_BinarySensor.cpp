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

#include "DALHAL_HA_BinarySensor.h"

#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_DeviceDiscovery.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_PubSubClient_JsonWriter.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_CountingPubSubClient.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_Constants.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_CreateFunctionContext.h>

#include "DALHAL_HA_BinarySensor_JSON_Schema.h"



#include <System/DeviceUID.h>

#define DALHAL_HA_BINARY_SENSOR_PAYLOAD_OFF "OFF"
#define DALHAL_HA_BINARY_SENSOR_PAYLOAD_ON "ON"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase HA_BinarySensor::RegistryDefine = {
        Create,
        &JsonSchema::HA_BinarySensor::Root,
    };
    //volatile const void* keep_HA_BinarySensor = &DALHAL::HA_BinarySensor::RegistryDefine;

    void HA_BinarySensor::SendDeviceDiscovery(PubSubClient& mqtt, const HA_DD_Context& ctx) {
        HA_DeviceDiscovery::SendAvailabilityTopicCfg(mqtt, ctx); // adds , before
        HA_DeviceDiscovery::SendStateTopicCfg(mqtt, ctx); // adds , before
        mqtt.write(',');
        mqtt.write('\n');
        PSC_JsonWriter::kv(mqtt, "platform", "binary_sensor");
    }

    Device* HA_BinarySensor::Create(DeviceCreateContext& context) {
        return new HA_BinarySensor(static_cast<HA_CreateFunctionContext&>(context));
    }
    
    HA_BinarySensor::HA_BinarySensor(HA_CreateFunctionContext& context) : HA_DeviceEntity(context) {
        JsonSchema::HA_BinarySensor::Extractors::Apply(context, this);
        wasOnline = false;
        lastMs = millis()-refreshMs; // force a direct update after start
    }
    
    HA_BinarySensor::~HA_BinarySensor() {
        delete cdr;
        delete eventSource;
    }

    String HA_BinarySensor::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\"";
       
        return ret;
    }

    bool HA_BinarySensor::IsTimedRefresh_NOT_Due() {
        // this check should not be needed
        if (refreshMs == 0) return true;
        unsigned long now = millis();
        if (now - lastMs < refreshMs) {
            return true;
        }
        lastMs = now;
        return false;
    }

    void HA_BinarySensor::loop() {

        switch (consumerMode)
        {
            case Consumer::Mode::Manual:
                return;
            case Consumer::Mode::TimedRefresh:
                if (IsTimedRefresh_NOT_Due()) {
                    return;
                }
                break;
            case Consumer::Mode::Event:
                // check should not be needed in final version as then every mode should be explicit
                if (eventSource == nullptr) { return; }
                if (eventSource->CheckForEvent() == false) { return; }
                break;
            default: // should never happend
                return;
        }
        // check should not be needed in final version as then every mode should be explicit
        if (cdr == nullptr) { return; }

        //GlobalLogger.Info(F("BinarySensor::loop() exec"));

        HALValue val;
        HALOperationResult res = cdr->ReadSimple(val);
        bool isOnline = (res == HALOperationResult::Success);

        if (isOnline)
        {
            if (!wasOnline)
            {
                HA_DeviceDiscovery::SetAvailability(mqttClient, hass_uid.c_str(), wasOnline, true);
            }
            bool state = val.toBool();
            uint32_t state_cStr_Length = (state ? sizeof(DALHAL_HA_BINARY_SENSOR_PAYLOAD_ON) : sizeof(DALHAL_HA_BINARY_SENSOR_PAYLOAD_OFF)) - 1;
            const char* state_cStr = state ? DALHAL_HA_BINARY_SENSOR_PAYLOAD_ON : DALHAL_HA_BINARY_SENSOR_PAYLOAD_OFF;
            bool success = HA_DeviceDiscovery::SendState(mqttClient, hass_uid.c_str(), state_cStr, state_cStr_Length);
            if (success == false) {
                GlobalLogger.Error(F("could not send binary sensor state update to HASS"));
            }
        }
        else
        {
            if (wasOnline)
            {
                HA_DeviceDiscovery::SetAvailability(mqttClient, hass_uid.c_str(), wasOnline, false);
            }
        }
    }

    void HA_BinarySensor::begin() {

    }

    HALOperationResult HA_BinarySensor::read(HALValue& val) {
        if (cdr != nullptr) {
            return cdr->ReadSimple(val);
        }
        return HALOperationResult::UnsupportedOperation;

    }
    HALOperationResult HA_BinarySensor::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        if (!wasOnline) {
            HA_DeviceDiscovery::SetAvailability(mqttClient, hass_uid.c_str(), wasOnline, true);
        }
        if (!wasOnline) {
            return HALOperationResult::ExecutionFailed;
        }
        bool state = val.toBool();
        uint32_t state_cStr_Length = (state ? sizeof(DALHAL_HA_BINARY_SENSOR_PAYLOAD_ON) : sizeof(DALHAL_HA_BINARY_SENSOR_PAYLOAD_OFF)) - 1;
        const char* state_cStr = state ? DALHAL_HA_BINARY_SENSOR_PAYLOAD_ON : DALHAL_HA_BINARY_SENSOR_PAYLOAD_OFF;
        bool success = HA_DeviceDiscovery::SendState(mqttClient, hass_uid.c_str(), state_cStr, state_cStr_Length);

        return success ? HALOperationResult::Success: HALOperationResult::ExecutionFailed;
    };
    
}