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
    //volatile const void* keep_LatchingRelay = &DALHAL::LatchingRelay::RegistryDefine;

    /*virtual override*/
    const Registry::DefineBase* LatchingRelay::GetRegistryDefine() {
        return &RegistryDefine;
    }

    //__attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::Exec_FuncType> LatchingRelay::execFunctions[] = {
        {"reset", &exec_drive_to_reset, "drive relay state to reset"},
        {"set", &exec_drive_to_set, "drive relay state to set"},
        {"toA", &exec_drive_to_set, "drive relay state to A"},
        {"toB", &exec_drive_to_reset, "drive relay state to B"},
        {"stop", &exec_stop, "stops the relay action"},
        {"resetmode", &exec_resetMode, "stop the relay action and reset the internal states"},
    };

    //__attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::ReadString_FuncType> LatchingRelay::readStringFunctions[] = {
        {"states", &getRelayStates, "gets the endstops"}
    };

    //__attribute__((used, externally_visible))
    constexpr FunctionEntry<DeviceFunctionTable::ReadToHALValue_FuncType> LatchingRelay::readValueFunctions[] = {
        {"resetActive", &getResetActive, "get if reset state is active", FunctionValueType::_Bool_},
        {"setActive", &getSetActive, "get if set state is active", FunctionValueType::_Bool_}
    };

    //__attribute__((used, externally_visible))
    constexpr DeviceFunctionTable LatchingRelay::FunctionTable = {
        {execFunctions, sizeof(execFunctions) / sizeof(execFunctions[0])}, 
        {readValueFunctions, sizeof(readValueFunctions) / sizeof(readValueFunctions[0])}, 
        EmptyFunctionTable<DeviceFunctionTable::WriteHALValue_FuncType>,
        EmptyFunctionTable<DeviceFunctionTable::BracketOpRead_FuncType>,
        EmptyFunctionTable<DeviceFunctionTable::BracketOpWrite_FuncType>,
        {readStringFunctions, sizeof(readStringFunctions) / sizeof(readStringFunctions[0])},
        EmptyFunctionTable<DeviceFunctionTable::WriteString_FuncType>
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
#if defined(ESP32)
        // FREE all used pins by setting them to INPUTS
        uint64_t mask = 0;

        if (mode == DriveMode::Direct) {
            mask |= (1ULL << pins.direct.a);
            mask |= (1ULL << pins.direct.b);
        } else if (mode == DriveMode::DataEnable) {
            mask |= (1ULL << pins.data_enable.data);
            mask |= (1ULL << pins.data_enable.enable);
        }

        mask |= (1ULL << pinFeedbackReset);
        mask |= (1ULL << pinFeedbackSet);

        if (mask == 0) return;  // nothing to configure

        gpio_config_t io_conf = {};
        io_conf.pin_bit_mask = mask;
        io_conf.mode = GPIO_MODE_INPUT;      // set all to input
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.intr_type = GPIO_INTR_DISABLE; // no interrupts

        esp_err_t res = gpio_config(&io_conf);
        if (res != ESP_OK) {
            printf("\r\nesp erro while gpio_config @ ~Actuator:%d\r\n", (uint32_t)res);
        }
#elif defined(ESP8266)
        if (mode == DriveMode::Direct) {
            pinMode((uint8_t)pins.direct.a, INPUT);
            pinMode((uint8_t)pins.direct.b, INPUT);
        } else if (mode == DriveMode::DataEnable) {
            pinMode((uint8_t)pins.data_enable.data, INPUT);
            pinMode((uint8_t)pins.data_enable.enable, INPUT);
        }
#endif
    }

#if defined(ESP32) && !defined(esp32c3) && !defined(esp32c6)
    static void IRAM_ATTR WriteTo_GPIOs_A_SetReg(uint32_t mask) {
        GPIO.out_w1ts = mask;
    }

    static void IRAM_ATTR WriteTo_GPIOs_A_ClearReg(uint32_t mask) {
        GPIO.out_w1tc = mask;
    }

    static void IRAM_ATTR WriteTo_GPIOs_B_SetReg(uint32_t mask) {
        GPIO.out1_w1ts.val = mask;
    }

    static void IRAM_ATTR WriteTo_GPIOs_B_ClearReg(uint32_t mask) {
        GPIO.out1_w1tc.val = mask;
    }

#elif defined(esp32c3) || defined(esp32c6)
    static void IRAM_ATTR WriteTo_GPIOs_SetReg(uint32_t mask) {
        GPIO.out_w1ts.val = mask;
    }

    static void IRAM_ATTR WriteTo_GPIOs_ClearReg(uint32_t mask) {
        GPIO.out_w1tc.val = mask;
    }

