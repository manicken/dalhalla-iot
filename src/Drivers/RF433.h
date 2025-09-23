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

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include "../Support/CharArrayHelpers.h"
#include "../HAL_JSON/HAL_JSON_ArduinoJSON_ext.h"

#define HAL_JSON_KEYNAME_TX433_HEXID "hexid"
#define HAL_JSON_KEYNAME_TX433_ALPHA_NUMERIC_ID "anid"
#define HAL_JSON_KEYNAME_TX433_GRP_BTN "grp_btn"
#define HAL_JSON_KEYNAME_TX433_STATE "state"
#define HAL_JSON_KEYNAME_TX433_BTN "btn"
#define HAL_JSON_KEYNAME_TX433_CH "ch"
#define HAL_JSON_DEVICE_RF433

namespace RF433 {

    /** used only for Learning code data send mask */
    constexpr uint32_t VALUE_2_POW_31 = 1u << 31;
    /** used both by Learning code and fixed code data send masks */
    constexpr uint32_t VALUE_2_POW_23 = 1u << 23;
    constexpr uint32_t VALUE_2_POW_3 = 1u << 3;

#if defined(__AVR__)
    using arch_word_t = uint8_t;
#elif defined(__arm__) || defined(__ARM__) || defined(__aarch32__) || defined(__ARM_ARCH_ISA_THUMB)
    using arch_word_t = uint32_t;
#elif defined(__x86_64__) || defined(_M_X64)
    using arch_word_t = uint64_t;
#elif defined(__i386__) || defined(_M_IX86)
    using arch_word_t = uint32_t;
#elif defined(ESP32) || defined(ESP8266)
    using arch_word_t = uint32_t;
#else
    #error "Unknown architecture"
#endif

    arch_word_t Get1AsciiHex(arch_word_t value );
    arch_word_t Get1AsciiHexValue(arch_word_t hex);
    arch_word_t GetAsciiHexValue(const char *dataArrayIn, arch_word_t nibbleCount);
    

    uint32_t decode5AlphaNumericTo4byteId(const std::string& id);

    void init(arch_word_t _pin);

    //  ███████ ██ ██   ██ ███████ ██████       ██████  ██████  ██████  ███████ 
    //  ██      ██  ██ ██  ██      ██   ██     ██      ██    ██ ██   ██ ██      
    //  █████   ██   ███   █████   ██   ██     ██      ██    ██ ██   ██ █████   
    //  ██      ██  ██ ██  ██      ██   ██     ██      ██    ██ ██   ██ ██      
    //  ██      ██ ██   ██ ███████ ██████       ██████  ██████  ██████  ███████ 

    void Send433FC_One(void);
    void Send433FC_Zero(void);
    void Send433FC_Float(void);
    void Send433FC_Sync(void);

    void SendTo433_FC_bits(uint32_t data, uint32_t mask);
    void SendTo433_FC(uint32_t data);
    void SendTo433_FC(uint32_t staticData, arch_word_t state);

    //  ██      ███████  █████  ██████  ███    ██ ██ ███    ██  ██████       ██████  ██████  ██████  ███████ 
    //  ██      ██      ██   ██ ██   ██ ████   ██ ██ ████   ██ ██           ██      ██    ██ ██   ██ ██      
    //  ██      █████   ███████ ██████  ██ ██  ██ ██ ██ ██  ██ ██   ███     ██      ██    ██ ██   ██ █████   
    //  ██      ██      ██   ██ ██   ██ ██  ██ ██ ██ ██  ██ ██ ██    ██     ██      ██    ██ ██   ██ ██      
    //  ███████ ███████ ██   ██ ██   ██ ██   ████ ██ ██   ████  ██████       ██████  ██████  ██████  ███████

    void Send433LC_One(void);
    void Send433LC_Zero(void);
    void Send433LC_Start(void);

    void SendTo433_LC_bits(uint32_t data, uint32_t mask);
    void SendTo433_LC(uint32_t data);
    void SendTo433_LC(uint32_t staticData, arch_word_t state);

    //  ███████ ███████ ███    ██ ██████      ██████  ██    ██          ██ ███████  ██████  ███    ██ 
    //  ██      ██      ████   ██ ██   ██     ██   ██  ██  ██           ██ ██      ██    ██ ████   ██ 
    //  ███████ █████   ██ ██  ██ ██   ██     ██████    ████            ██ ███████ ██    ██ ██ ██  ██ 
    //       ██ ██      ██  ██ ██ ██   ██     ██   ██    ██        ██   ██      ██ ██    ██ ██  ██ ██ 
    //  ███████ ███████ ██   ████ ██████      ██████     ██         █████  ███████  ██████  ██   ████ 

    bool Validate_FC_JSON(const JsonVariant &jsonObj);
    uint32_t Get433_SFC_Data(const JsonVariant &jsonObj);
    uint32_t Get433_AFC_Data(const JsonVariant &jsonObj);
    void DecodeFromJSON_SFC(const JsonVariant &jsonObj);
    void DecodeFromJSON_AFC(const JsonVariant &jsonObj);

    void GetLCSettings(const JsonVariant &jsonObj);
    void DecodeFromJSON_LC(const JsonVariant &jsonObj);

    void DecodeFromJSON(const JsonVariant &jsonObj);
    //void DecodeFromJSON(const String& jsonStr);
    void DecodeFromJSON(std::string jsonStr);

    bool VerifyLC_JSON(const JsonVariant &jsonObj);
    uint32_t Get433_LC_Data(const JsonVariant &jsonObj);

    /* example json
    {
    "cmd": "RF433",
    "type":"slc",
    "uid":"3AD4FF6",
    "btn":"F",
    "state":"0",
    "grp_btn":"0"
    }
    */
}