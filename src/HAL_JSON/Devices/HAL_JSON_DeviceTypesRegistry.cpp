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
#include "CoreDevices/HAL_JSON_SinglePulseOutput.h"

#include "CoreDevices/HAL_JSON_I2C_BUS.h"

#include "OneWireTemp/HAL_JSON_OneWireTemp.h"
#include "HAL_JSON_DHT.h"
#include "RF433/HAL_JSON_TX433.h"
#include "REGO600/HAL_JSON_REGO600.h"

#include "HAL_JSON_ThingSpeak.h"
#include "HAL_JSON_WS2812.h"
#include "HAL_JSON_REST_Value.h"
#include "HAL_JSON_REST_Cmd.h"
#include "HAL_JSON_DeviceContainer.h"


namespace HAL_JSON {

    const DeviceTypeDef DeviceRegistry[] = {
        {UseRootUID::Mandatory, "VAR", ScriptVariable::Create, ScriptVariable::VerifyJSON},


        {UseRootUID::Mandatory, "DIN", DigitalInput::Create, DigitalInput::VerifyJSON},
        {UseRootUID::Mandatory, "DOUT", DigitalOutput::Create, DigitalOutput::VerifyJSON},
        {UseRootUID::Mandatory, "DPOUT", SinglePulseOutput::Create, SinglePulseOutput::VerifyJSON},
#if defined(ESP32) || defined(_WIN32)
        {UseRootUID::Mandatory, "ADC", AnalogInput::Create, AnalogInput::VerifyJSON},
#endif
        {UseRootUID::Optional, "PWM_AW", PWMAnalogWrite::Create, PWMAnalogWrite::VerifyJSON},
        {UseRootUID::Mandatory, "PWM_AW_CFG", PWMAnalogWriteConfig::Create, PWMAnalogWriteConfig::VerifyJSON},

        {UseRootUID::Optional, "1WTG", OneWireTempGroup::Create, OneWireTempGroup::VerifyJSON},
        {UseRootUID::Optional, "1WTB", OneWireTempBusAtRoot::Create, OneWireTempBus::VerifyJSON},
        {UseRootUID::Mandatory, "1WTD", OneWireTempDeviceAtRoot::Create, OneWireTempDeviceAtRoot::VerifyJSON},

        {UseRootUID::Mandatory, "DHT", DHT::Create, DHT::VerifyJSON},
        {UseRootUID::Mandatory, "TX433", TX433::Create, TX433::VerifyJSON},
        {UseRootUID::Mandatory, "REGO600", REGO600::Create, REGO600::VerifyJSON},
#if defined(ESP32) || defined(_WIN32)
        {UseRootUID::Mandatory, "PWM_LEDC", nullptr, nullptr},
#endif
        {UseRootUID::Mandatory, "I2C", I2C_BUS::Create, I2C_BUS::VerifyJSON},
        {UseRootUID::Mandatory, "THINGSPEAK", ThingSpeak::Create, ThingSpeak::VerifyJSON},
        {UseRootUID::Mandatory, "WS2812", WS2812::Create, WS2812::VerifyJSON},
        {UseRootUID::Mandatory, "CONSTVAR", ScriptVariableReadOnly::Create, ScriptVariableReadOnly::VerifyJSON},
        {UseRootUID::Mandatory, "WRITEVAR", ScriptVariableWriteOnlyTest::Create, ScriptVariableWriteOnlyTest::VerifyJSON},
        {UseRootUID::Mandatory, "REST_VAR", REST_Value::Create, REST_Value::VerifyJSON},
        {UseRootUID::Mandatory, "REST_CMD", REST_Cmd::Create, REST_Cmd::VerifyJSON},
        {UseRootUID::Mandatory, "CONTAINER", DeviceContainer::Create, DeviceContainer::VerifyJSON},
        {UseRootUID::Mandatory, "ARRAY", ScriptArray::Create, ScriptArray::VerifyJSON},
        /** mandatory null terminator */
        {UseRootUID::Void, nullptr, nullptr, nullptr} // terminator
    };

    const DeviceTypeDef* GetDeviceTypeDef(const char* type) {
        int i=0;
        while (true) {
            const DeviceTypeDef& def = DeviceRegistry[i++];
            if (def.typeName == nullptr) break;
            if (strcasecmp(def.typeName, type) == 0) return &def;
        }
        return nullptr;
    }
}