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

#define SERVICES_THINGSPEAK_H_

#include <ArduinoJson.h>
#include <LittleFS.h>

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#define WEBSERVER_TYPE ESP8266WebServer
#define DEBUG_UART Serial1
#elif defined(ESP32)
#include <HTTPClient.h>
#include "Support/fs_WebServer.h"
#define WEBSERVER_TYPE fs_WebServer
#define DEBUG_UART Serial
#endif
#include "../Support/LittleFS_ext.h"

//#include "DeviceManager.h"
#include "Support/ConvertHelper.h"
#include "HAL_JSON/HAL_JSON_ArduinoJSON_ext.h"

//#define TS_DEBUG_PRINT_AFTER_JSON_READ

namespace ThingSpeak
{
    int debug_dont_send_to_server = 0;
    int debug_output = 0;
    std::string lastError;

    WEBSERVER_TYPE *webserver = nullptr;
     
    const char TS_ROOT_URL[] = "http://api.thingspeak.com/update?api_key=";

    #define TS_ACTIVITY_LED_PIN            5
    #define TS_FILES_PATH                  F("/thingspeak")
    #define TS_CONFIG_JSON_FILE            "/thingspeak/cfg.json"
    #define TS_CONFIG_JSON_FILE_URL_RELOAD F("/thingspeak/refresh")
    #define TS_URL_GET_DATA                F("/thingspeak/getDataUrl")
    //#define TS_JSON_FIELD_API_KEY         F("api_key")
    //#define TS_JSON_FIELD_TEMP_FIELD      F("temp_field")
    //#define TS_JSON_FIELD_HUMIDITY_FIELD  F("humidity_field")

    #define TS_CHANNEL_MAX_FIELDS 8
    struct Channel
    {
        std::string api_write_key = "";
        /**
         * here each index in the array represents a field index,
         * and each value represents that "device" uid that it will get it's value from, 
         * a value of -1 represents a unused field and will get ignored when uploading data
         */
        int uids[TS_CHANNEL_MAX_FIELDS];

        Channel() {
            for (int i=0;i<TS_CHANNEL_MAX_FIELDS;i++) uids[i] = -1;
        }
    };

    Channel* channels = nullptr;
    int channelCount = 0;

    WiFiClient wifiClient;
    
    HTTPClient http;
    
    bool canPost = false;

    bool readJson()
    {
        if (!LittleFS.exists(TS_FILES_PATH))
        {
            LittleFS.mkdir(TS_FILES_PATH);
            lastError = "dir did not exist";
            return false;
        }
        if (LittleFS.exists(TS_CONFIG_JSON_FILE) == false) {
            lastError = "cfg file did not exist";
            return false;
        }
        int size = LittleFS_ext::getFileSize(TS_CONFIG_JSON_FILE);
        char jsonBuffer[size + 1]; // +1 for null char
        if (LittleFS_ext::load_from_file(TS_CONFIG_JSON_FILE, jsonBuffer) != LittleFS_ext::FileResult::Success)
        {
            lastError = "could not load json file";
            return false;
        }
        DynamicJsonDocument jsonDoc(1024);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        if (error)
        {
            lastError = "deserialization failed: " + std::string(error.c_str());
            return false;
        }
        if (!jsonDoc.is<JsonObject>()) {
            lastError = "jsonDoc is not a valid JsonObject\n";
            return false;
        }
        channelCount = jsonDoc.size();
        if (channels != nullptr) { delete[] channels; channels = nullptr; }// free(channels);
        //channels = (Channel*)malloc(sizeof(Channel) * channelCount);
        channels = new Channel[channelCount];
        if (channels == nullptr){ lastError="could not allocate memory"; return false; }
        int currChIndex = 0;
        lastError = "";
        for (JsonPair ckv : jsonDoc.as<JsonObject>()) {
            if (currChIndex >= channelCount) { // this should not happen
                lastError += "currChIndex exceeds channel array size\n";
                return false;
            }
            Channel &channel = channels[currChIndex++];
            const char* api_key_str = ckv.key() ? ckv.key().c_str() : nullptr;
            if (api_key_str == nullptr) {
                lastError+="could not get api_key_str @ currChIndex:" + std::to_string(currChIndex);
                continue;
            }
            if (strncmp(api_key_str, "debug", sizeof("debug")-1) == 0) {
                if (ckv.value() == nullptr) continue; // no error print here
                if (ckv.value().is<JsonObject>() == false) continue; // no error print here
                JsonVariant debug = ckv.value();
                if (debug.containsKey("dont_send_to_server")) debug_dont_send_to_server = 1;
                else debug_dont_send_to_server = 0;
                if (debug.containsKey("debug_output")) debug_output = 1;
                else debug_output = 0;

                continue;
            }
            else if (strncmp(api_key_str, "comment", sizeof("comment")-1) == 0) {
                continue;
            }
            channel.api_write_key = std::string(api_key_str);
            if (ckv.value() == nullptr) {
                lastError+="json object value is null @ currChIndex:" + std::to_string(currChIndex) + "\n";
                continue;
            }
            if (ckv.value().is<JsonObject>() == false) {
                lastError+="json object don't contain a json object @ currChIndex:" + std::to_string(currChIndex) + "\n";
                continue;
            }
            int currIndex = -1; // debug use only
            for (JsonPair fkv : ckv.value().as<JsonObject>()) {
                currIndex++; // debug use only
                const char* fieldIndexStr = fkv.key() ? fkv.key().c_str() : nullptr;
                if (fieldIndexStr == nullptr) {
                    lastError+="fieldIndex is not a string @ currIndex:" + std::to_string(currIndex) + "\n";
                    continue;
                }
                if (Convert::isInteger(fieldIndexStr) == false) {
                    lastError+="fieldIndex is not a integer @ currIndex:" + std::to_string(currIndex) + "\n";
                    continue;
                }
                int fieldIndex = atoi(fieldIndexStr);
                if (fieldIndex < 1 || fieldIndex > (TS_CHANNEL_MAX_FIELDS)) {
                    lastError += "fieldIndex out of bounds: " + std::to_string(fieldIndex) + "\n";
                    continue;
                }
                uint32_t uid = 0;
                if (fkv.value().is<int>()) {
                    uid = fkv.value().as<int>();
                }
                else if (HAL_JSON::IsConstChar(fkv.value())) {
                    const char* uid_asciiHexString = fkv.value().as<const char*>();
                    if (uid_asciiHexString == nullptr) { lastError+="uid could not convert to a string @ fieldIndex key:" + std::to_string(fieldIndex) + "\n"; return false; }
                    char* endptr = nullptr;
                    uid = static_cast<uint32_t>(std::strtoul(uid_asciiHexString, &endptr, 16));
                    if (endptr == uid_asciiHexString) {
                        lastError += "Error @ thingspead readjson: No valid conversion could be performed (invalid input).\n";
                    } else if (*endptr != '\0') {
                        lastError += "Warning @ thingspead readjson: Additional characters after number: ";
                        lastError += std::string(endptr);
                        lastError += "\n";
                    }
                }
                else {
                    lastError+="uid is either a number of a ascii hex string value @ fieldIndex key:" + std::to_string(fieldIndex) + "\n";
                    continue;
                }
                
                channel.uids[fieldIndex-1] = uid;
            }
        }

#ifdef TS_DEBUG_PRINT_AFTER_JSON_READ
        for (int ci=0;ci<channelCount;ci++)
        {
            DEBUG_UART.println("channel api_key:" + String(channels[ci].api_write_key.c_str()));
            DEBUG_UART.println("channel fields:");
            for (int fi=0;fi<TS_CHANNEL_MAX_FIELDS;fi++)
            {
                DEBUG_UART.println("field:" + String(fi+1) + ", uid:" + String(channels[ci].uids[fi]));
            }
            
        }
#endif

        canPost = true;
        return true;
    }
    

