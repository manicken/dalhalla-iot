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


#include "DALHAL_HA_DeviceTypeReg.h"

// Available Home Assistant device-endpoint-entities types here
#include "DALHAL_HA_DeviceContainer.h"
#include "Entities/DALHAL_HA_Sensor.h"
#include "Entities/DALHAL_HA_BinarySensor.h"
#include "Entities/DALHAL_HA_Switch.h"
#include "Entities/DALHAL_HA_Button.h"
#include "Entities/DALHAL_HA_Number.h"


namespace DALHAL {

    constexpr HA_DeviceRegistryDefine RegistryItemNullDefault = {nullptr, nullptr };
    constexpr HA_DeviceRegistryItem RegistryTerminatorItem = {nullptr, RegistryItemNullDefault};

    constexpr HA_DeviceRegistryItem HA_DeviceRegistry[] = {
        {"sensor",    Sensor::RegistryDefine},
        {"binary_sensor",  BinarySensor::RegistryDefine},
        {"switch",    Switch::RegistryDefine},
        {"button",    Button::RegistryDefine},
        {"number",    Number::RegistryDefine},
        {"CONTAINER", HA_DeviceContainer::RegistryDefine},
        /** mandatory null terminator */
        RegistryTerminatorItem
    };
    const HA_DeviceRegistryItem& Get_HA_DeviceRegistryItem(const char* type) {
        int i=0;
        while (true) {
            const HA_DeviceRegistryItem& regItem = HA_DeviceRegistry[i++];
            if (regItem.typeName == nullptr) break;
            if (strcasecmp(regItem.typeName, type) == 0) return regItem;
        }
        return RegistryTerminatorItem;
    }
}