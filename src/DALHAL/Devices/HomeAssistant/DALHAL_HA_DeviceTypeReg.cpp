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


#include "DALHAL_HA_DeviceTypeReg.h"

#include <DALHAL/Core/Types/DALHAL_Registry.h>

// Available Home Assistant device-endpoint-entities types here
#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_DeviceContainer.h>

#include <DALHAL/Devices/HomeAssistant/Entities/DALHAL_HA_Sensor.h>
#include <DALHAL/Devices/HomeAssistant/Entities/DALHAL_HA_BinarySensor.h>
#include <DALHAL/Devices/HomeAssistant/Entities/DALHAL_HA_Switch.h>
#include <DALHAL/Devices/HomeAssistant/Entities/DALHAL_HA_Button.h>
#include <DALHAL/Devices/HomeAssistant/Entities/DALHAL_HA_Number.h>


namespace DALHAL {

    //constexpr HA_Registry::DefineBase RegistryItemNullDefault = {nullptr, nullptr };
    //constexpr HA_DeviceRegistryItem RegistryTerminatorItem = {nullptr, RegistryItemNullDefault};

    constexpr Registry::Item HA_DeviceRegistry[] = {
        {"sensor",         &Sensor::RegistryDefine},
        {"binary_sensor",  &BinarySensor::RegistryDefine},
        {"switch",         &Switch::RegistryDefine},
        {"button",         &Button::RegistryDefine},
        {"number",         &Number::RegistryDefine},
        {"CONTAINER",      &HA_DeviceContainer::RegistryDefine},
        /** mandatory null terminator */
        Registry::TerminatorItem
    };
    
}