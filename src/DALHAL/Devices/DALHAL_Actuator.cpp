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

#include "DALHAL_Actuator.h"

#include "../Support/DALHAL_Logger.h"
#include "../Core/Device/DALHAL_JSON_Config_Defines.h"
#include "../Support/DALHAL_ArduinoJSON_ext.h"
#include "../Core/Manager/DALHAL_GPIO_Manager.h"

namespace DALHAL {
#if !defined(esp32c3) && !defined(esp32c6)
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
        
#if !defined(esp32c3) && !defined(esp32c6)
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

        // Disable only the triggering interrupt
        gpio_intr_disable(isr_data->gpio_currentPin);

        // signal to "main loop"
        isr_data->handled = true;
    }

    Actuator::Actuator(const JsonVariant &jsonObj, const char* type) : Device(type), state(State::Idle) {
        isr_data.location = Location::Unknown;
        isr_data.handled = false;
        const char* uidStr = GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID);
        uid = encodeUID(uidStr);

        // as we allready verified in VerifyJSON that the pin cfg is absolutely explicit defined
        // we only need to check any of the pins to exist to determine the mode
        if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_A)) {
            // H-bridge mode raw a/b pins defined
            pins.hbridge.a = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_A);
            pins.hbridge.b = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_B);
            mode = DriveMode::HBridge;
        } else if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_OPEN)) {
            // H-bridge mode open/close pins defined
            pins.hbridge.a = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_OPEN);
            pins.hbridge.b = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_CLOSE);
            mode = DriveMode::HBridge;
        } else if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_DIR)) {
            pins.diren.dir = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_DIR);
            pins.diren.enable = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_ENABLE);
            mode = DriveMode::DirEnable;
        } 
        // reserved for future modes
        if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP)) {
            pinMinEndStop = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP);
        } else {
            pinMinEndStop = gpio_num_t::GPIO_NUM_NC;
            printf("\r\nWarning %s is not set in json\r\n",DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP);
        }
        if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP)) {
            pinMaxEndStop = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP);
        } else {
            pinMaxEndStop = gpio_num_t::GPIO_NUM_NC;
            printf("\r\nWarning %s is not set in json\r\n",DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP);
        }

        if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP_ACTIVE_HIGH) && jsonObj[DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP_ACTIVE_HIGH].is<bool>()) {
            pinEndMinActiveHigh = jsonObj[DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP_ACTIVE_HIGH].as<bool>();
        } else {
            pinEndMinActiveHigh = true; // default
        }
        if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP_ACTIVE_HIGH) && jsonObj[DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP_ACTIVE_HIGH].is<bool>()) {
            pinEndMaxActiveHigh = jsonObj[DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP_ACTIVE_HIGH].as<bool>();
        } else {
            pinEndMaxActiveHigh = true; // default
        }

        if (jsonObj.containsKey("timeoutMs") && jsonObj["timeoutMs"].is<uint32_t>()) {
            timeoutMs = jsonObj["timeoutMs"].as<uint32_t>();
        } else {
            timeoutMs = 10000; // default 10 seconds
        }

        setup();
    }

    Actuator::~Actuator() {
        // FREE all used pins by setting them to INPUTS
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
    }

    bool Actuator::VerifyJSON(const JsonVariant &jsonObj) {
        bool anyError = false;

        bool have_pin_hbridge_a = jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_A);
        bool have_pin_hbridge_b = jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_B);
        bool have_pin_hbridge_open = jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_OPEN);
        bool have_pin_hbridge_close = jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_CLOSE);
        bool have_pin_dir = jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_DIR);
        bool have_pin_enable = jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_ENABLE);

        bool hbridge_mode_ab = have_pin_hbridge_a && have_pin_hbridge_b;
        bool hbridge_mode_ab_exor = have_pin_hbridge_a ^ have_pin_hbridge_b;
        bool hbridge_mode_open_close = have_pin_hbridge_open && have_pin_hbridge_close;
        bool hbridge_mode_open_close_exor = have_pin_hbridge_open ^ have_pin_hbridge_close;
        bool dir_enable_mode = have_pin_dir && have_pin_enable;
        bool dir_enable_mode_exor = have_pin_dir ^ have_pin_enable;

        
        const char* uidStr = GetAsConstChar(jsonObj, DALHAL_KEYNAME_UID); // used for eventual errors

        // Check for invalid configs
        if (hbridge_mode_ab_exor || hbridge_mode_open_close_exor) {
            GlobalLogger.Error(F("Incomplete H-Bridge pin config"), uidStr);
            SET_ERR_LOC("ACTUATOR_VJ_HBridge");
            anyError = true;
        }
        if (dir_enable_mode_exor) {
            GlobalLogger.Error(F("Incomplete Dir/Enable pin config"), uidStr);
            SET_ERR_LOC("ACTUATOR_VJ_DirEnable");
            anyError = true;
        }
        bool hbridge_invalid = hbridge_mode_ab && hbridge_mode_open_close;
        if (hbridge_invalid) {
            GlobalLogger.Error(F("Both h-bridge pin config options cannot be used at the same time"), uidStr);
            SET_ERR_LOC("ACTUATOR_VJ_HBridge");
            anyError = true;
        }
        bool any_hbridge_active = (hbridge_mode_ab || hbridge_mode_open_close) && !hbridge_invalid;
        
        int32_t pin_a_open_dir = -1; // set to unused, is either: a/open/dir
        int32_t pin_b_close_enable = -1; // set to unused, is either: b/close/enable
        int32_t pin_dir_enable_break = -1; // set to unused, is only: break in dir/enable mode
        int32_t pin_min_endstop = -1; // set to unused, is min endstop
        int32_t pin_max_endstop = -1; // set to unused, is max endstop
        
        // Determine mode
        if (any_hbridge_active && !dir_enable_mode) {
            // this defines a valid mode
            //mode = DriveMode::HBridge;
            if (hbridge_mode_ab) {
                pin_a_open_dir = GetAsINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_A);
                pin_b_close_enable = GetAsINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_B);
            } else if (hbridge_mode_open_close) {
                pin_a_open_dir = GetAsINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_OPEN);
                pin_b_close_enable = GetAsINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_CLOSE);
            } 
            // reserverved for more modes in the future
            // could have open/close to make more sense in door/fluid operations

        } else if (!any_hbridge_active && dir_enable_mode) {
            // this defines a valid mode
            //mode = DriveMode::DirEnable;
            pin_a_open_dir = GetAsINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_DIR);
            pin_b_close_enable = GetAsINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_ENABLE);
            if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_BREAK)) {
                pin_dir_enable_break = GetAsINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_BREAK);
            }
        } else {
            GlobalLogger.Error(F("Ambiguous or invalid motor driver config"), uidStr);
            SET_ERR_LOC("ACTUATOR_VJ_AmbiguousMode");
            anyError = true;
        }

        if ((pin_a_open_dir != -1) && GPIO_manager::CheckIfPinAvailableAndReserve(pin_a_open_dir, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)) == false) {
            SET_ERR_LOC("Actuator_VJ_pin_a_open_dir");
            anyError = true;
        }
        if ((pin_b_close_enable != -1) && GPIO_manager::CheckIfPinAvailableAndReserve(pin_b_close_enable, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)) == false) {
            SET_ERR_LOC("Actuator_VJ_pin_b_close_enable");
            anyError = true;
        }
        if ((pin_dir_enable_break != -1) && GPIO_manager::CheckIfPinAvailableAndReserve(pin_dir_enable_break, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)) == false) {
            SET_ERR_LOC("Actuator_VJ_pin_dir_enable_break");
            anyError = true;
        }

        if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP)) {
            int32_t pin_min_endstop = GetAsINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP);
            if (GPIO_manager::CheckIfPinAvailableAndReserve(pin_min_endstop, static_cast<uint8_t>(GPIO_manager::PinFunc::IN)) == false) {
                SET_ERR_LOC("Actuator_VJ_" DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP);
                anyError = true;
            }
        }
        if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP)) {
            int32_t pin_max_endstop = GetAsINT32(jsonObj, DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP);
            if (GPIO_manager::CheckIfPinAvailableAndReserve(pin_max_endstop, static_cast<uint8_t>(GPIO_manager::PinFunc::IN)) == false) {
                SET_ERR_LOC("Actuator_VJ_" DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP);
                anyError = true;
            }
        }
        

        if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP_ACTIVE_HIGH) && jsonObj[DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP_ACTIVE_HIGH].is<bool>() == false) {
            GlobalLogger.Error(F(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP_ACTIVE_HIGH " is not a bool"), uidStr);
            SET_ERR_LOC("Actuator_VJ_" DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MIN_END_STOP_ACTIVE_HIGH);
            anyError = true;
        }

        if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP_ACTIVE_HIGH) && jsonObj[DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP_ACTIVE_HIGH].is<bool>() == false) {
            GlobalLogger.Error(F(DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP_ACTIVE_HIGH " is not a bool"), uidStr);
            SET_ERR_LOC("Actuator_VJ_" DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_MAX_END_STOP_ACTIVE_HIGH);
            anyError = true;
        }

        if (jsonObj.containsKey(DALHAL_DEVICE_ACTUATOR_CFG_NAME_TIMEOUT_MS) && jsonObj[DALHAL_DEVICE_ACTUATOR_CFG_NAME_TIMEOUT_MS].is<uint32_t>() == false) {
            GlobalLogger.Error(F(DALHAL_DEVICE_ACTUATOR_CFG_NAME_TIMEOUT_MS " is not uint32"), uidStr);
            SET_ERR_LOC("Actuator_VJ_" DALHAL_DEVICE_ACTUATOR_CFG_NAME_TIMEOUT_MS);
            anyError = true;
        }

        return anyError == false;
    }

    Device* Actuator::Create(const JsonVariant &jsonObj, const char* type) {
        return new Actuator(jsonObj, type);
    }

    void Actuator::setup() {

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
            io_conf.intr_type = pinEndMinActiveHigh
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
            io_conf.intr_type = pinEndMaxActiveHigh
                                ? GPIO_INTR_POSEDGE
                                : GPIO_INTR_NEGEDGE;
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pin_bit_mask = (1ULL << pinMaxEndStop);
            io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;//pinEndMaxActiveHigh ? gpio_pullup_t::GPIO_PULLUP_DISABLE : gpio_pullup_t::GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;//pinEndMaxActiveHigh ? gpio_pulldown_t::GPIO_PULLDOWN_ENABLE : gpio_pulldown_t::GPIO_PULLDOWN_DISABLE;
            gpio_config(&io_conf);

            gpio_isr_handler_add(pinMaxEndStop, &endstop_isr, (void*)&this->isr_data);
        }
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
            } else if (state == State::MovingToMax) {
                isr_data.location = Location::Max;
            }
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
        return HALOperationResult::Success;
    }

    HALOperationResult Actuator::exec_drive_to_max(Device* device) {
        static_cast<Actuator*>(device)->stopDrive(); // direct call no vtable
        static_cast<Actuator*>(device)->driveToMax(); // direct call no vtable
        return HALOperationResult::Success;
    }

    HALOperationResult Actuator::exec_stop(Device* device) {
        static_cast<Actuator*>(device)->stopDrive(); // direct call no vtable
        return HALOperationResult::Success;
    }

    HALOperationResult Actuator::exec_reset(Device* device) {
        auto* d = static_cast<Actuator*>(device);
        d->reset();
        d->stopDrive();
        return HALOperationResult::Success;
    }


    Device::Exec_FuncType Actuator::GetExec_Function(ZeroCopyString& zcFuncName) {
        if (zcFuncName == DALHAL_DEVICE_ACTUATOR_CMD_CLOSE || zcFuncName == DALHAL_DEVICE_ACTUATOR_CMD_TO_MIN) {
            return exec_drive_to_min;
        } else if (zcFuncName == DALHAL_DEVICE_ACTUATOR_CMD_OPEN || zcFuncName == DALHAL_DEVICE_ACTUATOR_CMD_TO_MAX) {
            return exec_drive_to_max;
        } else if (zcFuncName == DALHAL_DEVICE_ACTUATOR_CMD_STOP) {
            return exec_stop;
        } else if (zcFuncName == DALHAL_DEVICE_ACTUATOR_CMD_RESET) {
            return exec_reset;
        } else {
            return nullptr;
        }
    }

    HALOperationResult Actuator::exec(const ZeroCopyString& cmd) {
        if (cmd == DALHAL_DEVICE_ACTUATOR_CMD_CLOSE || cmd == DALHAL_DEVICE_ACTUATOR_CMD_TO_MIN) {
            driveToMin();
            return HALOperationResult::Success;
        } else if (cmd == DALHAL_DEVICE_ACTUATOR_CMD_OPEN || cmd == DALHAL_DEVICE_ACTUATOR_CMD_TO_MAX) {
            driveToMax();
            return HALOperationResult::Success;
        } else if (cmd == DALHAL_DEVICE_ACTUATOR_CMD_STOP) {
            stopDrive();
            return HALOperationResult::Success;
        } else if (cmd == DALHAL_DEVICE_ACTUATOR_CMD_RESET) {
            reset();
            stopDrive();
            return HALOperationResult::Success;
        }
        return HALOperationResult::UnsupportedCommand;
    }


    HALOperationResult Actuator::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) { /*printf("\nSinglePulseOutput::write TEST\n");*/ return HALOperationResult::Success; }// test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;

        if (val.isUintOrInt() == false) {
            return HALOperationResult::WriteValueNotUintOrInt;
        }

        int32_t v = val.asInt();
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

        return HALOperationResult::Success;
    }

    HALOperationResult Actuator::read(const HALReadStringRequestValue& val) {
        if (val.cmd == "endstops") {
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
        if (pinMinEndStop != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_disable(pinMinEndStop);
        }
        if (pinMaxEndStop != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_disable(pinMaxEndStop);
        }
    }

    


    void Actuator::stopDrive() {
        if (state != State::TimeoutFault)
            state = State::Idle;
        motionStartMs = 0;
        disableEndstopInterrupts();

        if (mode == DriveMode::HBridge) {
            gpio_set_level(pins.hbridge.a, 0);
            gpio_set_level(pins.hbridge.b, 0);
        } else if (mode == DriveMode::DirEnable) {
            gpio_set_level(pins.diren.enable, 0);
        }
    }

    void Actuator::driveToMin() {
        if (endMinActive()) return;
        disableEndstopInterrupts();
        state = State::MovingToMin;
        isr_data.location = Location::Unknown;
        if (mode == DriveMode::HBridge) {
            gpio_set_level(pins.hbridge.b, 0);
            gpio_set_level(pins.hbridge.a, 1);            
            configureISRData(pins.hbridge.a, GpioRegType::Clear);
        } else if (mode == DriveMode::DirEnable) {
            gpio_set_level(pins.diren.dir, 0);
            gpio_set_level(pins.diren.enable, 1);
            configureISRData(pins.diren.enable, GpioRegType::Clear);
        }
        motionStartMs = millis();
        //gpio_intr_disable(pinMaxEndStop); allready disabled in disableEndstopInterrupts
        if (pinMinEndStop != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_enable(pinMinEndStop);
        }
    }

    void Actuator::driveToMax() {
        if (endMaxActive()) return;
        disableEndstopInterrupts();
        state = State::MovingToMax;
        isr_data.location = Location::Unknown;
        if (mode == DriveMode::HBridge) {
            gpio_set_level(pins.hbridge.a, 0);
            gpio_set_level(pins.hbridge.b, 1);
            configureISRData(pins.hbridge.b, GpioRegType::Clear);
        } else if (mode == DriveMode::DirEnable) {
            gpio_set_level(pins.diren.dir, 0);
            gpio_set_level(pins.diren.enable, 1);
            configureISRData(pins.diren.enable, GpioRegType::Clear);
        }
        motionStartMs = millis();
        // gpio_intr_disable(pinMinEndStop); allready disabled in disableEndstopInterrupts
        if (pinMaxEndStop != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_enable(pinMaxEndStop);
        }
    }

    

    bool Actuator::endMinActive() const {
        // 1) Physical pin says active
        int level = (pinMinEndStop != gpio_num_t::GPIO_NUM_NC)?gpio_get_level(pinMinEndStop):0;
        bool pinActive = pinEndMinActiveHigh ? (level == 1) : (level == 0);

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
        // 1) Physical pin says active
        int level = (pinMaxEndStop != gpio_num_t::GPIO_NUM_NC)?gpio_get_level(pinMaxEndStop):0;
        bool pinActive = pinEndMaxActiveHigh ? (level == 1) : (level == 0);

        if (pinActive) {
            return true;
        }
        // 2) Latched ISR confirmation
        if (isr_data.location == Location::Max) {
            return true;
        }

        return false;
    }

    String Actuator::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        if (mode == DriveMode::HBridge) {
            ret += "\"mode\":\"h-bridge\"";
            ret += ',';
            ret += "\"pin a\":";
            ret += std::to_string(pins.hbridge.a).c_str();
            ret += ',';
            ret += "\"pin b\":";
            ret += std::to_string(pins.hbridge.b).c_str();
        } else if (mode == DriveMode::DirEnable) {
            ret += "\"mode\":\"dir_enable\"";
            ret += ',';
            ret += "\"pin dir\":";
            ret += std::to_string(pins.diren.dir).c_str();
            ret += ',';
            ret += "\"pin enable\":";
            ret += std::to_string(pins.diren.enable).c_str();
        }
        
        ret += ',';
        ret += "\"pinEndMin\":";
        ret += std::to_string(pinMinEndStop).c_str();
        ret += ',';
        ret += "\"pinEndMax\":";
        ret += std::to_string(pinMaxEndStop).c_str();
        return ret;
    }

} // namespace DALHAL
