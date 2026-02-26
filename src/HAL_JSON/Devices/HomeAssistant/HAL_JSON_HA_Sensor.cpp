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

#include "HAL_JSON_HA_Sensor.h"
#include "HAL_JSON_HA_DeviceDiscovery.h"
#include "HAL_JSON_HA_CountingPubSubClient.h"
#include "HAL_JSON_HA_Constants.h"

#include "../../Core/HAL_JSON_Manager.h"
#include "../../Support/HAL_JSON_Logger.h"
#include "../../Core/HAL_JSON_JSON_Config_Defines.h"
#include "../../Support/HAL_JSON_ArduinoJSON_ext.h"

namespace HAL_JSON {


    void Sensor::SendDeviceDiscovery(PubSubClient& mqtt, const JsonVariant& jsonObj, TopicBasePath& topicBasePath) {
        mqtt.write(',');
        mqtt.write('\n');
        HA_DeviceDiscovery::SendAvailabilityTopicCfg(mqtt, topicBasePath);
        const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
        mqtt.write(',');
        mqtt.write('\n');
        PSC_JsonWriter::printf_str(mqtt, JSON("state_topic":"%s"), stateTopicStr);
    }
    
    Sensor::Sensor(const JsonVariant &jsonObj, const char* type_cStr, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) : mqttClient(mqttClient), Device(type_cStr) {
        const char* uidStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);
        
        const char* deviceId_cStr = jsonObjRoot["deviceId"];
        topicBasePath.Set(deviceId_cStr, uidStr);

        if (ValidateJsonStringField(jsonObj, "source")) {
            ZeroCopyString zcSrcDeviceUidStr = GetAsConstChar(jsonObj, "source");
            cdr = new CachedDeviceRead();
            if (cdr->Set(zcSrcDeviceUidStr) == false) {
                // emit errors inside so no reporting is needed here unless one need to specific
                delete cdr;
                cdr = nullptr;
            }
        } else {
            cdr = nullptr;
        }
        refreshMs = ParseRefreshTimeMs(jsonObj, HAL_JSON_HA_SENSOR_DEFAULT_REFRESH_MS);

        //const char* cfgTopic_cStr = HA_DeviceDiscovery::GetDiscoveryCfgTopic(deviceId_cStr, type, uidStr);
        HA_DeviceDiscovery::SendDiscovery(mqttClient, deviceId_cStr, type_cStr, uidStr, jsonObj, jsonObjGlobal, topicBasePath, Sensor::SendDeviceDiscovery);
        //delete[] cfgTopic_cStr;

