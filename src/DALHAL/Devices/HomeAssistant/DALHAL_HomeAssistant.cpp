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

#include "DALHAL_HomeAssistant.h"
#if defined(ESP32) || (defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Arduino.h>

#include <PubSubClient.h>
#include <PubSubClient_ErrorStrings.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_DeviceTypeReg.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_Constants.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_DeviceDiscovery.h>

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include <DALHAL/Core/Types/DALHAL_Device.h>
#include "Core/DALHAL_HA_Device.h"
#include "Core/DALHAL_HA_DeviceEntity.h"
#include <System/DeviceUID.h> // getDeviceUID

#include "DALHAL_HomeAssistant_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase HomeAssistant::RegistryDefine = {
        Create,
        &JsonSchema::HomeAssistant::Root,
        /*nullptr*/ /* no events available */
    };
    
    /* override */
    const Registry::DefineBase* HomeAssistant::GetRegistryDefine() {
        return &RegistryDefine;
    }

    Device* HomeAssistant::Create(DeviceCreateContext& context) {
        return new HomeAssistant(context);
    }
    
    HomeAssistant::HomeAssistant(DeviceCreateContext& context) : Device(context.deviceType) {
        mqttClient.setClient(wifiClient);
        JsonSchema::HomeAssistant::Extractors::Apply(context, this);
        
        mqttClient.setOnPublishHeaderCallback(this, MqttOnPublishHeaderCallback);
        mqttClient.setOnErrorCallback(this, MqttOnErrorCallback);
    }

    void HomeAssistant::ConfigureMqttClient() {
        if (WiFi.hostByName(host.c_str(), ip)) {
            // Successfully resolved
            mqttClient.setServer(ip, port);
        } else {
            Serial.printf("Failed to resolve %s, will retry later\n", host.c_str());
            // Optionally, store host string for next retry attempt
            mqttClient.setServer(host.c_str(), port); // last-resort attempt
            // could also have retry when doing automatic reconnect
        }
    }

    void HomeAssistant::Connect() {
        const char* clientIdStr = DeviceUID::Get();
        GlobalLogger.Info(F("HASS MQTT - connecting using clientId:"), clientIdStr);// Serial.println(clientIdStr);
        if (username.length() != 0 && password.length() != 0) { // the default connect is using CleanSession = true
            if (mqttClient.connect(clientIdStr, username.c_str(), password.c_str()) == false) {
                GlobalLogger.Error(F("ERROR - HASS MQTT - could not connect using credentials"));
            }
        } else {
            if (mqttClient.connect(clientIdStr) == false) { // the default connect is using CleanSession = true
                GlobalLogger.Error(F("ERROR - HASS MQTT - could not connect without credentials"));
            } 
        }
        if (mqttClient.connected()) {
            GlobalLogger.Info(F("HASS MQTT - connected to brooker"));
            
        }
    }

    void HomeAssistant::MqttOnPubishCompleteCallback(void* context, char* topic, uint16_t topicLength, uint8_t* payloadData, uint32_t payload_len) {
        //HomeAssistant& self = *static_cast<HomeAssistant*>(context);
        ZeroCopyString zcTopic(topic, topic + topicLength);
        // preserve original topic view for later logging
        ZeroCopyString zcTmp = zcTopic;
        ZeroCopyString zcHead = zcTmp.SplitOffHead('/');
        ZeroCopyString zcTail = zcTmp.SplitOffTail('/');
        if (zcHead.Equals(F(DALHAL_DEV_HOME_ASSISTANT_DD_BASENAME)) && 
            zcTail.Equals(F(DALHAL_DEV_HOME_ASSISTANT_DD_COMMAND_TOPIC_TAIL)))
        {
            ZeroCopyString zcEntityID = zcTmp.SplitOffTail('/');

            HA_DeviceEntity* item = static_cast<HomeAssistant*>(context)->findHassDevice(zcEntityID);

            if (item == nullptr) {
                GlobalLogger.Warn(F("PSCP Complete CB - could not find device with hass_uid on HA MQTT topic: "), zcTopic);
                return;
            }
            const ZeroCopyString zcPayload((const char*)payloadData, (const char*)payloadData+payload_len);

            HALOperationResult res = item->ha_apply(zcPayload);

            if (res != HALOperationResult::Success) {
                // really need to modify so that the second parameter given to GlobalLogger.Error can be annother FlashStringHelper
                // or maybe a given struct so that the type can be given
                GlobalLogger.Error(F("PSCP Complete CB - item->exec fail:"), String(HALOperationResultToString(res)).c_str() );
            }

        } else if (zcHead.Equals(F(DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_HEAD)) &&
                   zcTail.Equals(F(DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_TAIL)))
        {
            ZeroCopyString zcEntityID = zcTmp.SplitOffTail('/');
            
            Serial.printf("PSCP Complete CB - rx cleanup check for: %.*s\r\n", zcEntityID.Length(), zcEntityID.start);

        } else {
            // failsafe warning, this will likely never happend
            GlobalLogger.Warn(F("PSCP Complete CB - ignoring message of HA MQTT topic: "), zcTopic);
        }
    }

    PubSubClientPacketReceiver HomeAssistant::MqttOnPublishHeaderCallback(void* context, char* topic, uint16_t topicLength, uint32_t payloadLength, PSC_PublishFlags flags) {
        //Serial.println(F("PubSubClientPacketReceiver HomeAssistant::MqttOnPublishHeaderCallback"));
        //Serial.printf("\r\nPSC HEAD CB - topic= %.*s retain=%d\r\n", topicLength, topic, flags.RETAIN());
        HomeAssistant& self = *static_cast<HomeAssistant*>(context);
        // topic length is already provided by the MQTT protocol
        ZeroCopyString zcTopic(topic, topic + topicLength);
        // preserve original topic view for later logging
        ZeroCopyString zcTmp = zcTopic;
        ZeroCopyString zcHead = zcTmp.SplitOffHead('/');
        ZeroCopyString zcTail = zcTmp.SplitOffTail('/');
        //Serial.printf("PSCPRx CB topic head: >>>%.*s<<<\r\n", zcHead.Length(), zcHead.start);
        //Serial.printf("PSCPRx CB topic tail: >>>%.*s<<<\r\n", zcTail.Length(), zcTail.start);
        if (zcHead.Equals(F(DALHAL_DEV_HOME_ASSISTANT_DD_BASENAME)) && 
            zcTail.Equals(F(DALHAL_DEV_HOME_ASSISTANT_DD_COMMAND_TOPIC_TAIL)))
        {
            // first check if payload can fit into internal buffer
            // if it can not fit the payload is discarded
            // this can be fixed in the future if needed by providing a external buffer
            // however with the current implementation it's almost unlikely to happend
            if (self.mqttClient.getBufferSize() < (MQTT_MAX_HEADER_SIZE + topicLength + payloadLength)) {
                GlobalLogger.Error(F("PSCP HEAD CB - the internal buffersize can not fit the command payload"), zcTopic);
                return PubSubClientPacketReceiver(PubSubClientPayloadSink::Discard, nullptr);
            }
            return PubSubClientPacketReceiver(PubSubClientPayloadSink::Buffer, MqttOnPubishCompleteCallback);

        } else if (zcHead.Equals(F(DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_HEAD)) &&
                   zcTail.Equals(F(DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_TAIL)))
        {
            if (flags.RETAIN() == false) {
                GlobalLogger.Warn(F("PSCP HEAD CB - 'cleanup' topic did not have RETAIN FLAG SET: "), zcTopic);
            }
            if (payloadLength == 0) {
                // cleanup allready done
                return PubSubClientPacketReceiver(PubSubClientPayloadSink::Discard, nullptr);
            }
            // Expected topic format:
            // <homeassistant_const_string>/<type>/dalhal_<deviceID>/<entityID>/config
            // Subscribed topic:
            // <homeassistant_const_string>/+/dalhal_<deviceID>/+/config
            // 
            // The structure of <deviceID> is defined by DALHAL_HA_DeviceDiscovery 
            ZeroCopyString zcEntityID = zcTmp.SplitOffTail('/');
            ZeroCopyString zcType = zcTmp.SplitOffHead('/');

            bool overriddenRemove = false;
            if (DeviceUID::Overridden()) {
                zcTmp.SplitOffHead('_'); // just discard dalhal fixed string
                ZeroCopyString zcDeviceUID = zcTmp.SplitOffTail('/');
                if (zcDeviceUID.Equals(DeviceUID::Get()) == false) {
                    // this mean that the deviceID is overriden
                    // and this topic need to be 'removed'
                    overriddenRemove = true;
                }
            }
            
            Serial1.printf("PSCP HEAD CB - rx cleanup check for: %.*s type:%.*s\r\n", zcEntityID.Length(), zcEntityID.start, zcType.Length(), zcType.start);

            const HA_DeviceEntity* item = nullptr;
            if (!overriddenRemove) {
                item = self.findHassDevice(zcEntityID);
            }
            if (item == nullptr) {
                
                bool success = HA_DeviceDiscovery::RemoveCfgTopic(self.mqttClient, zcType, zcEntityID);
                if (success) {
                    Serial1.println(F("PSCP HEAD CB - cleanup executed"));
                } else {
                    Serial1.println(F("PSCP HEAD CB - cleanup fail"));
                }
            }

        } else {
            // failsafe warning, this will likely never happend
            GlobalLogger.Warn(F("PSCP HEAD CB - ignoring message of HA MQTT topic: "), zcTopic);
        }
        // default if not specified above
        return PubSubClientPacketReceiver(PubSubClientPayloadSink::Discard, nullptr);
    }

    void HomeAssistant::MqttOnErrorCallback(void* context, PubSubClientResult error, PubSubClientErrorType type) {
        if (type == PubSubClientErrorType::FramingError) {
            HomeAssistant& self = *static_cast<HomeAssistant*>(context);
            self.Connect();
            if (self.mqttClient.connected() && self.initializedOnce) {
                HA_DeviceDiscovery::SubscribeToCommandTopic(self.mqttClient);
                HA_DeviceDiscovery::SubscribeToCleanupTopic(self.mqttClient);
            }
        } 
        GlobalLogger.Error(PubSubClientErrorToString(error));
    }

    HomeAssistant::~HomeAssistant() {
        if (devices) {
            for (int i=0;i<deviceCount;i++) {
                delete devices[i];
            }
            delete[] devices;
        }
    }

    void HomeAssistant::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("items"));
        sbs.write_json_array_begin();
        
        for (int i = 0; i < deviceCount; ++i) {
            if (i > 0) { sbs.write_json_value_separator(); }
            sbs.write_json_object_begin();
            devices[i]->PrintTo(sbs);
            sbs.write_json_object_end();
        }
        sbs.write_json_array_end();
    }

    void HomeAssistant::loop() {
        if (!devices) return;
#if defined(ESP8266) || defined(ESP32)
        if (WiFi.status() != WL_CONNECTED) { return; } // need some timer to print this otherwise it will just flood the Serial port Serial.println("WiFi not connected, skipping HomeAssistant task"); }
#endif
        if (mqttClient.connected() == false) {
            
            unsigned long now = millis();
            if (now - lastReconnectAttempt >= reconnectInterval) {
                lastReconnectAttempt = now;
                GlobalLogger.Info(F("HASS - MQTT connection is down, trying to reconnect"));
                Connect();
            }
            return;
        }
        mqttClient.loop();

        for (int i=0;i<deviceCount;i++) {
            devices[i]->loop();
        }

    }

    void HomeAssistant::begin() {
        if (!devices) return;
        for (int i=0;i<deviceCount;i++) {
            devices[i]->begin();
        }
        if (mqttClient.connected()) {
            HA_DeviceDiscovery::SubscribeToCommandTopic(mqttClient);
            HA_DeviceDiscovery::SubscribeToCleanupTopic(mqttClient);
        }
        initializedOnce = true;
    }

    DeviceFindResult HomeAssistant::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(devices, deviceCount, path, this, outDevice);
    }

    HA_DeviceEntity* HomeAssistant::findHassDevice(const ZeroCopyString& zcHassUid) {
        if (devices == nullptr || deviceCount == 0) { return nullptr; }
        
        for (int i=0;i<deviceCount;++i) {
            HA_DeviceEntity* item = static_cast<HA_Device*>(devices[i])->findHassDevice(zcHassUid);
            //const HA_DeviceEntity* item = devices[i]->findHassDevice(zcHassUid);
            if (item != nullptr) return item;
        }
        
        return nullptr;
    }

    // debug entry
    /*HALOperationResult HomeAssistant::exec_ddTest(Device* device) {
        //if (cmd.EqualsIC(F("ddTest"))) {
            StaticJsonDocument<256> jsonDoc;
            JsonObject root = jsonDoc.to<JsonObject>();
            root["unit_of_measurement"] = "°C";
            root["device_class"] = "temperature";

            HA_DD_Context ha_dd_ctx = {"uid", "deviceId", "sensor", "deviceName", "groupID", "groupName", root};  
            DALHAL::HA_DeviceDiscovery::SendDiscovery(static_cast<HomeAssistant*>(device)->mqttClient, ha_dd_ctx, nullptr);
            return HALOperationResult::Success;
        //}
        //return HALOperationResult::UnsupportedCommand;
    }*/

}