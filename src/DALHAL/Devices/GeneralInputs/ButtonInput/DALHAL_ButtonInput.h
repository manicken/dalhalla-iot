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

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <DALHAL/Core/Types/DALHAL_Registry.h>
#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Device/DALHAL_CachedDeviceAccess.h>

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(BUTTON_INPUT)
#include "DALHAL_ButtonInput_Reactive.h"
using ButtonInput_DeviceBase = DALHAL::ButtonInput_Reactive;
#else
using ButtonInput_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    namespace JsonSchema { namespace ButtonInput { struct Extractors; } } // forward declaration

    class ButtonInput : public ButtonInput_DeviceBase {
        friend struct JsonSchema::ButtonInput::Extractors; // allow access to private memebers of this class from the schema extractor

    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static Device* Create(DeviceCreateContext& context);

    private:
        uint8_t pin;
        uint32_t debounceMs;
        uint8_t activeLevel;

        bool stableState;       // debounced button state
        bool lastRaw;           // last raw read
        uint32_t lastChangeMs;  // last change timestamp

        CachedDeviceAccess* toggleTarget; // optional external action

    public:
        ButtonInput(DeviceCreateContext& context);
        ~ButtonInput() override;

        HALOperationResult read(HALValue &val) override;
        String ToString() override;

        void loop();

    };

} // namespace DALHAL