#endif

void LatchingRelay::configureISRData(gpio_num_t& somePin, GpioRegType regType) {
        isr_data.handled = false;
        isr_data.driveOn = true;
        isr_data.gpio_currentPin = somePin;
        
#if defined(ESP32) && !defined(esp32c3) && !defined(esp32c6)
        if (somePin < 32) {
            isr_data.gpio_currentActivePinMask = (1UL << somePin);
            if (regType == GpioRegType::Set) {
                isr_data.gpio_reg_func = &WriteTo_GPIOs_A_SetReg;
            } else if (regType == GpioRegType::Clear) {
                isr_data.gpio_reg_func = &WriteTo_GPIOs_A_ClearReg;
            }
 
        } else {
            isr_data.gpio_currentActivePinMask = (1UL << (somePin - 32));
            if (regType == GpioRegType::Set) {
                isr_data.gpio_reg_func = &WriteTo_GPIOs_B_SetReg;
            } else if (regType == GpioRegType::Clear) {
                isr_data.gpio_reg_func = &WriteTo_GPIOs_B_ClearReg;
            }
        }
#elif defined(esp32c3) || defined(esp32c6)
        isr_data.gpio_currentActivePinMask = (1UL << somePin);
        if (regType == GpioRegType::Set) {
            isr_data.gpio_reg_func = &WriteTo_GPIOs_SetReg;
        } else if (regType == GpioRegType::Clear) {
            isr_data.gpio_reg_func = &WriteTo_GPIOs_ClearReg;
        }
#endif
    }

    /*static*/
    void IRAM_ATTR LatchingRelay::endstop_isr(void* arg) {
        LatchingRelay::ISR_DATA* isr_data = static_cast<LatchingRelay::ISR_DATA*>(arg);

        // Immediate motor kill
        // Minimal ISR: just write precomputed register and mask
        if (isr_data->gpio_reg_func != nullptr) {
            isr_data->gpio_reg_func(isr_data->gpio_currentActivePinMask);
            isr_data->driveOn = false;
        }
#if defined(ESP32)
        // Disable only the triggering interrupt
        gpio_intr_disable(isr_data->gpio_currentPin);
#elif defined(ESP8266)

#endif
        // signal to "main loop"
        isr_data->handled = true;
    }

    void LatchingRelay::setup() {
        isr_data.location = Location::Unknown;
        isr_data.handled = false;

#if defined(ESP32)
        gpio_install_isr_service(ESP_INTR_FLAG_IRAM);


        if (mode == DriveMode::DataEnable) {
            gpio_config_t io_conf{};
            io_conf.intr_type = GPIO_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_OUTPUT;
            io_conf.pin_bit_mask = (1ULL << pins.data_enable.data) | (1ULL << pins.data_enable.enable);
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
            gpio_config(&io_conf);
        } else if (mode == DriveMode::Direct) {
            gpio_config_t io_conf{};
            io_conf.intr_type = GPIO_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_OUTPUT;
            io_conf.pin_bit_mask = (1ULL << pins.direct.a) | (1ULL << pins.direct.b);
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
            gpio_config(&io_conf);
        }
        // reserved for future modes


        // --------------------------
        // Configure endstop pins
        // --------------------------
        if (pinFeedbackReset != GPIO_NUM_NC) {
            gpio_config_t io_conf{};
            io_conf.intr_type = pinFeedbackResetActiveHigh
                                ? GPIO_INTR_POSEDGE
                                : GPIO_INTR_NEGEDGE;
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pin_bit_mask = (1ULL << pinFeedbackReset);
            io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;//pinEndMinActiveHigh ? gpio_pullup_t::GPIO_PULLUP_DISABLE : gpio_pullup_t::GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;//pinEndMinActiveHigh ? gpio_pulldown_t::GPIO_PULLDOWN_ENABLE : gpio_pulldown_t::GPIO_PULLDOWN_DISABLE;
            gpio_config(&io_conf);

            // Attach interrupt handler
            gpio_isr_handler_add(pinFeedbackReset, &endstop_isr, (void*)&this->isr_data);
        }

        if (pinFeedbackSet != GPIO_NUM_NC) {
            gpio_config_t io_conf{};
            io_conf.intr_type = pinFeedbackSetActiveHigh
                                ? GPIO_INTR_POSEDGE
                                : GPIO_INTR_NEGEDGE;
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pin_bit_mask = (1ULL << pinFeedbackSet);
            io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;//pinEndMaxActiveHigh ? gpio_pullup_t::GPIO_PULLUP_DISABLE : gpio_pullup_t::GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;//pinEndMaxActiveHigh ? gpio_pulldown_t::GPIO_PULLDOWN_ENABLE : gpio_pulldown_t::GPIO_PULLDOWN_DISABLE;
            gpio_config(&io_conf);

            gpio_isr_handler_add(pinFeedbackSet, &endstop_isr, (void*)&this->isr_data);
        }
#elif defined(ESP8266)

#endif
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
    HALOperationResult LatchingRelay::getRelayStates(Device* device, StringBuilderStreamer& sbs) {

        sbs.write_jsonBool(F("reset"), static_cast<LatchingRelay*>(device)->resetActive());
        sbs.write_json_value_separator(); 
        sbs.write_jsonBool(F("set"), static_cast<LatchingRelay*>(device)->setActive());

        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult LatchingRelay::getResetActive(Device* device, HALValue& val) {
        val = static_cast<LatchingRelay*>(device)->resetActive();
        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult LatchingRelay::getSetActive(Device* device, HALValue& val) {
        val = static_cast<LatchingRelay*>(device)->setActive();
        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult LatchingRelay::exec_drive_to_reset(Device* device) {
        static_cast<LatchingRelay*>(device)->stopDrive(); // direct call no vtable
        static_cast<LatchingRelay*>(device)->driveToReset(); // direct call no vtable
#if HAS_REACTIVE_EXEC(RELAY_LATCHING)
        static_cast<LatchingRelay*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult LatchingRelay::exec_drive_to_set(Device* device) {
        static_cast<LatchingRelay*>(device)->stopDrive(); // direct call no vtable
        static_cast<LatchingRelay*>(device)->driveToSet(); // direct call no vtable
#if HAS_REACTIVE_EXEC(RELAY_LATCHING)
        static_cast<LatchingRelay*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    /*static*/
    HALOperationResult LatchingRelay::exec_stop(Device* device) {
        static_cast<LatchingRelay*>(device)->stopDrive(); // direct call no vtable
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

    Device::Exec_FuncType LatchingRelay::GetExec_Function(ZeroCopyString& zcFuncName) {
        return GetDeviceFunction<Exec_FuncType>(FunctionTable.exec, zcFuncName);
    }

    /*virtual override*/
    HALOperationResult LatchingRelay::exec(const ZeroCopyString& cmd) {
        Exec_FuncType fn = GetDeviceFunction<Exec_FuncType>(FunctionTable.exec, cmd);
        if (fn == nullptr) { return HALOperationResult::UnsupportedCommand; }
        return fn(this);
    }

    /*virtual override*/
    HALOperationResult LatchingRelay::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) { /*printf("\nSinglePulseOutput::write TEST\n");*/ return HALOperationResult::Success; }// test write to check feature
        if (!val.isBoolCompatible()) { 
            GlobalLogger.Error(F("LatchingRelay::write !val.isBoolCompatible(): "), val.typeToString());
            return HALOperationResult::WriteValueNaN;
        }

        int32_t v = val.toInt();
        if (v != 0 && v != 1) {
            return HALOperationResult::InvalidArgument;
        }

        // Abort any previous motion
        stopDrive();

        if (v == 0) {
            driveToReset();
        } else {
            driveToSet();
        }
#if HAS_REACTIVE_WRITE(ACTUATOR)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    /*virtual override*/ 
    HALOperationResult LatchingRelay::read(const HALReadValueByCmd& val) /*override*/ {
        DeviceFunctionTable::ReadToHALValue_FuncType fn = GetDeviceFunction<DeviceFunctionTable::ReadToHALValue_FuncType>(FunctionTable.readValue, val.cmd);
        if (fn == nullptr) { return HALOperationResult::UnsupportedCommand; }
        return fn(this, val.out_value);
    }

    /*virtual override*/
    HALOperationResult LatchingRelay::read(const HALReadStringRequestValue& val) {
        DeviceFunctionTable::ReadString_FuncType fn = GetDeviceFunction<DeviceFunctionTable::ReadString_FuncType>(FunctionTable.readString, val.cmd);
        if (fn == nullptr) { return HALOperationResult::UnsupportedCommand; }
        return fn(this, val.sbs);
    }

    /*virtual override*/
    HALOperationResult LatchingRelay::read(HALValue& val) {
        if (state == State::TimeoutFault) {
            return HALOperationResult::Timeout;
        }
        bool startReached = resetActive();
        bool stopReached = setActive();
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
            val = (int32_t)isr_data.location;
        }
#if HAS_REACTIVE(RELAY_LATCHING, READ)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }

    void LatchingRelay::resetMode() {
        state = State::Idle;
        isr_data.location = Location::Unknown;
    }

    void LatchingRelay::disableFeedbackSignalInterrupts() {
#if defined(ESP32)
        if (pinFeedbackReset != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_disable(pinFeedbackReset);
        }
        if (pinFeedbackSet != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_disable(pinFeedbackSet);
        }
#elif defined(ESP8266)

#endif
    }

    void LatchingRelay::stopDrive() {
        if (state != State::TimeoutFault)
            state = State::Idle;
        motionStartMs = 0;
        disableFeedbackSignalInterrupts();
#if defined(ESP32)
        if (mode == DriveMode::Direct) {
            gpio_set_level(pins.direct.a, 0);
            gpio_set_level(pins.direct.b, 0);
        } else if (mode == DriveMode::DataEnable) {
            gpio_set_level(pins.data_enable.enable, 0);
        }
#elif defined(ESP8266)

#endif
    }

    void LatchingRelay::driveToReset() {
        if (resetActive()) return;
        disableFeedbackSignalInterrupts();
        state = State::DrivingReset;
        isr_data.location = Location::Unknown;
#if defined(ESP32)
        if (mode == DriveMode::Direct) {
            gpio_set_level(pins.direct.b, 0);
            gpio_set_level(pins.direct.a, 1);            
            configureISRData(pins.direct.a, GpioRegType::Clear);
        } else if (mode == DriveMode::DataEnable) {
            gpio_set_level(pins.data_enable.data, 0);
            gpio_set_level(pins.data_enable.enable, 1);
            configureISRData(pins.data_enable.enable, GpioRegType::Clear);
        }
#elif defined(ESP8266)

#endif
        motionStartMs = millis();
#if defined(ESP32)
        //gpio_intr_disable(pinMaxEndStop); allready disabled in disableEndstopInterrupts
        if (pinFeedbackReset != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_enable(pinFeedbackReset);
        }
#elif defined(ESP8266)

#endif
    }

    void LatchingRelay::driveToSet() {
        if (setActive()) return;
        disableFeedbackSignalInterrupts();
        state = State::DrivingSet;
        isr_data.location = Location::Unknown;
#if defined(ESP32)
        if (mode == DriveMode::Direct) {
            gpio_set_level(pins.direct.a, 0);
            gpio_set_level(pins.direct.b, 1);
            configureISRData(pins.direct.b, GpioRegType::Clear);
        } else if (mode == DriveMode::DataEnable) {
            gpio_set_level(pins.data_enable.data, 0);
            gpio_set_level(pins.data_enable.enable, 1);
            configureISRData(pins.data_enable.enable, GpioRegType::Clear);
        }
#elif defined(ESP8266)

#endif
        motionStartMs = millis();
#if defined(ESP32)
        // gpio_intr_disable(pinMinEndStop); allready disabled in disableEndstopInterrupts
        if (pinFeedbackSet != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_enable(pinFeedbackSet);
        }
#elif defined(ESP8266)

#endif
    }

    bool LatchingRelay::resetActive() const {
#if defined(ESP32)
        // 1) Physical pin says active
        int level = (pinFeedbackReset != gpio_num_t::GPIO_NUM_NC)?gpio_get_level(pinFeedbackReset):0;
        bool pinActive = pinFeedbackResetActiveHigh ? (level == 1) : (level == 0);

        if (pinActive) {
            return true;
        }

        // 2) Latched ISR confirmation
        if (isr_data.location == Location::Reset) {
            return true;
        }

        return false;
#else
        return true;
#endif
    }

    bool LatchingRelay::setActive() const {
#if defined(ESP32)
        // 1) Physical pin says active
        int level = (pinFeedbackSet != gpio_num_t::GPIO_NUM_NC)?gpio_get_level(pinFeedbackSet):0;
        bool pinActive = pinFeedbackSetActiveHigh ? (level == 1) : (level == 0);

        if (pinActive) {
            return true;
        }
        // 2) Latched ISR confirmation
        if (isr_data.location == Location::Set) {
            return true;
        }

        return false;
#else
        return true;
#endif
    }

    void LatchingRelay::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
        
        sbs.write_json_value_separator();
        if (mode == DriveMode::Direct) {
            sbs.write(F("\"mode\":\"Direct\""));
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin a"), (int8_t)pins.direct.a);
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin b"), (int8_t)pins.direct.b);
        } else if (mode == DriveMode::DataEnable) {
            sbs.write(F("\"mode\":\"data_enable\""));
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin data"), (int8_t)pins.data_enable.data);
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("pin enable"), (int8_t)pins.data_enable.enable);
        } else {
            sbs.write(F("\"mode\":\"__unknown__\""));
        }

        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pinFeedbackReset"), (int8_t)pinFeedbackReset);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pinFeedbackSet"), (int8_t)pinFeedbackSet);
    }

} // namespace DALHAL
