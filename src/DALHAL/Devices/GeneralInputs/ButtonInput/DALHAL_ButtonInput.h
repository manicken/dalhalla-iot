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

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <DALHAL/Devices/_Registry/DALHAL_DevicesRegistry.h>
#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Device/DALHAL_CachedDeviceAccess.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>


#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(BUTTON_INPUT)
#include "DALHAL_ButtonInput_Reactive.h"
using ButtonInput_DeviceBase = DALHAL::ButtonInput_Reactive;
#else
using ButtonInput_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

class ButtonInput : public ButtonInput_DeviceBase {
public:
    // Factory and JSON validation
    static Device* Create(const JsonVariant &jsonObj, const char* type);
    static bool VerifyJSON(const JsonVariant &jsonObj);
    static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            Create,
            VerifyJSON,
            DALHAL_REACTIVE_EVENT_TABLE(BUTTON_INPUT)
        };

    // Constructor
    ButtonInput(const JsonVariant &jsonObj, const char* type);
    ~ButtonInput();

    // DAL interface
    HALOperationResult read(HALValue &val) override;
    String ToString() override;

    // Call in main loop for debounce + toggle
    void loop();

private:
    uint8_t pin;
    uint32_t debounceMs;
    bool activeLow;

    bool stableState;       // debounced button state
    bool lastRaw;           // last raw read
    uint32_t lastChangeMs;  // last change timestamp

    //bool toggleState;       // TEMP: internal toggle variable
    CachedDeviceAccess* toggleTarget; // optional external action
};

} // namespace DALHAL
