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

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase WS2812::RegistryDefine = {
        Create,
        &JsonSchema::WS2812::Root,
        DALHAL_REACTIVE_EVENT_TABLE(WS2812),
        &WS2812::FunctionTable
    };
    
    /* override */
    const Registry::DefineBase* WS2812::GetRegistryDefine() {
        return &RegistryDefine;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::Exec_FuncType> WS2812::execFunctions[] = {
        {"pause", &WS2812::exec_pause_Function, "pause fx operation"},
        {"resume", &WS2812::exec_resume_Function, "resume fx operation"},
        {"stop", &WS2812::exec_stop_Function, "stop fx operation"},
        {"restart", &WS2812::exec_start_Function, "restart fx operation"},
    };
    HALOperationResult WS2812::exec_pause_Function(Device* device) {
        static_cast<WS2812*>(device)->ws2812fx->pause();
        return HALOperationResult::Success;
    }
    HALOperationResult WS2812::exec_resume_Function(Device* device) {
        static_cast<WS2812*>(device)->ws2812fx->resume();
        return HALOperationResult::Success;
    }
    HALOperationResult WS2812::exec_stop_Function(Device* device) {
        static_cast<WS2812*>(device)->ws2812fx->stop();
        return HALOperationResult::Success;
    }
    HALOperationResult WS2812::exec_start_Function(Device* device) {
        static_cast<WS2812*>(device)->ws2812fx->start();
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::WriteHALValue_FuncType> WS2812::writeValueFunctions[] = {
        {"brightness", &WS2812::writeBrightness, "sets the brightness"},
        {"color", &WS2812::writeColor, "sets the color for the first pixel"},
        {"mode", &WS2812::writeMode, "sets the mode by index"},
        {"fxspeed", &WS2812::writeFxSpeed, "sets the fx speed (0-65535)"},
    };

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::WriteString_FuncType> WS2812::writeStringFunctions[] = {
        {"setpixel", &WS2812::writeString_setpixel_Function, "help"}
    };
    HALOperationResult WS2812::writeString_setpixel_Function(Device* device, ZeroCopyString zcParams, StringBuilderStreamer& sbs) {
        WS2812& self = *static_cast<WS2812*>(device);
        self.ws2812fx->pause();
        // ws2812fx->setSpeed(0);
        ZeroCopyString zcIndex = zcParams.SplitOffHead('/');
        ZeroCopyString zcR = zcParams.SplitOffHead('/');
        ZeroCopyString zcG = zcParams.SplitOffHead('/');
        ZeroCopyString zcB = zcParams.SplitOffHead('/');
        uint32_t index,r,g,b;
        zcIndex.ConvertTo_uint32(index);
        zcR.ConvertTo_uint32(r);
        zcG.ConvertTo_uint32(g);
        zcB.ConvertTo_uint32(b);
        if (zcParams.IsEmpty()) {// simple rgb
            
            self.ws2812fx->setPixelColor(index,r,g,b);
            //printf("\nsetpixel: index=%d, r=%d, g=%d, b=%d\n",index,r,g,b);
        }
        else {
            uint32_t w;
            ZeroCopyString zcUval = zcParams.SplitOffHead('/');
            zcUval.ConvertTo_uint32(w);
            self.ws2812fx->setPixelColor(index,r,g,b,w);
            //printf("\nsetpixel: index=%d, r=%d, g=%d, b=%d, w=%d\n",index,r,g,b,w);
        }
        self.ws2812fx->execShow();
        return HALOperationResult::Success;
    }

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable WS2812::FunctionTable = {
        {execFunctions, sizeof(execFunctions) / sizeof(execFunctions[0])},
        EmptyFunctionTable<DeviceFunctionTable::ReadToHALValue_FuncType>,
        {writeValueFunctions, sizeof(writeValueFunctions) / sizeof(writeValueFunctions[0])},
        EmptyFunctionTable<DeviceFunctionTable::BracketOpRead_FuncType>,
        EmptyFunctionTable<DeviceFunctionTable::BracketOpWrite_FuncType>,
        EmptyFunctionTable<DeviceFunctionTable::ReadString_FuncType>,
        {writeStringFunctions, sizeof(writeStringFunctions) / sizeof(writeStringFunctions[0])},
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
    /* static */
    HALOperationResult WS2812::writeBrightness(Device* device, const HALValue& val) {
        //printf("\nWS2812::writeBrightness\n");
        WS2812* ws2812_hal_device = static_cast<WS2812*>(device);
        
        ws2812_hal_device->ws2812fx->setBrightness(val.toUInt());

#if HAS_REACTIVE_WRITE(WS2812)
        ws2812_hal_device->triggerWrite();
#endif
        return HALOperationResult::Success;
    }
    /* static */
    HALOperationResult WS2812::writeColor(Device* device, const HALValue& val) {
        //printf("\nWS2812::writeColor\n");
        WS2812* ws2812_hal_device = static_cast<WS2812*>(device);
        ws2812_hal_device->ws2812fx->pause();
        ws2812_hal_device->ws2812fx->setPixelColor(0, val.toUInt());
        ws2812_hal_device->ws2812fx->execShow();

#if HAS_REACTIVE_WRITE(WS2812)
        ws2812_hal_device->triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    /* static */
    HALOperationResult WS2812::writeMode(Device* device, const HALValue& val) {
        WS2812* ws2812_hal_device = static_cast<WS2812*>(device);
        
        ws2812_hal_device->ws2812fx->setMode(val.toUInt());

#if HAS_REACTIVE_WRITE(WS2812)
        ws2812_hal_device->triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    /* static */
    HALOperationResult WS2812::writeFxSpeed(Device* device, const HALValue& val) {
        WS2812* ws2812_hal_device = static_cast<WS2812*>(device);
        
        ws2812_hal_device->ws2812fx->setSpeed(val.toUInt());

#if HAS_REACTIVE_WRITE(WS2812)
        ws2812_hal_device->triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    Device::WriteHALValue_FuncType WS2812::GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) {
        return GetDeviceFunction<DeviceFunctionTable::WriteHALValue_FuncType>(FunctionTable.writeValue, zcFuncName);
    }

    HALOperationResult WS2812::write(const HALWriteValueByCmd& val) {

        DeviceFunctionTable::WriteHALValue_FuncType fn = GetDeviceFunction<DeviceFunctionTable::WriteHALValue_FuncType>(FunctionTable.writeValue, val.cmd);
        if (fn == nullptr) { return HALOperationResult::UnsupportedCommand; }
        return fn(this, val.value);
    }

    HALOperationResult WS2812::write(const HALWriteStringRequestValue& val) {

        DeviceFunctionTable::WriteString_FuncType fn = GetDeviceFunction<DeviceFunctionTable::WriteString_FuncType>(FunctionTable.writeString, val.cmd);
        if (fn == nullptr) { return HALOperationResult::UnsupportedCommand; }
        return fn(this, val.parameters, val.sbs);
    }

    void WS2812::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pin"), ws2812fx->getPin());
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("curr mode"), ws2812fx->getMode());
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("curr brightness"), ws2812fx->getBrightness());
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("curr speed"), ws2812fx->getSpeed());
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