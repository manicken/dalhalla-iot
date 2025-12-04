/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (c) 2025 Jannik Svensson

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/


#include "HAL_JSON_DeviceTypesRegistry.h"

// all HAL devices to use here
#include "Script/HAL_JSON_ScriptVariable.h"
#include "Script/HAL_JSON_ScriptVariableReadOnly.h"
#include "Script/HAL_JSON_ScriptVariableWriteOnlyTest.h"
#include "Script/HAL_JSON_ScriptArray.h"

#include "CoreDevices/HAL_JSON_AnalogInput.h"
#include "CoreDevices/HAL_JSON_DigitalInput.h"
#include "CoreDevices/HAL_JSON_DigitalOutput.h"
#include "CoreDevices/HAL_JSON_PWMAnalogWrite.h"

#include "HAL_JSON_SinglePulseOutput.h"

#include "CoreDevices/HAL_JSON_I2C_BUS.h"

#include "OneWireTemp/HAL_JSON_OneWireTemp.h"
#include "HAL_JSON_DHT.h"
#include "RF433/HAL_JSON_TX433.h"
#include "REGO600/HAL_JSON_REGO600.h"


#include "HAL_JSON_WS2812.h"
#include "HAL_JSON_REST_Value.h"
#include "HAL_JSON_REST_Cmd.h"
#include "HAL_JSON_DeviceContainer.h"

#include "HAL_JSON_ThingSpeak.h"
#include "HomeAssistant/HAL_JSON_HomeAssistant.h"


namespace HAL_JSON {

    constexpr DeviceRegistryDefine RegistryItemNullDefault = {UseRootUID::Void, nullptr, nullptr };
    constexpr DeviceRegistryItem RegistryTerminatorItem = {nullptr, RegistryItemNullDefault};

    constexpr DeviceRegistryItem DeviceRegistry[] = {
        {"VAR", ScriptVariable::RegistryDefine},

        {"DIN", DigitalInput::RegistryDefine},
        {"DOUT", DigitalOutput::RegistryDefine},
        {"DPOUT", SinglePulseOutput::RegistryDefine},
#if defined(ESP32) || defined(_WIN32)
        {"ADC", AnalogInput::RegistryDefine},
#endif
        {"PWM_AW", PWMAnalogWrite::RegistryDefine},
        {"PWM_AW_CFG", PWMAnalogWriteConfig::RegistryDefine},

        {"1WTG", OneWireTempGroup::RegistryDefine},
        {"1WTB", OneWireTempBusAtRoot::RegistryDefine},
        {"1WTD", OneWireTempDeviceAtRoot::RegistryDefine},

        {"DHT", DHT::RegistryDefine},
        {"TX433", TX433::RegistryDefine},
        {"REGO600", REGO600::RegistryDefine},

        {"I2C", I2C_BUS::RegistryDefine},
        
        {"CONSTVAR", ScriptVariableReadOnly::RegistryDefine},
        {"WRITEVAR", ScriptVariableWriteOnlyTest::RegistryDefine},
        {"REST_VAR", REST_Value::RegistryDefine},
        {"REST_CMD", REST_Cmd::RegistryDefine},
        {"CONTAINER", DeviceContainer::RegistryDefine},
        {"ARRAY", ScriptArray::RegistryDefine},

        {"THINGSPEAK", ThingSpeak::RegistryDefine},
        {"HOMEASSISTANT", HomeAssistant::RegistryDefine},
        {"WS2812", WS2812::RegistryDefine},
        /** mandatory null terminator */
        RegistryTerminatorItem, // Stop for active devices

        /** --- Planned / future devices --- */
#if defined(ESP32) || defined(_WIN32)
        {"PWM_LEDC", RegistryItemNullDefault},
#endif
    };

    const DeviceRegistryItem& GetDeviceRegistryItem(const char* type) {
        int i=0;
        while (true) {
            const DeviceRegistryItem& regItem = DeviceRegistry[i++];
            if (regItem.typeName == nullptr) break;
            if (strcasecmp(regItem.typeName, type) == 0) return regItem;
        }
        return RegistryTerminatorItem;
    }
}