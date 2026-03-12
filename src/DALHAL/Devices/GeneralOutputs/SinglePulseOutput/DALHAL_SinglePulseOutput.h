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

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include <Ticker.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(SINGLE_PULSE_OUTPUT)
#include "DALHAL_SinglePulseOutput_Reactive.h"
using SinglePulseOutput_DeviceBase = DALHAL::SinglePulseOutput_Reactive;
#else
using SinglePulseOutput_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    class SinglePulseOutput : public SinglePulseOutput_DeviceBase {
    private:
        uint8_t pin = 0;
        uint32_t pulseLength = 0;
        uint8_t inactiveState = 0;
        Ticker pulseTicker;
        void endPulse();
        static void pulseTicker_Callback(SinglePulseOutput* context);
    public:
        static bool VerifyJSON(const JsonVariant& jsonObj);
        static Device* Create(const JsonVariant& jsonObj, const char* type, void* context);
        static constexpr Registry::Define RegistryDefine = {
            Registry::UseRootUID::Mandatory,
            Create,
            VerifyJSON,
            DALHAL_REACTIVE_EVENT_TABLE(SINGLE_PULSE_OUTPUT)
        };
        SinglePulseOutput(const JsonVariant& jsonObj, const char* type);
        ~SinglePulseOutput();

        HALOperationResult read(HALValue& val) override;
        HALOperationResult write(const HALValue& val) override;
        HALOperationResult exec() override;
        Exec_FuncType GetExec_Function(ZeroCopyString& zcFuncName) override;
        static HALOperationResult exec(Device* device);
        String ToString() override;
    };

}