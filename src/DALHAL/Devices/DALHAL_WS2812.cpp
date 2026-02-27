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

#include "DALHAL_WS2812.h"

#include "../Support/DALHAL_Logger.h"
#include "../Core/Device/DALHAL_JSON_Config_Defines.h"
#include "../Support/DALHAL_ArduinoJSON_ext.h"
#include "../Core/Manager/DALHAL_GPIO_Manager.h"

namespace DALHAL {
    
    WS2812::WS2812(const JsonVariant &jsonObj, const char* type) : Device(type) {
        const char* uidStr = GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID);
        uid = encodeUID(uidStr);
        pin = GetAsUINT8(jsonObj, "pin");

        int numLeds = GetAsUINT32(jsonObj, "ledcount");
        if (numLeds == 0) numLeds = 1; // defaults to only one led
        neoPixelType typeDef = 0; // basically a uint16_t
        const char* led_c = GetAsConstChar(jsonObj, "format");
        
        if (led_c == nullptr || strcasecmp(led_c, "RGB") == 0) typeDef += NEO_RGB;
        else if (strcasecmp(led_c, "RBG") == 0) typeDef += NEO_RBG;
        else if (strcasecmp(led_c, "GRB") == 0) typeDef += NEO_GRB;
        else if (strcasecmp(led_c, "GBR") == 0) typeDef += NEO_GBR;
        else if (strcasecmp(led_c, "BRG") == 0) typeDef += NEO_BRG;
        else if (strcasecmp(led_c, "BGR") == 0) typeDef += NEO_BGR;

        else if (strcasecmp(led_c, "WRGB") == 0) typeDef += NEO_WRGB;
        else if (strcasecmp(led_c, "WRBG") == 0) typeDef += NEO_WRBG;
        else if (strcasecmp(led_c, "WGRB") == 0) typeDef += NEO_WGRB;
        else if (strcasecmp(led_c, "WGBR") == 0) typeDef += NEO_WGBR;
        else if (strcasecmp(led_c, "WBRG") == 0) typeDef += NEO_WBRG;
        else if (strcasecmp(led_c, "WBGR") == 0) typeDef += NEO_WBGR;

        else if (strcasecmp(led_c, "RWGB") == 0) typeDef += NEO_RWGB;
        else if (strcasecmp(led_c, "RWBG") == 0) typeDef += NEO_RWBG;
        else if (strcasecmp(led_c, "GWRB") == 0) typeDef += NEO_GWRB;
        else if (strcasecmp(led_c, "GWBR") == 0) typeDef += NEO_GWBR;
        else if (strcasecmp(led_c, "BWRG") == 0) typeDef += NEO_BWRG;
        else if (strcasecmp(led_c, "BWGR") == 0) typeDef += NEO_BWGR;

        else if (strcasecmp(led_c, "RGWB") == 0) typeDef += NEO_RGWB;
        else if (strcasecmp(led_c, "RBWG") == 0) typeDef += NEO_RBWG;
        else if (strcasecmp(led_c, "GRWB") == 0) typeDef += NEO_GRWB;
        else if (strcasecmp(led_c, "GBWR") == 0) typeDef += NEO_GBWR;
        else if (strcasecmp(led_c, "BRWG") == 0) typeDef += NEO_BRWG;
        else if (strcasecmp(led_c, "BGWR") == 0) typeDef += NEO_BGWR;

        else if (strcasecmp(led_c, "RGBW") == 0) typeDef += NEO_RGBW;
        else if (strcasecmp(led_c, "RBGW") == 0) typeDef += NEO_RBGW;
        else if (strcasecmp(led_c, "GRBW") == 0) typeDef += NEO_GRBW;
        else if (strcasecmp(led_c, "GBRW") == 0) typeDef += NEO_GBRW;
        else if (strcasecmp(led_c, "BRGW") == 0) typeDef += NEO_BRGW;
        else if (strcasecmp(led_c, "BGRW") == 0) typeDef += NEO_BGRW;

        const char* speedStr = GetAsConstChar(jsonObj, "ifspeed");
        if (speedStr && strcasecmp(speedStr, "KHZ800"))
            typeDef += NEO_KHZ800;
        else
            typeDef += NEO_KHZ400;

        uint8_t brightness = GetAsUINT8(jsonObj, "brightness");
        if (brightness == 0) brightness = 127; // default
        uint8_t mode = GetAsUINT8(jsonObj, "mode");
        uint16_t fxSpeed = GetAsUINT16(jsonObj, "fxspeed");
        if (fxSpeed < 2) fxSpeed = 3000;

