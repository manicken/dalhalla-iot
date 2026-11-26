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

#include "DALHAL_LatchingRelay.h"
#include <Arduino.h> // millis

#include <DALHAL/Support/DALHAL_Logger.h>

#include "DALHAL_LatchingRelay_JSON_Schema.h"

namespace DALHAL {

    //__attribute__((used, externally_visible))
    constexpr Registry::DefineBase LatchingRelay::RegistryDefine = {
        Create,
        &JsonSchema::LatchingRelay::Root,
        DALHAL_REACTIVE_EVENT_TABLE(RELAY_LATCHING),
        &LatchingRelay::FunctionTable
    };

    /* override */
    const Registry::DefineBase* LatchingRelay::GetRegistryDefine() {
        return &RegistryDefine;
    }

    //__attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::Exec> LatchingRelay::execFunctions[] = {
        DALHAL_FUNCTION_ENTRY("reset", exec_drive_to_reset, "drive relay state to reset"),
        DALHAL_FUNCTION_ENTRY("set", exec_drive_to_set, "drive relay state to set"),
        DALHAL_FUNCTION_ENTRY("toA", exec_drive_to_set, "drive relay state to A"),
        DALHAL_FUNCTION_ENTRY("toB", exec_drive_to_reset, "drive relay state to B"),
        DALHAL_FUNCTION_ENTRY("stop", exec_stop, "stops the relay action"),
        DALHAL_FUNCTION_ENTRY("resetmode", exec_resetMode, "stop the relay action and reset the internal states"),
    };

    //__attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::ReadString> LatchingRelay::readStringFunctions[] = {
        DALHAL_FUNCTION_ENTRY("states", getRelayStates, "gets the endstops")
    };

    //__attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> LatchingRelay::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "get current state"),
        DALHAL_FUNCTION_ENTRY_WITH_VAL_TYPE("resetActive", getResetActive, "get if reset state is active", FunctionValueType::_Bool_),
        DALHAL_FUNCTION_ENTRY_WITH_VAL_TYPE("setActive", getSetActive, "get if set state is active", FunctionValueType::_Bool_)
    };

    constexpr FunctionEntry<FunctionTypes::WriteHALValue> LatchingRelay::writeValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_write, "set the state")
    };

    //__attribute__((used, externally_visible))
    constexpr DeviceFunctionTable LatchingRelay::FunctionTable = {
        DALHAL_FUNCTION_TABLE_ENTRY(execFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeValueFunctions),
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        DALHAL_FUNCTION_TABLE_ENTRY(readStringFunctions),
        EmptyFunctionTable<FunctionTypes::WriteString>
    };

    /*static*/
    Device* LatchingRelay::Create(DeviceCreateContext& context) {
        return new LatchingRelay(context);
    }

    LatchingRelay::LatchingRelay(DeviceCreateContext& context) : LatchingRelay_DeviceBase(context.deviceType), state(State::Idle) {
        JsonSchema::LatchingRelay::Extractors::Apply(context, this);
        setup();
    }

    /*override*/
    LatchingRelay::~LatchingRelay() {
        // FREE all used pins by setting them to INPUTS
        using namespace Platform::FastGpio;
        if (mode == DriveMode::Direct) {
            Release(pins.direct.a);
            Release(pins.direct.b);
        } else if (mode == DriveMode::DataEnable) {
            Release(pins.data_enable.data);
            Release(pins.data_enable.enable);
        }
        // the ISR arg points to this object, so the handlers must be removed
        DetachIsr(pinFeedbackReset);
        DetachIsr(pinFeedbackSet);
        Release(pinFeedbackReset);
        Release(pinFeedbackSet);
    }

    void LatchingRelay::configureISRData(Pin drivePin, GpioRegType regType) {
        isr_data.handled = false;
        isr_data.driveOn = true;
        isr_data.driveOffLevel = (regType == GpioRegType::Set);
        isr_data.drivePin = drivePin;
    }

    /*static*/
    void PLATFORM_ISR_ATTR LatchingRelay::endstop_isr(void* arg) {
        LatchingRelay::ISR_DATA* isr_data = static_cast<LatchingRelay::ISR_DATA*>(arg);

        // Immediate coil drive kill
        // Minimal ISR: just a single precomputed register write
        if (isr_data->drivePin != Platform::FastGpio::NC) {
            Platform::FastGpio::IsrWrite(isr_data->drivePin, isr_data->driveOffLevel);
            isr_data->driveOn = false;
        }
        // Disable only the triggering feedback interrupt (to ignore contact bounce)
        Platform::FastGpio::IsrDisable(isr_data->feedbackPin);
        // signal to "main loop"
        isr_data->handled = true;
    }

    void LatchingRelay::setup() {
        using namespace Platform::FastGpio;
        isr_data.location = Location::Unknown;
        isr_data.handled = false;

        if (mode == DriveMode::DataEnable) {
            ConfigureOutput(pins.data_enable.data);
            ConfigureOutput(pins.data_enable.enable);
        } else if (mode == DriveMode::Direct) {
            ConfigureOutput(pins.direct.a);
            ConfigureOutput(pins.direct.b);
        }
        // reserved for future modes

        // --------------------------
        // Configure feedback pins
        // --------------------------
        AttachEdgeIsr(pinFeedbackReset, pinFeedbackResetActiveHigh ? Edge::Rising : Edge::Falling, &endstop_isr, (void*)&this->isr_data);
        AttachEdgeIsr(pinFeedbackSet, pinFeedbackSetActiveHigh ? Edge::Rising : Edge::Falling, &endstop_isr, (void*)&this->isr_data);

        driveToReset();
        stopDrive();
    }

    /*virtual override*/
    void LatchingRelay::loop() {
        if (state == State::Idle) return;
        if (state == State::TimeoutFault) return;
        if (isr_data.handled) {
            isr_data.handled = false;
            if (isr_data.driveOn) {
                isr_data.driveOn = false;
                stopDrive(); // if for some reason the ISR missed it
                printf("\r\nWarning - ISR have not turned off the motor.\r\n");
            }
            if (state == State::DrivingReset) {
                isr_data.location = Location::Reset;
#if HAS_REACTIVE_CUSTOM(RELAY_LATCHING)
                triggerReset();
#endif
            } else if (state == State::DrivingSet) {
                isr_data.location = Location::Set;
#if HAS_REACTIVE_CUSTOM(RELAY_LATCHING)
                triggerSet();
#endif
            }
            state = State::Idle;
#if HAS_REACTIVE_STATE_CHANGE(RELAY_LATCHING)
            triggerStateChange();
#endif
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

    /*static*/ 
    HALOperationResult LatchingRelay::getRelayStates(Device* device, ZeroCopyString zcParams, StringBuilderStreamer& sbs) {

        sbs.write_jsonBool(F("reset"), static_cast<LatchingRelay*>(device)->resetActive());
        sbs.write_json_value_separator(); 
        sbs.write_jsonBool(F("set"), static_cast<LatchingRelay*>(device)->setActive());

        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult LatchingRelay::getResetActive(Device* device, HALValue& val) {
        val.set(static_cast<LatchingRelay*>(device)->resetActive());
        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult LatchingRelay::getSetActive(Device* device, HALValue& val) {
        val.set(static_cast<LatchingRelay*>(device)->setActive());
        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult LatchingRelay::exec_drive_to_reset(Device* device) {
        static_cast<LatchingRelay*>(device)->stopDrive(); 
        static_cast<LatchingRelay*>(device)->driveToReset();
#if HAS_REACTIVE_EXEC(RELAY_LATCHING)
        static_cast<LatchingRelay*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult LatchingRelay::exec_drive_to_set(Device* device) {
        static_cast<LatchingRelay*>(device)->stopDrive(); 
        static_cast<LatchingRelay*>(device)->driveToSet();
#if HAS_REACTIVE_EXEC(RELAY_LATCHING)
        static_cast<LatchingRelay*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult LatchingRelay::exec_stop(Device* device) {
        static_cast<LatchingRelay*>(device)->stopDrive();
#if HAS_REACTIVE_EXEC(RELAY_LATCHING)
        static_cast<LatchingRelay*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }


  

    /*static*/
    HALOperationResult LatchingRelay::exec_resetMode(Device* device) {
        static_cast<LatchingRelay*>(device)->driveToReset();
        static_cast<LatchingRelay*>(device)->stopDrive();
#if HAS_REACTIVE_EXEC(RELAY_LATCHING)
        static_cast<LatchingRelay*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    /* static */
    HALOperationResult LatchingRelay::HALValue_primary_write(Device* device, const HALValue& val) {
        LatchingRelay& self = static_cast<LatchingRelay&>(*device);
        if (!val.isBoolCompatible()) { 
            GlobalLogger.Error(F("LatchingRelay::write !val.isBoolCompatible(): "), val.typeToString());
            return HALOperationResult::WriteValueNaN;
        }

        int32_t v = val.toInt();
        if (v != 0 && v != 1) {
            return HALOperationResult::InvalidArgument;
        }

        // Abort any previous motion
        self.stopDrive();

        if (v == 0) {
            self.driveToReset();
        } else {
            self.driveToSet();
        }
#if HAS_REACTIVE_WRITE(ACTUATOR)
        self.triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    /* static */
    HALOperationResult LatchingRelay::HALValue_primary_read(Device* device, HALValue& val) {
        LatchingRelay& self = static_cast<LatchingRelay&>(*device);
        if (self.state == State::TimeoutFault) {
            return HALOperationResult::Timeout;
        }
        bool startReached = self.resetActive();
        bool stopReached = self.setActive();
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
#if HAS_REACTIVE(RELAY_LATCHING, READ)
        self.triggerRead();
#endif
        return HALOperationResult::Success;
    }

    void LatchingRelay::resetMode() {
        state = State::Idle;
        isr_data.location = Location::Unknown;
    }

    void LatchingRelay::disableFeedbackSignalInterrupts() {
        Platform::FastGpio::IsrDisable(pinFeedbackReset);
        Platform::FastGpio::IsrDisable(pinFeedbackSet);
    }

    void LatchingRelay::stopDrive() {
        using namespace Platform::FastGpio;
        if (state != State::TimeoutFault)
            state = State::Idle;
        motionStartMs = 0;
        disableFeedbackSignalInterrupts();
        if (mode == DriveMode::Direct) {
            Write(pins.direct.a, false);
            Write(pins.direct.b, false);
        } else if (mode == DriveMode::DataEnable) {
            Write(pins.data_enable.enable, false);
        }
    }

    void LatchingRelay::driveToReset() {
        using namespace Platform::FastGpio;
        if (resetActive()) return;
        disableFeedbackSignalInterrupts();
        state = State::DrivingReset;
        isr_data.location = Location::Unknown;
        if (mode == DriveMode::Direct) {
            Write(pins.direct.b, false);
            Write(pins.direct.a, true);
            configureISRData(pins.direct.a, GpioRegType::Clear);
        } else if (mode == DriveMode::DataEnable) {
            Write(pins.data_enable.data, false);
            Write(pins.data_enable.enable, true);
            configureISRData(pins.data_enable.enable, GpioRegType::Clear);
        }
        motionStartMs = millis();
        // the set feedback interrupt is already disabled in disableFeedbackSignalInterrupts
        isr_data.feedbackPin = pinFeedbackReset;
        IsrEnable(pinFeedbackReset);
    }

    void LatchingRelay::driveToSet() {
        using namespace Platform::FastGpio;
        if (setActive()) return;
        disableFeedbackSignalInterrupts();
        state = State::DrivingSet;
        isr_data.location = Location::Unknown;
        if (mode == DriveMode::Direct) {
            Write(pins.direct.a, false);
            Write(pins.direct.b, true);
            configureISRData(pins.direct.b, GpioRegType::Clear);
        } else if (mode == DriveMode::DataEnable) {
            Write(pins.data_enable.data, true); // opposite coil polarity of driveToReset
            Write(pins.data_enable.enable, true);
            configureISRData(pins.data_enable.enable, GpioRegType::Clear);
        }
        motionStartMs = millis();
        // the reset feedback interrupt is already disabled in disableFeedbackSignalInterrupts
        isr_data.feedbackPin = pinFeedbackSet;
        IsrEnable(pinFeedbackSet);
    }

    bool LatchingRelay::resetActive() const {
        if (Platform::FastGpio::Available == false) {
            return true; // no GPIO backend, never drive
        }
        // 1) Physical pin says active
        int level = (pinFeedbackReset != Platform::FastGpio::NC) ? Platform::FastGpio::Read(pinFeedbackReset) : 0;
        bool pinActive = pinFeedbackResetActiveHigh ? (level == 1) : (level == 0);

        if (pinActive) {
            return true;
        }

        // 2) Latched ISR confirmation
        if (isr_data.location == Location::Reset) {
            return true;
        }

        return false;
    }

    bool LatchingRelay::setActive() const {
        if (Platform::FastGpio::Available == false) {
            return true; // no GPIO backend, never drive
        }
        // 1) Physical pin says active
        int level = (pinFeedbackSet != Platform::FastGpio::NC) ? Platform::FastGpio::Read(pinFeedbackSet) : 0;
        bool pinActive = pinFeedbackSetActiveHigh ? (level == 1) : (level == 0);

        if (pinActive) {
            return true;
        }
        // 2) Latched ISR confirmation
        if (isr_data.location == Location::Set) {
            return true;
        }

        return false;
    }

    void LatchingRelay::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
        
        sbs.write_json_value_separator();
        if (mode == DriveMode::Direct) {
            sbs.write(F("\"mode\":\"Direct\""));
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin a"), (int32_t)pins.direct.a);
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin b"), (int32_t)pins.direct.b);
        } else if (mode == DriveMode::DataEnable) {
            sbs.write(F("\"mode\":\"data_enable\""));
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin data"), (int32_t)pins.data_enable.data);
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin enable"), (int32_t)pins.data_enable.enable);
        } else {
            sbs.write(F("\"mode\":\"__unknown__\""));
        }

        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pinFeedbackReset"), (int32_t)pinFeedbackReset);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pinFeedbackSet"), (int32_t)pinFeedbackSet);
    }

} // namespace DALHAL
