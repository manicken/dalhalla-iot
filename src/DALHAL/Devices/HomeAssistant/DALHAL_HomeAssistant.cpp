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
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_TopicBasePath.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_Constants.h>
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_DeviceDiscovery.h>

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include <System/DeviceUID.h> // getDeviceUID

#include "DALHAL_HomeAssistant_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase HomeAssistant::RegistryDefine = {
        Create,
        &JsonSchema::HomeAssistant::Root,
        nullptr /* no events available */
    };
    //volatile const void* keep_HomeAssistant = &DALHAL::HomeAssistant::RegistryDefine;

    Device* HomeAssistant::Create(DeviceCreateContext& context) {
        return new HomeAssistant(context);
    }
    
    HomeAssistant::HomeAssistant(DeviceCreateContext& context) : Device(context.deviceType) {
        mqttClient.setClient(wifiClient);
        JsonSchema::HomeAssistant::Extractors::Apply(context, this);
        
        mqttClient.setOnPublishHeaderCallback(this, MqttOnPublishHeaderCallback);
        mqttClient.setOnErrorCallback(this, MqttOnErrorCallback);

        // only do this once
        HA_DeviceDiscovery::SubscribeToCleanupTopic(mqttClient);
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
        const char* clientIdFormat_cStr = DALHAL_DEV_HOME_ASSISTANT_DD_BASENAME "_%s_%s";
        std::string uidStr = decodeUID(uid);
        const char* uid_cStr = uidStr.c_str();
        const char* deviceID_cStr = deviceID.c_str();

        int commandTopicStrLength = snprintf(nullptr, 0, clientIdFormat_cStr, deviceID_cStr, uid_cStr) + 1;
        char* clientIdStr = new char[commandTopicStrLength];
        snprintf(clientIdStr, commandTopicStrLength, clientIdFormat_cStr, deviceID_cStr, uid_cStr);
        Serial.printf("connecting using clientId:%s\n", clientIdStr);
        if (username.length() != 0 && password.length() != 0) { // the default connect is using CleanSession = true
            if (mqttClient.connect(clientIdStr, username.c_str(), password.c_str()) == false) {
                //Serial.println("ERR - HA MQTT nocould not connect using credentials");
            }
        } else {
            if (mqttClient.connect(clientIdStr) == false) { // the default connect is using CleanSession = true
                //Serial.println("ERROR - HASS MQTT could not connect without credentials");
            } 
        }
        delete[] clientIdStr;
        if (mqttClient.connected()) {
            Serial.println("HASS - MQTT connected to brooker");
            HA_DeviceDiscovery::SubscribeToCommandTopic(mqttClient, deviceID_cStr);
            
        }
    }

    

    void HomeAssistant::MqttOnPubishCompleteCallback(void* context, char* topic, uint16_t topicLength, uint8_t* payloadData, uint32_t payload_len) {
        //HomeAssistant& self = *static_cast<HomeAssistant*>(context);
        ZeroCopyString zcTopic(topic, topic + topicLength);

    }

    PubSubClientPacketReceiver HomeAssistant::MqttOnPublishHeaderCallback(void* context, char* topic, uint16_t topicLength, uint32_t payloadLength, PSC_PublishFlags flags) {
        //HomeAssistant& self = *static_cast<HomeAssistant*>(context);
        // topic length is already provided by the MQTT protocol
        ZeroCopyString zcTopic(topic, topic + topicLength);
        // preserve original topic view for later logging
        ZeroCopyString zcTmp = zcTopic;
        ZeroCopyString zcHead = zcTmp.SplitOffHead('/');
        ZeroCopyString zcTail = zcTmp.SplitOffTail('/');
        if (zcHead.Equals(F(DALHAL_DEV_HOME_ASSISTANT_DD_BASENAME))) {
            return PubSubClientPacketReceiver(PubSubClientPayloadSink::Buffer, MqttOnPubishCompleteCallback);
        } else if (zcHead.Equals(F(DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_HEAD)) &&
                   zcTail.Equals(F(DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_TAIL)) && flags.RETAIN())
        {
            // Expected topic format:
            // <homeassistant>/<type>/<deviceID>/<entityID>/config
            // Subscribed topic:
            // <homeassistant_const_string>/+/<deviceID>/+/config
            // 
            // The structure of <deviceID> is defined by DALHAL_HA_DeviceDiscovery 
            ZeroCopyString zcEntityID = zcTmp.SplitOffTail('/');


        } else {
            // failsafe warning, this will likely never happend
            GlobalLogger.Warn(F("ignoring message of HA MQTT topic: "), zcTopic);
        }
        // default if not specified above
        return PubSubClientPacketReceiver(PubSubClientPayloadSink::Discard, nullptr);
    }

    void HomeAssistant::MqttOnErrorCallback(void* context, PubSubClientResult error, PubSubClientErrorType type) {
        if (type == PubSubClientErrorType::FramingError) {
            static_cast<HomeAssistant*>(context)->Connect();
        } 
        GlobalLogger.Error(PubSubClientErrorToString(error));
    }

    void HomeAssistant::mqttCallback(char* topic, byte* payload, unsigned int length) {
#if defined(ESP32)
        Serial.printf("\r\nmqttCallback\r\ntopic:%s\r\n payload:%.*s\r\n", topic, (int)length, (char*)payload);
#elif defined(ESP8266)
        Serial1.printf("\r\nmqttCallback\r\ntopic:%s\r\n payload:%.*s\r\n", topic, (int)length, (char*)payload);
#endif

        // wrap in a ZeroCopyString for neat functions
        ZeroCopyString zcTopic(topic);
        const char* zcStartFirstDelimiter = zcTopic.FindChar('/');
        if (zcStartFirstDelimiter == nullptr) {
            // no point continue on either code path as then topic is not valid
            // in either case
            return;
        }
        ZeroCopyString zcStart = zcTopic.GetHead(zcStartFirstDelimiter);
        if (zcStart.NotEmpty() && zcStart == DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_HEAD) {
            // see DALHAL_HA_DD_CFG_TOPIC_FORMAT for the formatstr
            //const char* format = DALHAL_HA_DD_CFG_TOPIC_FORMAT;
            zcTopic.start = zcStartFirstDelimiter+1;
            if (zcTopic.IsEmpty()) {
                GlobalLogger.Error(F("zcTopic.IsEmpty()"));
                return;
            } // error incorrect topic string
            ZeroCopyString zcType = zcTopic.SplitOffHead('/');
            if (zcTopic.IsEmpty() || zcType.IsEmpty()) {
                GlobalLogger.Error(F("zcTopic.IsEmpty() || zcType.IsEmpty()"));
                return;
            } // error incorrect topic string
            ZeroCopyString zcUID = zcTopic.SplitOffHead('/');
            if (zcUID.IsEmpty()) {
                GlobalLogger.Error(F("zcUID.IsEmpty() - incorrect topic string"));
                return;
            } // error incorrect topic string
            if (zcUID.MoveStartAfter('_') == false) { // immutable framework id (DALHAL_DEV_HOME_ASSISTANT_BASENAME)
                GlobalLogger.Error(F("immutable framework id - incorrect topic string"));
                return; // error incorrect topic string
            } 
            if (zcUID.MoveStartAfter('_') == false) { // immutable device UID
                GlobalLogger.Error(F("immutable device UID - incorrect topic string"));
                return; // error incorrect topic string
            } 
            ZeroCopyString zcDeviceID = zcUID.SplitOffHead('_'); // mutable device ID, used to sort IDENTIFY entities in HA
            if (zcDeviceID.IsEmpty() || zcUID.IsEmpty()) {
                GlobalLogger.Error(F("incorrect topic string"));
                return; // error incorrect topic string
            }
            if (zcDeviceID.Equals(deviceID.c_str()) == false) {
                // deviceID have changed remove item
                GlobalLogger.Info(F("removed stale device bc deviceID change/removed"), topic);
                mqttClient.publish(topic, "");  // empty retained
                GlobalLogger.Error(F("empty retained"));
                return;
            }

            // now zcUID is the actual HA device entity UID
            HAL_UID uid = encodeUID(zcUID);
            if (uid.NotSet() || uid.Invalid()) { return; }
            // cleanup process
            // Iterate over all devices
            for (int i = 0; i < deviceCount; i++) {
                Device* dev = devices[i];
                if (uid != dev->uid) continue;
                if (zcType != dev->Type) continue;
                // found device just return
                GlobalLogger.Info(F("found device just return"));
                return;
                
            }
            // did not find device
            // Remove stale entity
            GlobalLogger.Info(F("removed stale device bc uid change/removed"), topic);
            mqttClient.publish(topic, "");  // empty retained
            return;
        }

        // find the lastSlash
        const char* lastSlash = zcTopic.FindCharReverse('/');
        if (lastSlash == nullptr) { Serial.println("last slash not found"); return; }

        // split of the tail
        ZeroCopyString zcTopicTail = zcTopic.SplitOffTail(lastSlash);

        const TopicSuffix cmdTopic = GetTopicSuffix(TopicBasePathMode::Command);
        if (zcTopicTail != cmdTopic.str) { Serial.printf("\nis not command:%s\n", zcTopicTail.ToString().c_str()); return; } // not command

        // find second-to-last slash
        const char* secondLastSlash = zcTopic.FindCharReverse('/');
        if (secondLastSlash == nullptr) { Serial.println("second last slash not found"); return; }

        ZeroCopyString zcUID = zcTopic.SplitOffTail(secondLastSlash);
        //std::string uidDbgStr = zcUID.ToString();
        //Serial.printf("\r\nextracted UID:>>>%s<<<\r\n", uidDbgStr.c_str());
        // encode as uid for fast lockup
        HAL_UID uid = encodeUID(zcUID);

        // Iterate over all devices
        for (int i = 0; i < deviceCount; i++) {
            Device* dev = devices[i];
            if (uid != dev->uid) continue;

            ZeroCopyString zcCmdStr((char*)payload, (char*)payload+length);
            dev->exec(zcCmdStr);
            //dev->HandleCommand(zcCmdStr);
            return;  // found a match
        }
        Serial.println("device not found");
    }

    HomeAssistant::~HomeAssistant() {
        if (devices) {
            for (int i=0;i<deviceCount;i++) {
                delete devices[i];
            }
            delete[] devices;
        }
    }

    String HomeAssistant::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\",\"items\":[";
        for (int i=0;i<deviceCount;i++) {
            ret += '{';
            ret += devices[i]->ToString();
            ret += '}';
            if (i<deviceCount-1) ret += ',';
        }
        ret += ']';
       
        return ret;
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
    }

    DeviceFindResult HomeAssistant::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(devices, deviceCount, path, this, outDevice);
    }

}