        wasOnline = false;
        lastMs = millis()-refreshMs; // force a direct update after start
    }
    Sensor::~Sensor() {
        delete deviceEvent;
        delete cdr;
    }

    bool Sensor::VerifyJSON(const JsonVariant &jsonObj)
    {
        bool anyError = false;

        if (!ValidateJsonStringField(jsonObj, "uid")) {
            SET_ERR_LOC("HA_SENSOR_VJ"); // can be set here because ValidateJsonStringField uses GlobalLogger on error
            anyError = true;
        }

        if (!ValidateJsonStringField(jsonObj, "name")) {
            SET_ERR_LOC("HA_SENSOR_VJ"); // can be set here because ValidateJsonStringField uses GlobalLogger on error
            anyError = true;
        }

        bool haveRefreshMs = ParseRefreshTimeMs(jsonObj, 0) != 0;
        bool haveValueSource = false;
        bool haveEventSource = false;

        ZeroCopyString zcSrcDeviceUidStr;

        // ---- SOURCE (optional) ----
        if (ValidateJsonStringField(jsonObj, "source")) {
            zcSrcDeviceUidStr = GetAsConstChar(jsonObj, "source");

            CachedDeviceRead cdr;
            if (!cdr.Set(zcSrcDeviceUidStr)) {
                GlobalLogger.Error(F("source device error"), zcSrcDeviceUidStr);
                SET_ERR_LOC("HA_SENSOR_VJ");
                anyError = true;
            } else {
                haveValueSource = true;
            }
        }

        // ---- EVENT SOURCE (optional but not without "value"-source) ----
        if (ValidateJsonStringField(jsonObj, "eventSource")) {

            if (!haveValueSource) {
                // eventSource without source makes no sense
                GlobalLogger.Error(F("eventSource without 'value'-source defined"));
                SET_ERR_LOC("HA_SENSOR_VJ");
                anyError = true;
            } else {
                ZeroCopyString zcEvtDeviceUidStr = GetAsConstChar(jsonObj, "eventSource");

                HALOperationResult res =
                    Manager::ValidateDeviceEvent(zcEvtDeviceUidStr);

                if (res != HALOperationResult::Success) {
                    GlobalLogger.Error(F("eventSource error"));
                    SET_ERR_LOC(HALOperationResultToString(res));
                    anyError = true;
                } else {
                    haveEventSource = true;
                }
            }
        }

        // ---- refreshTime requires source ----
        if (haveRefreshMs && !haveValueSource) {
            GlobalLogger.Error(F("haveRefreshMs without source"));
            SET_ERR_LOC("HA_SENSOR_VJ");
            anyError = true;
        }

        // ---- Scenario B: source only, no refreshTime, no eventSource ----
        if (haveValueSource && !haveRefreshMs && !haveEventSource) {

            HALOperationResult res =
                Manager::ValidateDeviceEvent(zcSrcDeviceUidStr);

            if (res != HALOperationResult::Success) {
                GlobalLogger.Error(F("source without events do not work without refreshTime or eventSource"));
                SET_ERR_LOC(HALOperationResultToString(res));
                anyError = true;
            }
        }

        return !anyError;
    }

    Device* Sensor::Create(const JsonVariant &jsonObj, const char* type, PubSubClient& mqttClient, const JsonVariant& jsonObjGlobal, const JsonVariant& jsonObjRoot) {
        return new Sensor(jsonObj, type, mqttClient, jsonObjGlobal, jsonObjRoot);
    }

    String Sensor::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
       
        return ret;
    }

    void Sensor::loop() {

        if (cdr == nullptr) return; // nothing to automate

        unsigned long now = millis();
        if (now - lastMs < refreshMs) {
            return;
        }
        lastMs = now;
        //GlobalLogger.Info(F("Sensor::loop() exec"));

        HALValue val;
        HALOperationResult res = cdr->ReadSimple(val);
        if (res == HALOperationResult::Success) {
            //GlobalLogger.Info(F("Sensor::loop() exec Success"));
            if (!wasOnline) {
                const char* availabilityTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::Status);
                bool success = mqttClient.publish(availabilityTopicStr, HAL_JSON_HOME_ASSISTANT_AVAILABILITY_ONLINE);
                if (success) {
                    // this will make the availability update secure and non deadlock
                    GlobalLogger.Info(F("Sensor::loop() exec Success availability changed to active"));
                    wasOnline = true;
                }
                
            }
            const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
            // if the following fails then it will try again next update
            // could implement a try again mechanism but that would require 
            // refactor to make the code DRY
            mqttClient.publish(stateTopicStr, val.toString().c_str());
           // GlobalLogger.Info(F("Sensor::loop() exec Success sent to topic: "), stateTopicStr);
        } else {
            GlobalLogger.Info(F("Sensor::loop() exec fail"));
            if (wasOnline) {
                const char* availabilityTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::Status);
                bool success = mqttClient.publish(availabilityTopicStr, HAL_JSON_HOME_ASSISTANT_AVAILABILITY_OFFLINE);
                
                if (success) {
                    // this will make the availability update secure and non deadlock
                    GlobalLogger.Info(F("Sensor::loop() exec Success availability changed to inactive"));
                    wasOnline = false;
                }
            }
        }
    }
    void Sensor::begin() {

    }

    HALOperationResult Sensor::read(HALValue& val) {
        if (cdr != nullptr) {
            return cdr->ReadSimple(val);
        }
        return HALOperationResult::UnsupportedOperation;

    }
    HALOperationResult Sensor::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        if (!wasOnline) {
            const char* availabilityTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::Status);
            wasOnline = mqttClient.publish(availabilityTopicStr, HAL_JSON_HOME_ASSISTANT_AVAILABILITY_ONLINE);
        }
        const char* stateTopicStr = topicBasePath.SetAndGet(TopicBasePathMode::State);
        mqttClient.publish(stateTopicStr, val.toString().c_str());
        return HALOperationResult::Success;
    };
    
}