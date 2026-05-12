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

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>

#include "DALHAL_Actuator_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase Actuator::RegistryDefine = {
        Create,
        &JsonSchema::Actuator::Root,
        DALHAL_REACTIVE_EVENT_TABLE(ACTUATOR)
    };
    //volatile const void* keep_Actuator = &DALHAL::Actuator::RegistryDefine;

    Device* Actuator::Create(DeviceCreateContext& context) {
        return new Actuator(context);
    }

    Actuator::Actuator(DeviceCreateContext& context) : Actuator_DeviceBase(context.deviceType), state(State::Idle) {
        JsonSchema::Actuator::Extractors::Apply(context, this);
        setup();
    }

    Actuator::~Actuator() {
        // FREE all used pins by setting them to INPUTS
#if defined(ESP32)
        uint64_t mask = 0;

        if (mode == DriveMode::HBridge) {
            mask |= (1ULL << pins.hbridge.a);
            mask |= (1ULL << pins.hbridge.b);
        } else if (mode == DriveMode::DirEnable) {
            mask |= (1ULL << pins.diren.dir);
            mask |= (1ULL << pins.diren.enable);
        }

        mask |= (1ULL << pinMinEndStop);
        mask |= (1ULL << pinMaxEndStop);

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
        if (mode == DriveMode::HBridge) {
            pinMode((uint8_t)pins.hbridge.a, INPUT);
            pinMode((uint8_t)pins.hbridge.b, INPUT);
        } else if (mode == DriveMode::DirEnable) {
            pinMode((uint8_t)pins.diren.dir, INPUT);
            pinMode((uint8_t)pins.diren.enable, INPUT);
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

void Actuator::configureISRData(gpio_num_t& somePin, GpioRegType regType) {
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

    void IRAM_ATTR Actuator::endstop_isr(void* arg) {
        Actuator::ISR_DATA* isr_data = static_cast<Actuator::ISR_DATA*>(arg);

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

    void Actuator::setup() {
        isr_data.location = Location::Unknown;
        isr_data.handled = false;

#if defined(ESP32)
        gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

        if (mode == DriveMode::DirEnable) {
            gpio_config_t io_conf{};
            io_conf.intr_type = GPIO_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_OUTPUT;
            io_conf.pin_bit_mask = (1ULL << pins.diren.dir) | (1ULL << pins.diren.enable);
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
            gpio_config(&io_conf);
        } else if (mode == DriveMode::HBridge) {
            gpio_config_t io_conf{};
            io_conf.intr_type = GPIO_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_OUTPUT;
            io_conf.pin_bit_mask = (1ULL << pins.hbridge.a) | (1ULL << pins.hbridge.b);
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
            gpio_config(&io_conf);
        }
        // reserved for future modes

        // --------------------------
        // Configure endstop pins
        // --------------------------
        if (pinMinEndStop != GPIO_NUM_NC) {
            gpio_config_t io_conf{};
            io_conf.intr_type = pinMinEndStopActiveHigh
                                ? GPIO_INTR_POSEDGE
                                : GPIO_INTR_NEGEDGE;
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pin_bit_mask = (1ULL << pinMinEndStop);
            io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;//pinEndMinActiveHigh ? gpio_pullup_t::GPIO_PULLUP_DISABLE : gpio_pullup_t::GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;//pinEndMinActiveHigh ? gpio_pulldown_t::GPIO_PULLDOWN_ENABLE : gpio_pulldown_t::GPIO_PULLDOWN_DISABLE;
            gpio_config(&io_conf);

            // Attach interrupt handler
            gpio_isr_handler_add(pinMinEndStop, &endstop_isr, (void*)&this->isr_data);
        }

        if (pinMaxEndStop != GPIO_NUM_NC) {
            gpio_config_t io_conf{};
            io_conf.intr_type = pinMaxEndStopActiveHigh
                                ? GPIO_INTR_POSEDGE
                                : GPIO_INTR_NEGEDGE;
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pin_bit_mask = (1ULL << pinMaxEndStop);
            io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;//pinEndMaxActiveHigh ? gpio_pullup_t::GPIO_PULLUP_DISABLE : gpio_pullup_t::GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;//pinEndMaxActiveHigh ? gpio_pulldown_t::GPIO_PULLDOWN_ENABLE : gpio_pulldown_t::GPIO_PULLDOWN_DISABLE;
            gpio_config(&io_conf);

            gpio_isr_handler_add(pinMaxEndStop, &endstop_isr, (void*)&this->isr_data);
        }
#elif defined(ESP8266)
        
#endif
        reset();
        stopDrive();
    }

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

    HALOperationResult Actuator::exec_drive_to_min(Device* device) {
        static_cast<Actuator*>(device)->stopDrive(); // direct call no vtable
        static_cast<Actuator*>(device)->driveToMin(); // direct call no vtable
#if HAS_REACTIVE_EXEC(ACTUATOR)
        static_cast<Actuator*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult Actuator::exec_drive_to_max(Device* device) {
        static_cast<Actuator*>(device)->stopDrive(); // direct call no vtable
        static_cast<Actuator*>(device)->driveToMax(); // direct call no vtable
#if HAS_REACTIVE_EXEC(ACTUATOR)
        static_cast<Actuator*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult Actuator::exec_stop(Device* device) {
        static_cast<Actuator*>(device)->stopDrive(); // direct call no vtable
#if HAS_REACTIVE_EXEC(ACTUATOR)
        static_cast<Actuator*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult Actuator::exec_reset(Device* device) {
        static_cast<Actuator*>(device)->reset();
        static_cast<Actuator*>(device)->stopDrive();
#if HAS_REACTIVE_EXEC(ACTUATOR)
        static_cast<Actuator*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    Device::Exec_FuncType Actuator::GetExec_Function(ZeroCopyString& zcFuncName) {
        if (zcFuncName.EqualsIC(F(DALHAL_DEVICE_ACTUATOR_CMD_CLOSE)) || zcFuncName.EqualsIC(F(DALHAL_DEVICE_ACTUATOR_CMD_TO_MIN))) {
            return exec_drive_to_min;
        } else if (zcFuncName.EqualsIC(F(DALHAL_DEVICE_ACTUATOR_CMD_OPEN)) || zcFuncName.EqualsIC(F(DALHAL_DEVICE_ACTUATOR_CMD_TO_MAX))) {
            return exec_drive_to_max;
        } else if (zcFuncName.EqualsIC(F(DALHAL_DEVICE_ACTUATOR_CMD_STOP))) {
            return exec_stop;
        } else if (zcFuncName.EqualsIC(F(DALHAL_DEVICE_ACTUATOR_CMD_RESET))) {
            return exec_reset;
        } else {
            return nullptr;
        }
    }

    HALOperationResult Actuator::exec(const ZeroCopyString& cmd) {
        if (cmd.EqualsIC(F(DALHAL_DEVICE_ACTUATOR_CMD_CLOSE)) || cmd.EqualsIC(F(DALHAL_DEVICE_ACTUATOR_CMD_TO_MIN))) {
            driveToMin();
        } else if (cmd.EqualsIC(F(DALHAL_DEVICE_ACTUATOR_CMD_OPEN)) || cmd.EqualsIC(F(DALHAL_DEVICE_ACTUATOR_CMD_TO_MAX))) {
            driveToMax();
        } else if (cmd.EqualsIC(F(DALHAL_DEVICE_ACTUATOR_CMD_STOP))) {
            stopDrive();
        } else if (cmd.EqualsIC(F(DALHAL_DEVICE_ACTUATOR_CMD_RESET))) {
            reset();
            stopDrive();   
        } else {
            return HALOperationResult::UnsupportedCommand;
        }
#if HAS_REACTIVE_EXEC(ACTUATOR)
        triggerExec();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult Actuator::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) { /*printf("\nSinglePulseOutput::write TEST\n");*/ return HALOperationResult::Success; }// test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;

        if (val.isUintOrInt() == false) {
            return HALOperationResult::WriteValueNotUintOrInt;
        }

        int32_t v = val.toInt();
        if (v != 0 && v != 1) {
            return HALOperationResult::InvalidArgument;
        }

        // Abort any previous motion
        stopDrive();

        if (v == 0) {
            driveToMin();
        } else {
            driveToMax();
        }
#if HAS_REACTIVE_WRITE(ACTUATOR)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult Actuator::read(const HALReadStringRequestValue& val) {
        if (val.cmd.EqualsIC(F("endstops"))) {
            val.out_value += "\"min\":";
            val.out_value += endMinActive() ? "true":"false";
            val.out_value += ',';
            val.out_value += "\"max\":";
            val.out_value += endMaxActive() ? "true":"false";

            return HALOperationResult::Success;
        } else {
            return HALOperationResult::UnsupportedCommand;
        }
    }

    HALOperationResult Actuator::read(HALValue& val) {
        if (state == State::TimeoutFault) {
            return HALOperationResult::Timeout;
        }
        bool startReached = endMinActive();
        bool stopReached = endMaxActive();
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
        return HALOperationResult::Success;
    }

    void Actuator::reset() {
        state = State::Idle;
        isr_data.location = Location::Unknown;
    }

    void Actuator::disableEndstopInterrupts() {
#if defined(ESP32)
        if (pinMinEndStop != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_disable(pinMinEndStop);
        }
        if (pinMaxEndStop != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_disable(pinMaxEndStop);
        }
#elif defined(ESP8266)

#endif
    }

    void Actuator::stopDrive() {
        if (state != State::TimeoutFault)
            state = State::Idle;
        motionStartMs = 0;
        disableEndstopInterrupts();
#if defined(ESP32)
        if (mode == DriveMode::HBridge) {
            gpio_set_level(pins.hbridge.a, 0);
            gpio_set_level(pins.hbridge.b, 0);
        } else if (mode == DriveMode::DirEnable) {
            gpio_set_level(pins.diren.enable, 0);
        }
#elif defined(ESP8266)

#endif
    }

    void Actuator::driveToMin() {
        if (endMinActive()) return;
        disableEndstopInterrupts();
        state = State::MovingToMin;
        isr_data.location = Location::Unknown;
#if defined(ESP32)
        if (mode == DriveMode::HBridge) {
            gpio_set_level(pins.hbridge.b, 0);
            gpio_set_level(pins.hbridge.a, 1);            
            configureISRData(pins.hbridge.a, GpioRegType::Clear);
        } else if (mode == DriveMode::DirEnable) {
            gpio_set_level(pins.diren.dir, 0);
            gpio_set_level(pins.diren.enable, 1);
            configureISRData(pins.diren.enable, GpioRegType::Clear);
        }
#elif defined(ESP8266)

#endif
        motionStartMs = millis();
#if defined(ESP32)
        //gpio_intr_disable(pinMaxEndStop); allready disabled in disableEndstopInterrupts
        if (pinMinEndStop != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_enable(pinMinEndStop);
        }
#elif defined(ESP8266)

#endif
    }

    void Actuator::driveToMax() {
        if (endMaxActive()) return;
        disableEndstopInterrupts();
        state = State::MovingToMax;
        isr_data.location = Location::Unknown;
#if defined(ESP32)
        if (mode == DriveMode::HBridge) {
            gpio_set_level(pins.hbridge.a, 0);
            gpio_set_level(pins.hbridge.b, 1);
            configureISRData(pins.hbridge.b, GpioRegType::Clear);
        } else if (mode == DriveMode::DirEnable) {
            gpio_set_level(pins.diren.dir, 0);
            gpio_set_level(pins.diren.enable, 1);
            configureISRData(pins.diren.enable, GpioRegType::Clear);
        }
#elif defined(ESP8266)

#endif
        motionStartMs = millis();
#if defined(ESP32)
        // gpio_intr_disable(pinMinEndStop); allready disabled in disableEndstopInterrupts
        if (pinMaxEndStop != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_enable(pinMaxEndStop);
        }
#elif defined(ESP8266)

#endif
    }

    bool Actuator::endMinActive() const {
#if defined(ESP32)
        // 1) Physical pin says active
        int level = (pinMinEndStop != gpio_num_t::GPIO_NUM_NC)?gpio_get_level(pinMinEndStop):0;
        bool pinActive = pinMinEndStopActiveHigh ? (level == 1) : (level == 0);

        if (pinActive) {
            return true;
        }

        // 2) Latched ISR confirmation
        if (isr_data.location == Location::Min) {
            return true;
        }

        return false;
#else
        return true;
#endif
    }

    bool Actuator::endMaxActive() const {
#if defined(ESP32)
        // 1) Physical pin says active
        int level = (pinMaxEndStop != gpio_num_t::GPIO_NUM_NC)?gpio_get_level(pinMaxEndStop):0;
        bool pinActive = pinMaxEndStopActiveHigh ? (level == 1) : (level == 0);

        if (pinActive) {
            return true;
        }
        // 2) Latched ISR confirmation
        if (isr_data.location == Location::Max) {
            return true;
        }

        return false;
#else
        return true;
#endif
    }

    String Actuator::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\",";
        if (mode == DriveMode::HBridge) {
            ret += "\"mode\":\"h-bridge\"";
            ret += ',';
            ret += "\"pin a\":";
            ret += std::to_string((int8_t)pins.hbridge.a).c_str();
            ret += ',';
            ret += "\"pin b\":";
            ret += std::to_string((int8_t)pins.hbridge.b).c_str();
        } else if (mode == DriveMode::DirEnable) {
            ret += "\"mode\":\"dir_enable\"";
            ret += ',';
            ret += "\"pin dir\":";
            ret += std::to_string((int8_t)pins.diren.dir).c_str();
            ret += ',';
            ret += "\"pin enable\":";
            ret += std::to_string((int8_t)pins.diren.enable).c_str();
        }
        
        ret += ',';
        ret += "\"pinEndMin\":";
        ret += std::to_string((int8_t)pinMinEndStop).c_str();
        ret += ',';
        ret += "\"pinEndMax\":";
        ret += std::to_string((int8_t)pinMaxEndStop).c_str();
        return ret;
    }

} // namespace DALHAL
