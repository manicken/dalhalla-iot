/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2026 Jannik Svensson

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


#include "DALHAL_DevicesRegistry.h"

// all HAL devices to use here
#include <DALHAL/Devices/Script/ScriptVariable/DALHAL_ScriptVariable.h>
#include <DALHAL/Devices/Script/ScriptVariableReadOnly/DALHAL_ScriptVariableReadOnly.h>
#include <DALHAL/Devices/Script/ScriptVariableWriteOnlyTest/DALHAL_ScriptVariableWriteOnlyTest.h>
#include <DALHAL/Devices/Script/ScriptArray/DALHAL_ScriptArray.h>

#include <DALHAL/Devices/GeneralInputs/ButtonInput/DALHAL_ButtonInput.h>
#include <DALHAL/Devices/GeneralInputs/AnalogInput/DALHAL_AnalogInput.h>
#include <DALHAL/Devices/GeneralInputs/DigitalInput/DALHAL_DigitalInput.h>

#include <DALHAL/Devices/GeneralOutputs/DigitalOutput/DALHAL_DigitalOutput.h>
//#include <DALHAL/Devices/GeneralOutputs/PWMAnalogWrite/DALHAL_PWMAnalogWrite.h> OBSOLETE TO BE REMOVED
#include <DALHAL/Devices/GeneralOutputs/SinglePulseOutput/DALHAL_SinglePulseOutput.h>

#include <DALHAL/Devices/I2C_Master/DALHAL_I2C_Master.h>

#include <DALHAL/Devices/Sensors/OneWireTemp/DALHAL_OneWireTemp.h>
#include <DALHAL/Devices/Sensors/DHT/DALHAL_DHT.h>



#include <DALHAL/Devices/RF433/DALHAL_TX433.h>
#include <DALHAL/Devices/REGO600/DALHAL_REGO600.h>


#include <DALHAL/Devices/Lights/WS2812/DALHAL_WS2812.h>

#include <DALHAL/Devices/DeviceContainer/DALHAL_DeviceContainer.h>

#include <DALHAL/Devices/DataStorageServices/ThingSpeak/DALHAL_ThingSpeak.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HomeAssistant.h>

#include <DALHAL/Devices/Actuators/Actuator/DALHAL_Actuator.h>
#include <DALHAL/Devices/Actuators/PWM_Servo/DALHAL_PWM_Servo.h>
#include <DALHAL/Devices/Actuators/LatchingRelay/DALHAL_LatchingRelay.h>

#include <DALHAL/Devices/API/REST_Value/DALHAL_REST_Value.h>
#include <DALHAL/Devices/API/REST_Cmd/DALHAL_REST_Cmd.h>

// this is only a template device and should only be enabled(uncommented)
// when major changes are done to the system, 
// to make sure that the template is updated as well
#include <DALHAL/Devices/Template/DALHAL__Template_.h> 

namespace DALHAL {

    constexpr Registry::Item DeviceRegistry[] = {

         /** ---------------- Script-backed Devices ---------------- */
        {"VAR", &ScriptVariable::RegistryDefine},
        {"CONSTVAR", &ScriptVariableReadOnly::RegistryDefine},
        {"WRITEVAR", &ScriptVariableWriteOnlyTest::RegistryDefine}, // only a development test device, that is used when testing write only functionality
        {"ARRAY", &ScriptArray::RegistryDefine},

        /** ---------------- Core Devices ---------------- */
        {"DIN", &DigitalInput::RegistryDefine},
        {"DOUT", &DigitalOutput::RegistryDefine},
        {"DPOUT", &SinglePulseOutput::RegistryDefine},
#if defined(ESP32) || defined(_WIN32)
        {"ADC", &AnalogInput::RegistryDefine},
#endif
        //{"PWM_AW", PWMAnalogWrite::RegistryDefine}, OBSOLETE TO BE REMOVED
        //{"PWM_AW_CFG", PWMAnalogWriteConfig::RegistryDefine}, OBSOLETE TO BE REMOVED

        /** ---------------- OneWire / Sensors ---------------- */
#if defined(DALHAL_DEVICE_ONEWIRE_TEMPERATURE_SENSORS)
        {"1WTG", &OneWireTempGroup::RegistryDefine},
        {"1WTB", &OneWireTempBusAtRoot::RegistryDefine},
        {"1WTD", &OneWireTempDeviceAtRoot::RegistryDefine},
#endif
        /** ---------------- Other / Sensors ---------------- */
        {"DHT", &DHT::RegistryDefine},

        /** ---------------- RF / Communication ---------------- */
        {"TX433", &TX433::RegistryDefine},
#if defined(DALHAL_DEVICE_REGO600_HEATPUMP_CONTROLLER)
        {"REGO600", &REGO600::RegistryDefine},
#endif
        {"I2C", &I2C_Master::RegistryDefine},

        /** ---------------- Lights ---------------- */
        {"WS2812", &WS2812::RegistryDefine},
        
        /** -------------- Virtual Devices --------- */
        //{"REST_VAR", REST_Value::RegistryDefine}, // obsolete as the REST API cannot handle long running requests, and is extremely unstable
        //{"REST_CMD", REST_Cmd::RegistryDefine}, // obsolete as the REST API cannot handle long running requests, and is extremely unstable

        /** ---------------- Device Containers ---------------- */
        {"CONTAINER", &DeviceContainer::RegistryDefine},
        
        /** ---------------- External / Cloud / API Devices ---------------- */
        {"THINGSPEAK", &ThingSpeak::RegistryDefine},
        {"HOMEASSISTANT", &HomeAssistant::RegistryDefine},
        
        /** ---------------- Actuators / Outputs ---------------- */
        {"ACTUATOR", &Actuator::RegistryDefine},
        {"RELAY_LATCHING", &LatchingRelay::RegistryDefine},
#if defined(ESP32) || defined(_WIN32)
        {"PWM_SERVO", &PWM_Servo::RegistryDefine}, // was LEDC_SERVO
#endif

        /** ---------------- Inputs ---------------- */
        {"BUTTON", &ButtonInput::RegistryDefine},

        /** template device */
#ifdef DALHAL_DEVICE__TEMPLATE_
        {"TEMPLATE", &_Template_::RegistryDefine},
#endif
        /** mandatory null terminator */
        Registry::TerminatorItem, // Stop for active devices
        

        /** --- Planned / future devices --- */
#if defined(ESP32) || defined(_WIN32)
        {"PWM_LEDC", nullptr},
#endif
    };

}