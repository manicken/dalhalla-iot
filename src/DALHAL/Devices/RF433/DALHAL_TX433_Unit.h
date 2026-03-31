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


#include <Arduino.h> // Needed for String class

#include <ArduinoJson.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Drivers/RF433.h>

#define DALHAL_KEYNAME_TX433_MODEL "model"

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(TX433_UNIT)
#include "DALHAL_TX433_Unit_Reactive.h"
using TX433unit_DeviceBase = DALHAL::TX433unit_Reactive;
#else
using TX433unit_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    struct TX433_Unit_CreateFunctionContext : DeviceCreateContext {
        const uint32_t pin;
        TX433_Unit_CreateFunctionContext(const uint32_t pin) : DeviceCreateContext(), pin(pin) {}
    };

    enum class TX433_MODEL {
        FixedCode,
        LearningCode
    };

    class TX433_Unit : public TX433unit_DeviceBase {
    public: // public static fields and exposed external structures
        static const Registry::DefineBase LCTypeRegistryDefine; // learning code
        static const Registry::DefineBase SFCTypeRegistryDefine; // simple fixed code
        static const Registry::DefineBase AFCTypeRegistryDefine; // advanced fixed code
        static Device* Create(DeviceCreateContext& context);

    public: // public static fields and exposed external structures
        //static bool VerifyJSON(const JsonVariant &jsonObj);

    private:
        //static bool VerifyFC_JSON(const JsonVariant &jsonObj);
        //static bool VerifyLC_JSON(const JsonVariant &jsonObj);

        /** this is set from root TX433 device and used when sending */
        const uint32_t pin;
        /** defines which type to send state to while using the standard write function */
        TX433_MODEL model;
        /** the part of the data that is fixed */
        uint32_t staticData;
        /** set to false when unused i.e. when the write function sets the state 
         * otherwise it will use the state that included into staticData while using the write function
        */
        bool fixedState=false;

    public:
        TX433_Unit(TX433_Unit_CreateFunctionContext& context);
        ~TX433_Unit() override = default;

        HALOperationResult write(const HALValue &val) override;

        String ToString() override;
    };
}