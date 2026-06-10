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

#include "DALHAL_HA_Sensor.h"

#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_DeviceDiscovery.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_PubSubClient_JsonWriter.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_CountingPubSubClient.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_Constants.h>

#include <DALHAL/Core/Manager/DALHAL_DeviceManager.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_CreateFunctionContext.h>

#include "DALHAL_HA_Sensor_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase HA_Sensor::RegistryDefine = {
        Create,
        &JsonSchema::HA_Sensor::Root,
    };
    
    /* override */
    const Registry::DefineBase* HA_Sensor::GetRegistryDefine() {
        return &RegistryDefine;
    }

    void HA_Sensor::SendDeviceDiscovery(PubSubClient& mqtt, const HA_DD_Context& ctx) {
        HA_DeviceDiscovery::SendAvailabilityTopicCfg(mqtt, ctx); // adds , before
        HA_DeviceDiscovery::SendStateTopicCfg(mqtt, ctx);  // adds , before
    }

    Device* HA_Sensor::Create(DeviceCreateContext& context) {
        return new HA_Sensor(static_cast<HA_CreateFunctionContext&>(context));
    }
    
    HA_Sensor::HA_Sensor(HA_CreateFunctionContext& context) : HA_DeviceEntity(context) {
        JsonSchema::HA_Sensor::Extractors::Apply(context, this);
        wasOnline = false;
        lastMs = millis()-refreshMs; // force a direct update after start
    }
    
    HA_Sensor::~HA_Sensor() {
        delete eventSource;
        delete cdr;
    }

    void HA_Sensor::PrintTo(StringBuilderStreamer& sbs) {
        //String ret = Device::ToString();
        Device::PrintTo(sbs);
        //return ret;
    }

    bool HA_Sensor::IsTimedRefresh_NOT_Due() {
        // this check should not be needed
        if (refreshMs == 0) return true;
        unsigned long now = millis();
        if (now - lastMs < refreshMs) {
            return true;
        }
        lastMs = now;
        return false;
    }

    void HA_Sensor::loop() {

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

        //GlobalLogger.Info(F("Sensor::loop() exec"));
        //Serial1.print(hass_uid.c_str()); Serial1.println(F("Sensor::loop() exec"));

        HALValue val;
        HALOperationResult res = cdr->ReadSimple(val);

        if (res == HALOperationResult::Success)
        {
            //Serial1.print(hass_uid.c_str()); Serial1.println(F("Sensor::loop() isOnline"));
            if (!wasOnline)
            {
                Serial1.print(hass_uid.c_str()); Serial1.println(F("Sensor::loop() !wasOnline @ operation success"));
                HA_DeviceDiscovery::SetAvailability(mqttClient, hass_uid.c_str(), wasOnline, true);
            }
            bool success = HA_DeviceDiscovery::SendState(mqttClient, hass_uid.c_str(), val);
            if (success == false) {
                GlobalLogger.Error(F("could not send sensor state update to HASS"));
            }
        }
        else
        {
            Serial1.print(hass_uid.c_str()); Serial1.println(F("Sensor::loop() operation fail"));
            if (wasOnline)
            {
                HA_DeviceDiscovery::SetAvailability(mqttClient, hass_uid.c_str(), wasOnline, false);
            }
        }
    }
    void HA_Sensor::begin() {

    }

    HALOperationResult HA_Sensor::read(HALValue& val) {
        if (cdr != nullptr) {
            return cdr->ReadSimple(val);
        }
        return HALOperationResult::UnsupportedOperation;

    }
    HALOperationResult HA_Sensor::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        if (!wasOnline) {
            HA_DeviceDiscovery::SetAvailability(mqttClient, hass_uid.c_str(), wasOnline, true);
        }
        bool success = HA_DeviceDiscovery::SendState(mqttClient, hass_uid.c_str(), val);
        
        return success ? HALOperationResult::Success: HALOperationResult::ExecutionFailed;
    };
    
}