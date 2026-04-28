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
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringAnyOfArrayConstrained.h> // also ByArrayConstraints
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringAnyOfByFuncConstrained.h> 
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>

#include <WS2812FX.h>
#include "DALHAL_WS2812.h"

#if !defined(ESP8266) && !defined(ESP32)
#define MODE_COUNT 80
#endif

namespace DALHAL {

    namespace JsonSchema {

        namespace WS2812 {

            struct LedFormatDefine {
                const char* name;
                uint16_t value;
            };

            constexpr LedFormatDefine notFoundItem = {nullptr, 0};

            constexpr const char* LED_FORMAT_RGB = "RGB";
            constexpr LedFormatDefine formatsTable[] = {
                {LED_FORMAT_RGB, NEO_RGB}, {"RBG", NEO_RBG}, 
                {"GRB", NEO_GRB}, {"GBR", NEO_GBR}, 
                {"BRG", NEO_BRG}, {"BGR", NEO_BGR},
                {"WRGB", NEO_WRGB}, {"WRBG", NEO_WRBG}, 
                {"WGRB", NEO_WGRB}, {"WGBR", NEO_WGBR}, 
                {"WBRG", NEO_WBRG}, {"WBGR", NEO_WBGR},
                {"RWGB", NEO_RWGB}, {"RWBG", NEO_RWBG}, 
                {"GWRB", NEO_GWRB}, {"GWBR", NEO_GWBR}, 
                {"BWRG", NEO_BWRG}, {"BWGR", NEO_BWGR},
                {"RGWB", NEO_RGWB}, {"RBWG", NEO_RBWG},
                {"GRWB", NEO_GRWB}, {"GBWR", NEO_GBWR},
                {"BRWG", NEO_BRWG}, {"BGWR", NEO_BGWR},
                {"RGBW", NEO_RGBW}, {"RBGW", NEO_RBGW},
                {"GRBW", NEO_GRBW}, {"GBRW", NEO_GBRW},
                {"BRGW", NEO_BRGW}, {"BGRW", NEO_BGRW}
            };
            constexpr size_t formatsTableSize = sizeof(formatsTable)/sizeof(formatsTable[0]);

            const LedFormatDefine& GetFormat(const char* name) {
                for (int i=0; i<(int)formatsTableSize; ++i) {
                    if (strcasecmp(formatsTable[i].name, name) == 0) {
                        return formatsTable[i];
                    }
                }
                return notFoundItem;
            }

            bool CheckFormatType(void* ctx, const char* type) { // here ctx is not used as we can access the table directly
                const LedFormatDefine& def = GetFormat(type);
                if (def.name != nullptr) {
                    return true;
                }
                return false;
            }

            std::string GetFormatStrings(void* ctx) { // here ctx is not used as we can access the table directly
                std::string out;
                out = '[';
                for (int i=0; i<(int)formatsTableSize; ++i) {
                    if (i>0) {
                        out += ',';
                    }
                    out += '"'; out += formatsTable[i].name; out += '"';
                }
                out += ']';
                return out;
            }

            constexpr const char* INTERFACE_SPEED_KHZ800 = "KHZ800";
            constexpr const char* INTERFACE_SPEED_KHZ400 = "KHZ400";
            constexpr const char* ifspeeds[] = {INTERFACE_SPEED_KHZ800, INTERFACE_SPEED_KHZ400, nullptr};

            constexpr SchemaUInt ledcountField = {"ledcount", FieldPolicy::Required, (unsigned int)1, (unsigned int)0, (unsigned int)1};

            

            // note here the context is not used as the source is known
            constexpr SchemaStringAnyOfByFuncConstrained formatField = {"format", FieldPolicy::Required, LED_FORMAT_RGB, CheckFormatType, GetFormatStrings, nullptr};

            constexpr ByArrayConstraints ifspeedFieldConstraints = {ifspeeds, ByArrayConstraints::Policy::IgnoreCase};
            constexpr SchemaStringAnyOfArrayConstrained ifspeedField = {"ifspeed", FieldPolicy::Optional, INTERFACE_SPEED_KHZ800, &ifspeedFieldConstraints};

            constexpr SchemaUInt brightnessField = {"brightness", FieldPolicy::Optional, (unsigned int)1, (unsigned int)127, (unsigned int)127};
            constexpr SchemaUInt modeField = {"mode", FieldPolicy::Optional, (unsigned int)0, (unsigned int)MODE_COUNT, (unsigned int)0}; // MODE_COUNT is from WS2812FX.h
            constexpr SchemaUInt fxspeedField = {"fxspeed", FieldPolicy::Optional, (unsigned int)0, (unsigned int)65535, (unsigned int)3000};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &CommonPins::OutputPinField,
                &ledcountField,
                &formatField,
                &ifspeedField,
                &brightnessField,
                &modeField,
                &fxspeedField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "WS2812",
                fields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(DALHAL::DeviceCreateContext& context, DALHAL::WS2812* out) {
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*(context.jsonObjItem)));
                uint8_t pin = JsonSchema::CommonPins::OutputPinField.ExtractFrom(*(context.jsonObjItem));
                uint32_t numLeds = JsonSchema::WS2812::ledcountField.ExtractFrom(*(context.jsonObjItem));

                neoPixelType ws2812cfgData = 0; // basically a uint16_t
                const char* ledFormat_cStr = JsonSchema::WS2812::formatField.ExtractFrom(*(context.jsonObjItem));
                ws2812cfgData |= GetFormat(ledFormat_cStr).value; // as this is prevalidated there is no situation where this returns the unknown default

                const char* speedStr = JsonSchema::WS2812::ifspeedField.ExtractFrom(*(context.jsonObjItem));
                // no speedStr null check is needed here as getting a field string allways returns a empty string "",
                // but here it returns the default INTERFACE_SPEED_KHZ800 if the optional field is missing
                if (strcasecmp(speedStr, INTERFACE_SPEED_KHZ800)) { 
                    ws2812cfgData |= NEO_KHZ800;
                }
                else if (strcasecmp(speedStr, INTERFACE_SPEED_KHZ400)) { // check the second one as well to make it easier to add new ones in the future
                    ws2812cfgData |= NEO_KHZ400;
                } // currently there is not any more modes

                uint8_t brightness = JsonSchema::WS2812::brightnessField.ExtractFrom(*(context.jsonObjItem));
                uint8_t mode = JsonSchema::WS2812::modeField.ExtractFrom(*(context.jsonObjItem));
                uint16_t fxSpeed = JsonSchema::WS2812::fxspeedField.ExtractFrom(*(context.jsonObjItem));

                WS2812FX* ws2812fx = new WS2812FX(numLeds, pin, ws2812cfgData);
                out->ws2812fx = ws2812fx; // out is the owner
                ws2812fx->init();
                ws2812fx->setBrightness(brightness);
                ws2812fx->setMode(mode);
                ws2812fx->setSpeed(fxSpeed);
                ws2812fx->start();
            }

        }

    }

}