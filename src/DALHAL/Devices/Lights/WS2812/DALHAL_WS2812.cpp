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

#include "DALHAL_WS2812.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include "DALHAL_WS2812_JSON_Schema.h"

namespace DALHAL {

    constexpr Registry::DefineBase WS2812::RegistryDefine = {
        Create,
        &JsonSchema::WS2812::Root,
        DALHAL_REACTIVE_EVENT_TABLE(WS2812)
    };

    Device* WS2812::Create(DeviceCreateContext& context) {
        return new WS2812(context);
    }
    
    WS2812::WS2812(DeviceCreateContext& context) : WS2812_DeviceBase(context.deviceType) {
        JsonSchema::WS2812::Extractors::Apply(context, this);
    }

    void WS2812::loop() {
        //if (ws2812fx->getMode() != 0) // only service when non static mode
        ws2812fx->service();
    }

    HALOperationResult WS2812::writeBrightness(Device* context, const HALValue& val) {
        //printf("\nWS2812::writeBrightness\n");
        WS2812* ws2812_hal_device = static_cast<WS2812*>(context);
        
        ws2812_hal_device->ws2812fx->setBrightness(val.asUInt());
#if HAS_REACTIVE_WRITE(WS2812)
        ws2812_hal_device->triggerWrite();
#endif
        return HALOperationResult::Success;
    }
    HALOperationResult WS2812::writeColor(Device* context, const HALValue& val) {
        //printf("\nWS2812::writeColor\n");
        WS2812* ws2812_hal_device = static_cast<WS2812*>(context);
        ws2812_hal_device->ws2812fx->pause();
        ws2812_hal_device->ws2812fx->setPixelColor(0,val.asUInt());
        ws2812_hal_device->ws2812fx->execShow();
#if HAS_REACTIVE_WRITE(WS2812)
        ws2812_hal_device->triggerWrite();
#endif
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
#if HAS_REACTIVE_WRITE(WS2812)
        triggerWrite();
#endif
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
#if HAS_REACTIVE_WRITE(WS2812)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    String WS2812::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += ",\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(ws2812fx->getPin()).c_str();
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