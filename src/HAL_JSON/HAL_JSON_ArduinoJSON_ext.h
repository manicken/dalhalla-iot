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

#pragma once


#include <Arduino.h> // Needed for String class

#include <ArduinoJson.h>
#include "../Support/Logger.h"
#include "HAL_JSON_Device_GlobalDefines.h"

namespace HAL_JSON {
    bool containsKeyIgnoreCase(JsonObject obj, const char* keyToFind);

    JsonVariant getValueIgnoreCase(JsonObject obj, const char* keyToFind);

    bool ValidateJsonStringField(const JsonVariant &jsonObj, const char* keyName);
    const char* GetValidatedJsonStringField(const JsonVariant &jsonObj, const char* keyName, const char* defaultValue);

    bool ValidateJsonStringField_noContains(const JsonVariant &jsonObj, const char* keyName);

    double ParseRefreshTime(const JsonVariant &jsonObj);
    uint32_t ParseRefreshTimeMs(const JsonVariant &jsonObj, const uint32_t defaultRefreshTimeMs);

    bool ValidateUINT8(const JsonVariant& jsonObj, const char* keyName);
    bool ValidateUINT32(const JsonVariant& jsonObj, const char* keyName);
    bool IsUINT32(const JsonVariant& jsonObj, const char* keyName);
    uint32_t GetAsUINT32(const JsonVariant& jsonObj, const char* keyName, uint32_t defaultValue);
    uint32_t GetAsUINT32(const JsonVariant& jsonObj, const char* keyName);
    uint16_t GetAsUINT16(const JsonVariant& jsonObj, const char* keyName);
    uint8_t GetAsUINT8(const JsonVariant& jsonObj, const char* keyName);
    
    inline bool IsConstChar(const JsonVariant& jsonObj) {
        return jsonObj.is<const char*>();
    }
    inline bool IsConstChar(const JsonVariant& jsonObj, const char* keyName) {
        return jsonObj[keyName].is<const char*>();
    }
    const char* GetAsConstChar(const JsonVariant& jsonObj, const char* keyName);

}