        ws2812fx = new WS2812FX(numLeds, pin, typeDef);
        ws2812fx->init();
        ws2812fx->setBrightness(brightness);
        ws2812fx->setMode(mode);
        ws2812fx->setSpeed(fxSpeed);
        ws2812fx->start();
    }
    HALOperationResult WS2812::writeBrightness(Device* context, const HALValue& val) {
        //printf("\nWS2812::writeBrightness\n");
        WS2812* ws2812fx = static_cast<WS2812*>(context);
        
        ws2812fx->ws2812fx->setBrightness(val.asUInt());
        
        return HALOperationResult::Success;
    }
    HALOperationResult WS2812::writeColor(Device* context, const HALValue& val) {
        //printf("\nWS2812::writeColor\n");
        WS2812* ws2812fx = static_cast<WS2812*>(context);
        ws2812fx->ws2812fx->pause();
        ws2812fx->ws2812fx->setPixelColor(0,val.asUInt());
        ws2812fx->ws2812fx->execShow();
        return HALOperationResult::Success;
    }

    Device::WriteHALValue_FuncType WS2812::GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) {
        if (zcFuncName == "brightness")
            return WS2812::writeBrightness;
        else if (zcFuncName == "color")
            return WS2812::writeColor;
        else
            return nullptr;
    }

    HALOperationResult WS2812::write(const HALWriteValueByCmd& val) {
        if (val.cmd == "brightness")
            ws2812fx->setBrightness(val.value.asUInt());
        else if (val.cmd == "mode")
            ws2812fx->setMode(val.value.asUInt());
        else if (val.cmd == "fxspeed")
            ws2812fx->setSpeed(val.value.asUInt());
        else if (val.cmd == "color")
            WS2812::writeColor(this, val.value);
        else
            return HALOperationResult::UnsupportedCommand;

        return HALOperationResult::Success;
    }

    HALOperationResult WS2812::write(const HALWriteStringRequestValue& val) {
        ZeroCopyString zcStr = val.value; // copy first
        ZeroCopyString cmd = zcStr.SplitOffHead('/');
        if (cmd == "brightness") {
            uint32_t val;
            zcStr.ConvertTo_uint32(val);
            ws2812fx->setBrightness(val);
        }
        else if (cmd == "mode") {
            uint32_t val;
            zcStr.ConvertTo_uint32(val);
            ws2812fx->setMode(val);
            ws2812fx->resume();
        }
        else if (cmd == "fxspeed") {
            uint32_t val;
            zcStr.ConvertTo_uint32(val);
            ws2812fx->setSpeed(val);
        }
        else if (cmd == "setpixel") {
            ws2812fx->pause();
           // ws2812fx->setSpeed(0);
            ZeroCopyString zcIndex = zcStr.SplitOffHead('/');
            ZeroCopyString zcR = zcStr.SplitOffHead('/');
            ZeroCopyString zcG = zcStr.SplitOffHead('/');
            ZeroCopyString zcB = zcStr.SplitOffHead('/');
            uint32_t index,r,g,b;
            zcIndex.ConvertTo_uint32(index);
            zcR.ConvertTo_uint32(r);
            zcG.ConvertTo_uint32(g);
            zcB.ConvertTo_uint32(b);
            if (zcStr.IsEmpty()) {// simple rgb
                
                ws2812fx->setPixelColor(index,r,g,b);
                //printf("\nsetpixel: index=%d, r=%d, g=%d, b=%d\n",index,r,g,b);
            }
            else {
                uint32_t w;
                zcStr.ConvertTo_uint32(w);
                ws2812fx->setPixelColor(index,r,g,b,w);
                //printf("\nsetpixel: index=%d, r=%d, g=%d, b=%d, w=%d\n",index,r,g,b,w);
            }
            ws2812fx->execShow();
        }
        else if (cmd == "color") {
            uint32_t halval = 0;
            zcStr.ConvertTo_uint32(halval);
            const HALValue cVal = halval;
            WS2812::writeColor(this, cVal);
        }
        else if (cmd == "pause") {
            ws2812fx->pause();
        }
        else if (cmd == "resume") {
            ws2812fx->resume();
        }
        else if (cmd == "stop") {
            ws2812fx->stop();
        }
        else if (cmd == "start") {
            ws2812fx->start();
        }
        else
            return HALOperationResult::UnsupportedCommand;
        return HALOperationResult::Success;
    }

    void WS2812::loop() {
        //if (ws2812fx->getMode() != 0) // only service when non static mode
            ws2812fx->service();
    }

    bool WS2812::VerifyJSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj, DALHAL_KEYNAME_UID)){ 
            SET_ERR_LOC(DALHAL_ERROR_SOURCE_WS2812_VERIFY_JSON);
            return false;
        }
        if (false == GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinFunc::OUT) | static_cast<uint8_t>(GPIO_manager::PinFunc::IN)))) {
            SET_ERR_LOC(DALHAL_ERROR_SOURCE_WS2812_VERIFY_JSON);
            return false;
        }
        
        return true;
    }

    Device* WS2812::Create(const JsonVariant &jsonObj, const char* type) {
        return new WS2812(jsonObj, type);
    }

    String WS2812::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += ",\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        return ret;
    }

    uint8_t neoCodeFromString(const char* str) {
        if (!str) return NEO_RBG; // default to NEO_RGB if nullptr

        uint8_t code = 0;
        int idx[4] = {0xFF, 0xFF, 0xFF, 0xFF};

        auto charToIdx = [](char c) -> int {
            switch (std::toupper(c)) {
                case 'R': return 0;
                case 'G': return 1;
                case 'B': return 2;
                case 'W': return 3;
            }
            return 0xFF; // invalid
        };

        int i = 0;
        while (str[i] && i < 4) {
            idx[i] = charToIdx(str[i]);
            i++;
        }

        if (i >= 3) {
            code |= (idx[0] & 0x03) << 6; // first channel
            code |= (idx[0] & 0x03) << 4; // repeated (for RGBW)
            code |= (idx[1] & 0x03) << 2; // second channel
            code |= (idx[2] & 0x03) << 0; // third channel
        }

        if (i == 4) {
            // overwrite bits 5–4 for W channel
            code &= ~(0x30);            // clear bits 5–4
            code |= (idx[3] & 0x03) << 4;
        }

        return code;
    }
}