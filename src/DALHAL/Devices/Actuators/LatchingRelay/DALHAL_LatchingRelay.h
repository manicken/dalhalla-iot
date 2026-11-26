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

#include <System/Platform/FastGpio.h>


#include <string>

#include <DALHAL/Core/Types/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>
#include <DALHAL/Core/Types/DALHAL_DeviceFunctionTable.h>

#include <DALHAL/Core/Reactive/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(RELAY_LATCHING)
#include "DALHAL_LatchingRelay_Reactive.h"
using LatchingRelay_DeviceBase = DALHAL::LatchingRelay_Reactive;
#else
using LatchingRelay_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {
    namespace JsonSchema { namespace LatchingRelay { struct Extractors; } } // forward declaration

    class LatchingRelay : public LatchingRelay_DeviceBase {
        friend struct JsonSchema::LatchingRelay::Extractors; // allow access to private memebers of this class from the schema extractor

    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static Device* Create(DeviceCreateContext& context);

    private:
        static const DeviceFunctionTable FunctionTable;
        static const FunctionEntry<FunctionTypes::Exec> execFunctions[];
        static const FunctionEntry<FunctionTypes::ReadString> readStringFunctions[];
        static const FunctionEntry<FunctionTypes::ReadToHALValue> readValueFunctions[];
        static const FunctionEntry<FunctionTypes::WriteHALValue> writeValueFunctions[];

        static HALOperationResult HALValue_primary_write(Device* device, const HALValue& val);
        static HALOperationResult HALValue_primary_read(Device* device, HALValue& val);
        
    private:
        // private Static functions
        static void PLATFORM_ISR_ATTR endstop_isr(void* arg);
        static HALOperationResult exec_drive_to_reset(Device* device);
        static HALOperationResult exec_drive_to_set(Device* device);
        static HALOperationResult exec_stop(Device* device);
        static HALOperationResult exec_resetMode(Device* device);
        static HALOperationResult getRelayStates(Device* device, ZeroCopyString zcParams, StringBuilderStreamer& sbs);
        static HALOperationResult getResetActive(Device* device, HALValue& val);
        static HALOperationResult getSetActive(Device* device, HALValue& val);

        // private structures/enums/types
        using Pin = Platform::FastGpio::Pin;
        union DrivePins {
            struct { Pin a, b; } direct;
            struct { Pin data, enable; } data_enable;
        };
        enum class DriveMode : uint8_t {
            Direct,      // set / reset
            DataEnable     // Data + Enable
        };
        enum class GpioRegType {
            Set,
            Clear
        };
        enum class State : uint32_t {
            Idle,
            DrivingReset,
            DrivingSet,
            TimeoutFault
        };
        enum class Location : int32_t { 
            Unknown = -1, 
            Reset = 0, 
            Set = 1 
        };
        struct ISR_DATA {
            /** drive pin that the ISR writes to driveOffLevel to kill the coil drive (NC = nothing to write) */
            volatile Pin drivePin = Platform::FastGpio::NC;
            volatile bool driveOffLevel = false;
            /** the feedback pin whose interrupt is enabled (only one at a time), disabled by the ISR */
            volatile Pin feedbackPin = Platform::FastGpio::NC;
            volatile bool handled = false;
            volatile bool driveOn = false;
            volatile Location location = Location::Unknown;
        };
        // private member data
        ISR_DATA isr_data;

        State state = State::Idle;

        DrivePins pins;
        DriveMode mode;

        Pin pinFeedbackReset;
        Pin pinFeedbackSet;

        bool pinFeedbackResetActiveHigh;
        bool pinFeedbackSetActiveHigh;

        uint32_t motionStartMs;
        uint32_t timeoutMs;

        // private member functions
        void resetMode();
        void stopDrive();
        void driveToReset();
        void driveToSet();
        bool resetActive() const;
        bool setActive() const;

        void disableFeedbackSignalInterrupts();
        void configureISRData(Pin drivePin, GpioRegType regType);
    
    public:
        LatchingRelay(DeviceCreateContext& context);
        ~LatchingRelay() override;

        void setup();
        void loop() override;

        const Registry::DefineBase* GetRegistryDefine() override;

        void PrintTo(StringBuilderStreamer& sbs) override;
        
    };

} // namespace DALHAL
