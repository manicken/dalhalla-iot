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

#include "HAL_JSON_REST_Value.h"

namespace HAL_JSON {

    bool REST_Value::VerifyJSON(const JsonVariant &jsonObj) {
        if (ValidateJsonStringField(jsonObj, "url") == false){ SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_REMOTE_FETCH_REST_VERIFY_JSON); return false; }
        return true;
    }

    Device* REST_Value::Create(const JsonVariant &jsonObj, const char* type) {
        return new REST_Value(jsonObj, type);
    }

    REST_Value::REST_Value(const JsonVariant &jsonObj, const char* type)
        : HAL_JSON::Device(type),
        lastRefresh(0)
    {
        uid = encodeUID(GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID));
        refreshTimeMs = ParseRefreshTimeMs(jsonObj, 2000);
        remoteUrl = GetAsConstChar(jsonObj, "url");
    }

    HAL_JSON::HALOperationResult REST_Value::read(HAL_JSON::HALValue& val) {
        val = cachedValue;
        return HAL_JSON::HALOperationResult::Success;
    }

    void REST_Value::loop() {
        uint32_t now = millis();
        if (now - lastRefresh >= refreshTimeMs) {
            lastRefresh = now;
            fetchRemoteValue();
        }
    }

    void REST_Value::fetchRemoteValue() {
        int getCode = 0;
        HTTPClient client;
        WiFiClient wifiClient;
        if (client.begin(wifiClient, remoteUrl)) {
            getCode = client.GET();
        }

        if (getCode != 200) {
#if defined(_WIN32) || defined(__linux__)
            printf("\n[RemoteFetch_REST] Failed to fetch (%d): %s\n", getCode, remoteUrl.c_str());
#endif
            return;
        }
        String resp = client.getString();
        ZeroCopyString zcStrData(resp.c_str());
        if (zcStrData.FindString("\"error\"") != nullptr) {
            return;
        }
        if (zcStrData.FindString("\"value\"") == nullptr) {
            return;
        }
        zcStrData.SplitOffHead(':');
        zcStrData.end--; // trim off the last }
        float val = 0;
        if (zcStrData.ConvertTo_float(val)) {
            cachedValue = val;
        }
    }
}