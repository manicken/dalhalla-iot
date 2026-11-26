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

#include "DALHAL_Actuator.h"
#include <Arduino.h> // millis

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/Reactive/DALHAL_ReactiveConfig.h>
#include <DALHAL/Core/Types/DALHAL_DeviceFunctionTable.h>

#include "DALHAL_Actuator_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase Actuator::RegistryDefine = {
        Create,
        &JsonSchema::Actuator::Root,
        DALHAL_REACTIVE_EVENT_TABLE(ACTUATOR),
        &Actuator::FunctionTable
    };

    /*virtual override*/
    const Registry::DefineBase* Actuator::GetRegistryDefine() {
        return &RegistryDefine;
    }

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::Exec> Actuator::execFunctions[] = {
        DALHAL_FUNCTION_ENTRY("close", exec_drive_to_min, "drive to close/min"),
        DALHAL_FUNCTION_ENTRY("open", exec_drive_to_max, "drive to open/max"),
        DALHAL_FUNCTION_ENTRY("toMin", exec_drive_to_min, "drive to min"),
        DALHAL_FUNCTION_ENTRY("toMax", exec_drive_to_max, "drive to max"),
        DALHAL_FUNCTION_ENTRY("stop", exec_stop, "stops the actuator"),
        DALHAL_FUNCTION_ENTRY("resetmode", exec_reset, "stop the actuator and reset the internal states"),
    };

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::ReadString> Actuator::readStringFunctions[] = {
        DALHAL_FUNCTION_ENTRY("endstops", getEndstops, "gets the endstops")
    };

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> Actuator::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "get current state"),
        DALHAL_FUNCTION_ENTRY_WITH_VAL_TYPE("minEndStop", getMinEndstop, "get min endstop state", FunctionValueType::_Bool_),
        DALHAL_FUNCTION_ENTRY_WITH_VAL_TYPE("maxEndStop", getMaxEndstop, "get max endstop state", FunctionValueType::_Bool_)
    };

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::WriteHALValue> Actuator::writeValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_write, "set the state")
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable Actuator::FunctionTable = {
        DALHAL_FUNCTION_TABLE_ENTRY(execFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeValueFunctions),
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        DALHAL_FUNCTION_TABLE_ENTRY(readStringFunctions),
        EmptyFunctionTable<FunctionTypes::WriteString>
    };
    
    /*static*/
    Device* Actuator::Create(DeviceCreateContext& context) {
        return new Actuator(context);
    }

    Actuator::Actuator(DeviceCreateContext& context) : Actuator_DeviceBase(context.deviceType), state(State::Idle) {
        JsonSchema::Actuator::Extractors::Apply(context, this);
        setup();
    }

    Actuator::~Actuator() {
        // FREE all used pins by setting them to INPUTS
        using namespace Platform::FastGpio;
        if (mode == DriveMode::HBridge) {
            Release(pins.hbridge.a);
            Release(pins.hbridge.b);
        } else if (mode == DriveMode::DirEnable) {
            Release(pins.diren.dir);
            Release(pins.diren.enable);
        }
        // the ISR arg points to this object, so the handlers must be removed
        DetachIsr(pinMinEndStop);
        DetachIsr(pinMaxEndStop);
        Release(pinMinEndStop);
        Release(pinMaxEndStop);
    }

    /*virtual override*/
    void Actuator::loop() {
        if (state == State::Idle) return;
        if (state == State::TimeoutFault) return;
        if (isr_data.handled) {
            isr_data.handled = false;
            if (isr_data.driveOn) {
                isr_data.driveOn = false;
                stopDrive(); // if for some reason the ISR missed it
                printf("\r\nWarning - ISR have not turned off the motor.\r\n");
            }
            if (state == State::MovingToMin) {
                isr_data.location = Location::Min;
#if HAS_REACTIVE_CUSTOM(ACTUATOR)
                triggerReachedMin();
#endif

            } else if (state == State::MovingToMax) {
                isr_data.location = Location::Max;
#if HAS_REACTIVE_CUSTOM(ACTUATOR)
                triggerReachedMax();
#endif
            }
#if HAS_REACTIVE_STATE_CHANGE(ACTUATOR)
            triggerStateChange();
#endif
            state = State::Idle;
            
            return;
        }
        
        if ((millis() - motionStartMs) > timeoutMs) {
            stopDrive();
            state = State::TimeoutFault;
            isr_data.location = Location::Unknown;
            GlobalLogger.Error(F("Actuator motion timeout"), decodeUID(uid).c_str());
            return;
        }
    }


    void Actuator::configureISRData(Pin drivePin, GpioRegType regType) {
        isr_data.handled = false;
        isr_data.driveOn = true;
        isr_data.driveOffLevel = (regType == GpioRegType::Set);
        isr_data.drivePin = drivePin;
    }

    /*static*/
    void PLATFORM_ISR_ATTR Actuator::endstop_isr(void* arg) {
        Actuator::ISR_DATA* isr_data = static_cast<Actuator::ISR_DATA*>(arg);

        // Immediate motor kill
        // Minimal ISR: just a single precomputed register write
        if (isr_data->drivePin != Platform::FastGpio::NC) {
            Platform::FastGpio::IsrWrite(isr_data->drivePin, isr_data->driveOffLevel);
            isr_data->driveOn = false;
        }
        // Disable only the triggering end stop interrupt (to ignore contact bounce)
        Platform::FastGpio::IsrDisable(isr_data->endstopPin);
        // signal to "main loop"
        isr_data->handled = true;
    }

    void Actuator::setup() {
        using namespace Platform::FastGpio;
        isr_data.location = Location::Unknown;
        isr_data.handled = false;

        if (mode == DriveMode::DirEnable) {
            ConfigureOutput(pins.diren.dir);
            ConfigureOutput(pins.diren.enable);
        } else if (mode == DriveMode::HBridge) {
            ConfigureOutput(pins.hbridge.a);
            ConfigureOutput(pins.hbridge.b);
        }
        // reserved for future modes

        // --------------------------
        // Configure endstop pins
        // --------------------------
        AttachEdgeIsr(pinMinEndStop, pinMinEndStopActiveHigh ? Edge::Rising : Edge::Falling, &endstop_isr, (void*)&this->isr_data);
        AttachEdgeIsr(pinMaxEndStop, pinMaxEndStopActiveHigh ? Edge::Rising : Edge::Falling, &endstop_isr, (void*)&this->isr_data);

        reset();
        stopDrive();
    }

    /*static*/
    HALOperationResult Actuator::exec_drive_to_min(Device* device) {
        static_cast<Actuator*>(device)->stopDrive(); // direct call no vtable
        static_cast<Actuator*>(device)->driveToMin(); // direct call no vtable
#if HAS_REACTIVE_EXEC(ACTUATOR)
        static_cast<Actuator*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult Actuator::exec_drive_to_max(Device* device) {
        static_cast<Actuator*>(device)->stopDrive(); // direct call no vtable
        static_cast<Actuator*>(device)->driveToMax(); // direct call no vtable
#if HAS_REACTIVE_EXEC(ACTUATOR)
        static_cast<Actuator*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult Actuator::exec_stop(Device* device) {
        static_cast<Actuator*>(device)->stopDrive(); // direct call no vtable
#if HAS_REACTIVE_EXEC(ACTUATOR)
        static_cast<Actuator*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult Actuator::exec_reset(Device* device) {
        static_cast<Actuator*>(device)->reset();
        static_cast<Actuator*>(device)->stopDrive();
#if HAS_REACTIVE_EXEC(ACTUATOR)
        static_cast<Actuator*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    /*static*/ 
    HALOperationResult Actuator::getEndstops(Device* device, ZeroCopyString zcParams, StringBuilderStreamer& sbs) {

        sbs.write_jsonBool(F("min"), static_cast<Actuator*>(device)->endMinActive());
        sbs.write_json_value_separator(); 
        sbs.write_jsonBool(F("max"), static_cast<Actuator*>(device)->endMaxActive());

        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult Actuator::getMinEndstop(Device* device, HALValue& val) {
        val.set(static_cast<Actuator*>(device)->endMinActive());
        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult Actuator::getMaxEndstop(Device* device, HALValue& val) {
        val.set(static_cast<Actuator*>(device)->endMaxActive());
        return HALOperationResult::Success;
    }

    /* static */ 
    HALOperationResult Actuator::HALValue_primary_write(Device* device, const HALValue& val) {
        Actuator& self = static_cast<Actuator&>(*device);
        if (!val.isBoolCompatible()) return HALOperationResult::WriteValueNaN;

        int32_t v = val.toInt();
        if (v != 0 && v != 1) {
            return HALOperationResult::InvalidArgument;
        }

        // Abort any previous motion
        self.stopDrive();

        if (v == 0) {
            self.driveToMin();
        } else {
            self.driveToMax();
        }
#if HAS_REACTIVE_WRITE(ACTUATOR)
        self.triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    /* static */
    HALOperationResult Actuator::HALValue_primary_read(Device* device, HALValue& val) {
        Actuator& self = static_cast<Actuator&>(*device);
        if (self.state == State::TimeoutFault) {
            return HALOperationResult::Timeout;
        }
        bool startReached = self.endMinActive();
        bool stopReached = self.endMaxActive();
        if (startReached && stopReached) {
            return HALOperationResult::HardwareFault;
        }
        else if (startReached) {
            val = (int32_t)0;
        }
        else if (stopReached) {
            val = (int32_t)1;
        }
        else {
            val = (int32_t)self.isr_data.location;
        }
        return HALOperationResult::Success;
    }

    void Actuator::reset() {
        state = State::Idle;
        isr_data.location = Location::Unknown;
    }

    void Actuator::disableEndstopInterrupts() {
        Platform::FastGpio::IsrDisable(pinMinEndStop);
        Platform::FastGpio::IsrDisable(pinMaxEndStop);
    }

    void Actuator::stopDrive() {
        using namespace Platform::FastGpio;
        if (state != State::TimeoutFault)
            state = State::Idle;
        motionStartMs = 0;
        disableEndstopInterrupts();
        if (mode == DriveMode::HBridge) {
            Write(pins.hbridge.a, false);
            Write(pins.hbridge.b, false);
        } else if (mode == DriveMode::DirEnable) {
            Write(pins.diren.enable, false);
        }
    }

    void Actuator::driveToMin() {
        using namespace Platform::FastGpio;
        if (endMinActive()) return;
        disableEndstopInterrupts();
        state = State::MovingToMin;
        isr_data.location = Location::Unknown;
        if (mode == DriveMode::HBridge) {
            Write(pins.hbridge.b, false);
            Write(pins.hbridge.a, true);
            configureISRData(pins.hbridge.a, GpioRegType::Clear);
        } else if (mode == DriveMode::DirEnable) {
            Write(pins.diren.dir, false);
            Write(pins.diren.enable, true);
            configureISRData(pins.diren.enable, GpioRegType::Clear);
        }
        motionStartMs = millis();
        // the max end stop interrupt is already disabled in disableEndstopInterrupts
        isr_data.endstopPin = pinMinEndStop;
        IsrEnable(pinMinEndStop);
    }

    void Actuator::driveToMax() {
        using namespace Platform::FastGpio;
        if (endMaxActive()) return;
        disableEndstopInterrupts();
        state = State::MovingToMax;
        isr_data.location = Location::Unknown;
        if (mode == DriveMode::HBridge) {
            Write(pins.hbridge.a, false);
            Write(pins.hbridge.b, true);
            configureISRData(pins.hbridge.b, GpioRegType::Clear);
        } else if (mode == DriveMode::DirEnable) {
            Write(pins.diren.dir, true); // opposite direction of driveToMin
            Write(pins.diren.enable, true);
            configureISRData(pins.diren.enable, GpioRegType::Clear);
        }
        motionStartMs = millis();
        // the min end stop interrupt is already disabled in disableEndstopInterrupts
        isr_data.endstopPin = pinMaxEndStop;
        IsrEnable(pinMaxEndStop);
    }

    bool Actuator::endMinActive() const {
        if (Platform::FastGpio::Available == false) {
            return true; // no GPIO backend, never drive
        }
        // 1) Physical pin says active
        int level = (pinMinEndStop != Platform::FastGpio::NC) ? Platform::FastGpio::Read(pinMinEndStop) : 0;
        bool pinActive = pinMinEndStopActiveHigh ? (level == 1) : (level == 0);

        if (pinActive) {
            return true;
        }

        // 2) Latched ISR confirmation
        if (isr_data.location == Location::Min) {
            return true;
        }

        return false;
    }

    bool Actuator::endMaxActive() const {
        if (Platform::FastGpio::Available == false) {
            return true; // no GPIO backend, never drive
        }
        // 1) Physical pin says active
        int level = (pinMaxEndStop != Platform::FastGpio::NC) ? Platform::FastGpio::Read(pinMaxEndStop) : 0;
        bool pinActive = pinMaxEndStopActiveHigh ? (level == 1) : (level == 0);

        if (pinActive) {
            return true;
        }
        // 2) Latched ISR confirmation
        if (isr_data.location == Location::Max) {
            return true;
        }

        return false;
    }

    /*virtual override*/
    void Actuator::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
        
        sbs.write_json_value_separator();
        if (mode == DriveMode::HBridge) {
            sbs.write(F("\"mode\":\"h-bridge\""));
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin a"), (int32_t)pins.hbridge.a);
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin b"), (int32_t)pins.hbridge.b);

        } else if (mode == DriveMode::DirEnable) {
            sbs.write(F("\"mode\":\"dir_enable\""));
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin dir"), (int32_t)pins.diren.dir);
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin enable"), (int32_t)pins.diren.enable);
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin brk"), (int32_t)pins.diren.brk);
        } else {
            sbs.write(F("\"mode\":\"__unknown__\""));
        }
        
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pinEndMin"), (int32_t)pinMinEndStop);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pinEndMax"), (int32_t)pinMaxEndStop);
    }

} // namespace DALHAL
