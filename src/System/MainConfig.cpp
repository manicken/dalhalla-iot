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

#include "MainConfig.h"

namespace MainConfig {

    String mDNS_name;
    std::string lastJSONread_Error = "";

    //WEBSERVER_TYPE *webserver = nullptr;

    bool ReadJson()
    {
        lastJSONread_Error = "";
        if (!LittleFS.exists(MAIN_CONFIG_FILES_PATH))
        {
            LittleFS.mkdir(MAIN_CONFIG_FILES_PATH);
            lastJSONread_Error = "dir did not exist";
            return false;
        }
        if (LittleFS.exists(MAIN_CONFIG_CONFIG_JSON_FILE) == false) {
            lastJSONread_Error = "cfg file did not exist";
            return false;
        }
        int size = LittleFS_ext::getFileSize(MAIN_CONFIG_CONFIG_JSON_FILE);
        char jsonBuffer[size + 1]; // +1 for null char
        if (LittleFS_ext::load_from_file(MAIN_CONFIG_CONFIG_JSON_FILE, jsonBuffer) != LittleFS_ext::FileResult::Success)
        {
            lastJSONread_Error = "error could not load json file";
            return false;
        }
        DynamicJsonDocument jsonDoc(size*2);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        if (error)
        {
            lastJSONread_Error = "deserialization failed: " + std::string(error.c_str());
            return false;
        }
        if (jsonDoc.is<JsonObject>() == false) {
            lastJSONread_Error = "root json is not a JsonObject";
            return false;
        }
        if (jsonDoc.containsKey("mDNSname")) {
            if (HAL_JSON::IsConstChar(jsonDoc,"mDNSname") == false) {
                SetDefault_mDNS_name();
            }
            else {
                const char* nameStr = jsonDoc["mDNSname"].as<const char*>();
                if (nameStr == nullptr) SetDefault_mDNS_name();
                else mDNS_name = String(nameStr);
            }
        }
        else SetDefault_mDNS_name();

        return true;
    }

    void SetDefault_mDNS_name()
    {
        mDNS_name = WIFI_CHIPID_PREFIX;
        uint64_t macAddrBigEndian = Convert::reverseMACaddress(WIFI_getChipId());
        mDNS_name.concat(String(macAddrBigEndian & 0xFFFFFF,HEX));
        mDNS_name.toUpperCase();
    }

    void OnReadJsonFail()
    {
        SetDefault_mDNS_name();
    }

    void Restart(AsyncWebServerRequest *req)
    {
        req->send(200, "text/html", "The device will now restart!");
        delay(500);
        ESP.restart(); // the only way to do a proper reload of main settings
    }

    void begin(WEBSERVER_TYPE &srv) {

        //webserver = &srv;
        srv.on(MAIN_CONFIG_URL_RELOAD_JSON, HTTP_GET, Restart);
        if (ReadJson() == false) {
            OnReadJsonFail();
            DEBUG_UART.println(lastJSONread_Error.c_str());
        }
    }
}