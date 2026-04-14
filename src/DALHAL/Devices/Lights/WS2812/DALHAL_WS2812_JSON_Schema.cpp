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

#include "DALHAL_WS2812_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringAnyOfArrayConstrained.h> // also ByArrayConstraints
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>

#include <WS2812FX.h>

#if !defined(ESP8266) && !defined(ESP32)
#define MODE_COUNT 80
#endif

namespace DALHAL {

    namespace JsonSchema {

        constexpr const char* formats[] = {"RGB",  "RBG",  "GRB",  "GBR",  "BRG",  "BGR",
                                           "WRGB", "WRBG", "WGRB", "WGBR", "WBRG", "WBGR",
                                           "RWGB", "RWBG", "GWRB", "GWBR", "BWRG", "BWGR",
                                           "RGWB", "RBWG", "GRWB", "GBWR", "BRWG", "BGWR",
                                           "RGBW", "RBGW", "GRBW", "GBRW", "BRGW", "BGRW",
                                            nullptr};

        constexpr const char* ifspeeds[] = {"KHZ800", "KHZ400", nullptr};

        constexpr SchemaUInt ledcountField = {"ledcount", FieldPolicy::Required, 1, 0, 1};

        constexpr ByArrayConstraints formatFieldConstraints = {formats, ByArrayConstraints::Policy::IgnoreCase};
        constexpr SchemaStringAnyOfArrayConstrained formatField = {"format", FieldPolicy::Required, "RGB", &formatFieldConstraints};

        constexpr ByArrayConstraints ifspeedFieldConstraints = {ifspeeds, ByArrayConstraints::Policy::IgnoreCase};
        constexpr SchemaStringAnyOfArrayConstrained ifspeedField = {"ifspeed", FieldPolicy::Optional, "KHZ800", &ifspeedFieldConstraints};

        constexpr SchemaUInt brightnessField = {"brightness", FieldPolicy::Optional, 1, 127, 127};
        constexpr SchemaUInt modeField = {"mode", FieldPolicy::Optional, 0, MODE_COUNT, 0}; // MODE_COUNT is from WS2812FX.h
        constexpr SchemaUInt fxspeedField = {"fxspeed", FieldPolicy::Optional, 0, 65535, 3000};

        constexpr const SchemaTypeBase* fields[] = {
            &disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &OutputPinField,
            &ledcountField,
            &formatField,
            &ifspeedField,
            &brightnessField,
            &modeField,
            &fxspeedField,
            nullptr,
        };

        constexpr JsonObjectSchema WS2812 = {
            "WS2812",
            fields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}