    bool sendDataBackToWebbrowser = false;

    void SendData()
    {
        digitalWrite(TS_ACTIVITY_LED_PIN, HIGH); // TODO move this and make it more customaziable
        //DEBUG_UART.println("TS SendData");
        if (channels == nullptr) return;
#if defined(DEVICE_MANAGER_H)
        if (DeviceManager::getAllOneWireTemperatures() == false) return;
#endif
        for (int ci=0;ci<channelCount;ci++)
        {
            if (channels[ci].api_write_key.size() == 0) continue;

            std::string fieldData = "";
            for (int fi=0;fi<8;fi++)
            {
                if (channels[ci].uids[fi] == -1) continue;
                float value = 0;
#if defined(DEVICE_MANAGER_H)
                if (DeviceManager::getValue(channels[ci].uids[fi], &value) == false) continue;
#endif
                fieldData += "&field" + std::to_string(fi+1) + "=" + Convert::floatToString(value);
            }
            if (fieldData.length() == 0) continue;
            std::string urlApi = TS_ROOT_URL + channels[ci].api_write_key + fieldData;

            if (debug_dont_send_to_server == 0 && sendDataBackToWebbrowser == false) {
                http.begin(wifiClient, urlApi.c_str());
                
                int httpCode = http.GET();
                if (httpCode > 0) DEBUG_UART.println(F("[OK]\r\n"));
                else DEBUG_UART.println(F("[FAIL]\r\n"));

                http.end();
            }
            if (debug_output == 1) {
                DEBUG_UART.println(urlApi.c_str());
            }
            if (sendDataBackToWebbrowser) {
                sendDataBackToWebbrowser = false;
                webserver->send(200, "text/html", urlApi.c_str()); // in case the request is from a url to test
            }
        }
        digitalWrite(TS_ACTIVITY_LED_PIN, LOW); // TODO move this and make it more customaziable

    }

    void reloadJson()
    {
        if (readJson())
            webserver->send(200,F("text/plain"), F("Thingspeak loadSettings OK"));
        else
            webserver->send(200,F("text/plain"), F("Thingspeak loadSettings error"));
    }

    void htmlSendDataTest()
    {
        sendDataBackToWebbrowser = true;
        SendData();
    }

    void setup(WEBSERVER_TYPE &srv) {
        webserver = &srv;
        srv.on(TS_CONFIG_JSON_FILE_URL_RELOAD, reloadJson);
        srv.on(TS_URL_GET_DATA, htmlSendDataTest);
        readJson();
        pinMode(TS_ACTIVITY_LED_PIN, OUTPUT);
    }
}