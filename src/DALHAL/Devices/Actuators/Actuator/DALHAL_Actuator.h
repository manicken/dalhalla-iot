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

#include <DALHAL/Drivers/DALHAL_GPIO.h>

#include <Arduino.h> // Needed for String class

#include <string>

#include <DALHAL/Core/Types/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>
#include <DALHAL/Core/Types/DALHAL_DeviceFunctionTable.h>

#include <DALHAL/Core/Reactive/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(ACTUATOR)
#include "DALHAL_Actuator_Reactive.h"
using Actuator_DeviceBase = DALHAL::Actuator_Reactive;
#else
using Actuator_DeviceBase = DALHAL::Device;
#endif

#if !defined(ESP8266) && !defined(ESP32)
#define IRAM_ATTR
#include <gpio_types.h>  // PC stub
#endif

namespace DALHAL {
    namespace JsonSchema { namespace Actuator { struct Extractors; } } // forward declaration

    class Actuator : public Actuator_DeviceBase {
        friend struct JsonSchema::Actuator::Extractors; // allow access to private memebers of this class from the schema extractor

    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static Device* Create(DeviceCreateContext& context);

    private:
        static const DeviceFunctionTable FunctionTable;
        static const FunctionEntry<DeviceFunctionTable::Exec_FuncType> execFunctions[];
        static const FunctionEntry<DeviceFunctionTable::ReadString_FuncType> readStringFunctions[];
        static const FunctionEntry<DeviceFunctionTable::ReadToHALValue_FuncType> readValueFunctions[];

    private:
        union DrivePins {
            struct { gpio_num_t a, b; } hbridge;
            struct { gpio_num_t dir, enable, brk; } diren;
        };
        enum class DriveMode : uint8_t {
            HBridge,      // forward / backward
            DirEnable     // dir + enable
        };
        DrivePins pins;
        DriveMode mode;
        // private Static functions
        static void IRAM_ATTR endstop_isr(void* arg);
        static HALOperationResult exec_drive_to_min(Device* device);
        static HALOperationResult exec_drive_to_max(Device* device);
        static HALOperationResult exec_stop(Device* device);
        static HALOperationResult exec_reset(Device* device);
        static HALOperationResult getEndstops(Device* device, StringBuilderStreamer& sbs);
        static HALOperationResult getMinEndstop(Device* device, HALValue& val);
        static HALOperationResult getMaxEndstop(Device* device, HALValue& val);

        // private structures/enums/types
        enum class GpioRegType {
            Set,
            Clear
        };
        enum class State : uint32_t {
            Idle,
            MovingToMin,
            MovingToMax,
            TimeoutFault
        };
        enum class Location : int32_t { 
            Unknown = -1, 
            Min = 0, 
            Max = 1 
        };
        struct ISR_DATA {
            volatile gpio_num_t gpio_currentPin = gpio_num_t::GPIO_NUM_NC;
            void (* volatile gpio_reg_func)(uint32_t) = nullptr;
            volatile uint32_t gpio_currentActivePinMask = 0;
            volatile bool handled = false;
            volatile bool driveOn = false;
            volatile Location location = Location::Unknown;
        };

        // private member data
        ISR_DATA isr_data;

        State state = State::Idle;

        gpio_num_t pinMinEndStop;
        gpio_num_t pinMaxEndStop;

        bool pinMinEndStopActiveHigh;
        bool pinMaxEndStopActiveHigh;

        uint32_t motionStartMs;
        uint32_t timeoutMs;

        // private member functions
        void setup();
        void reset();
        void stopDrive();
        void driveToMin();
        void driveToMax();
        bool endMinActive() const;
        bool endMaxActive() const;

        void disableEndstopInterrupts();
        void configureISRData(gpio_num_t& somePin, GpioRegType regType);
        
    public:
        Actuator(DeviceCreateContext& context);
        ~Actuator() override;

        void loop() override;

        const Registry::DefineBase* GetRegistryDefine() override;

        HALOperationResult write(const HALValue& val) override;
        HALOperationResult read(HALValue& val) override;
        HALOperationResult read(const HALReadValueByCmd& val) override;
        HALOperationResult read(const HALReadStringRequestValue& val) override;

        Exec_FuncType GetExec_Function(ZeroCopyString& zcFuncName) override;
        /** Executes a device action with a provided command string, only used when doing remote cmd:s, i.e. not used by script. */
        HALOperationResult exec(const ZeroCopyString& cmd) override;

        void PrintTo(StringBuilderStreamer& sbs) override;
        
    };

} // namespace DALHAL
