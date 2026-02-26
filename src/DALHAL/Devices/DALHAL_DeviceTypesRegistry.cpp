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


#include "DALHAL_DeviceTypesRegistry.h"

// all HAL devices to use here
#include "Script/DALHAL_ScriptVariable.h"
#include "Script/DALHAL_ScriptVariableReadOnly.h"
#include "Script/DALHAL_ScriptVariableWriteOnlyTest.h"
#include "Script/DALHAL_ScriptArray.h"

#include "CoreDevices/DALHAL_AnalogInput.h"
#include "CoreDevices/DALHAL_DigitalInput.h"
#include "CoreDevices/DALHAL_DigitalOutput.h"
#include "CoreDevices/DALHAL_PWMAnalogWrite.h"

#include "DALHAL_SinglePulseOutput.h"

#include "CoreDevices/DALHAL_I2C_BUS.h"

#include "OneWireTemp/DALHAL_OneWireTemp.h"
#include "DALHAL_DHT.h"
#include "RF433/DALHAL_TX433.h"
#include "REGO600/DALHAL_REGO600.h"


#include "DALHAL_WS2812.h"
#include "DALHAL_REST_Value.h"
#include "DALHAL_REST_Cmd.h"
#include "DALHAL_DeviceContainer.h"

#include "DALHAL_ThingSpeak.h"
#include "HomeAssistant/DALHAL_HomeAssistant.h"

#include "DALHAL_Actuator.h"
#include "DALHAL_LEDC_Servo.h"
#include "DALHAL_ButtonInput.h"
#include "DALHAL_LatchingRelay.h"


namespace DALHAL {

    constexpr DeviceRegistryDefine RegistryItemNullDefault = {UseRootUID::Void, nullptr, nullptr };
    constexpr DeviceRegistryItem RegistryTerminatorItem = {nullptr, RegistryItemNullDefault};

    constexpr DeviceRegistryItem DeviceRegistry[] = {

         /** ---------------- Script-backed Devices ---------------- */
        {"VAR", ScriptVariable::RegistryDefine},
        {"CONSTVAR", ScriptVariableReadOnly::RegistryDefine},
        {"WRITEVAR", ScriptVariableWriteOnlyTest::RegistryDefine}, // only a development test device, that is used when testing write only functionality
        {"ARRAY", ScriptArray::RegistryDefine},

        /** ---------------- Core Devices ---------------- */
        {"DIN", DigitalInput::RegistryDefine},
        {"DOUT", DigitalOutput::RegistryDefine},
        {"DPOUT", SinglePulseOutput::RegistryDefine},
#if defined(ESP32) || defined(_WIN32)
        {"ADC", AnalogInput::RegistryDefine},
#endif
        {"PWM_AW", PWMAnalogWrite::RegistryDefine},
        {"PWM_AW_CFG", PWMAnalogWriteConfig::RegistryDefine},

        /** ---------------- OneWire / Sensors ---------------- */
#if defined(DALHAL_DEVICE_ONEWIRE_TEMPERATURE_SENSORS)
        {"1WTG", OneWireTempGroup::RegistryDefine},
        {"1WTB", OneWireTempBusAtRoot::RegistryDefine},
        {"1WTD", OneWireTempDeviceAtRoot::RegistryDefine},
#endif
        /** ---------------- Other / Sensors ---------------- */
        {"DHT", DHT::RegistryDefine},

        /** ---------------- RF / Communication ---------------- */
        {"TX433", TX433::RegistryDefine},
#if defined(DALHAL_DEVICE_REGO600_HEATPUMP_CONTROLLER)
        {"REGO600", REGO600::RegistryDefine},
#endif
        {"I2C", I2C_BUS::RegistryDefine},

        /** ---------------- Lights ---------------- */
        {"WS2812", WS2812::RegistryDefine},
        
        /** -------------- Virtual Devices --------- */
        //{"REST_VAR", REST_Value::RegistryDefine}, // obsolete as the REST API cannot handle long running requests, and is extremely unstable
        //{"REST_CMD", REST_Cmd::RegistryDefine}, // obsolete as the REST API cannot handle long running requests, and is extremely unstable

        /** ---------------- Device Containers ---------------- */
        {"CONTAINER", DeviceContainer::RegistryDefine},
        
        /** ---------------- External / Cloud / API Devices ---------------- */
        {"THINGSPEAK", ThingSpeak::RegistryDefine},
        {"HOMEASSISTANT", HomeAssistant::RegistryDefine},
        
        /** ---------------- Actuators / Outputs ---------------- */
        {"ACTUATOR", Actuator::RegistryDefine},
        {"RELAY_LATCHING", LatchingRelay::RegistryDefine},
#if defined(ESP32) || defined(_WIN32)
        {"LEDC_SERVO", LEDC_Servo::RegistryDefine},
#endif

        /** ---------------- Inputs ---------------- */
        {"BUTTON", ButtonInput::RegistryDefine},

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