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
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include "DALHAL_LatchingRelay_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>

namespace DALHAL {

    constexpr Registry::DefineBase LatchingRelay::RegistryDefine = {
        Create,
        &JsonSchema::LatchingRelay,
        DALHAL_REACTIVE_EVENT_TABLE(RELAY_LATCHING)
    };

    Device* LatchingRelay::Create(DeviceCreateContext& context) {
        return new LatchingRelay(context);
    }

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

void LatchingRelay::configureISRData(gpio_num_t& somePin, GpioRegType regType) {
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

    void IRAM_ATTR LatchingRelay::endstop_isr(void* arg) {
        LatchingRelay::ISR_DATA* isr_data = static_cast<LatchingRelay::ISR_DATA*>(arg);

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

    LatchingRelay::LatchingRelay(DeviceCreateContext& context) : LatchingRelay_DeviceBase(context.deviceType), state(State::Idle) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
        isr_data.location = Location::Unknown;
        isr_data.handled = false;
        const char* uidStr = GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID);
        uid = encodeUID(uidStr);

        // as we allready verified in VerifyJSON that the pin cfg is absolutely explicit defined
        // we only need to check any of the pins to exist to determine the mode
        if (jsonObj.containsKey(DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_A)) {
            // H-bridge mode raw a/b pins defined
            pins.direct.a = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_A);
            pins.direct.b = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_B);
            mode = DriveMode::Direct;
        } else if (jsonObj.containsKey(DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_SET)) {
            // H-bridge mode open/close pins defined
            pins.direct.a = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_SET);
            pins.direct.b = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_RESET);
            mode = DriveMode::Direct;
        } else if (jsonObj.containsKey(DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_DIR)) {
            pins.data_enable.data = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_DIR);
            pins.data_enable.enable = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_ENABLE);
            mode = DriveMode::DataEnable;
        } 
        // reserved for future modes
        if (jsonObj.containsKey(DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_SET_STATE)) {
            const JsonVariant& endStop = jsonObj[DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_SET_STATE];
            pinFeedbackSet = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_COMMON_CFG_NAME_PIN);
            if (endStop.containsKey(DALHAL_COMMON_CFG_NAME_PIN_ACTIVE_HIGH)) {
                pinFeedbackSetActiveHigh = endStop[DALHAL_COMMON_CFG_NAME_PIN_ACTIVE_HIGH];
            } else {
                pinFeedbackSetActiveHigh = true;
            }
        } else {
            pinFeedbackSet = gpio_num_t::GPIO_NUM_NC;
            pinFeedbackSetActiveHigh = true;
        }

        if (jsonObj.containsKey(DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_RESET_STATE)) {
            const JsonVariant& endStop = jsonObj[DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_RESET_STATE];
            pinFeedbackReset = (gpio_num_t)GetAsUINT32(jsonObj, DALHAL_COMMON_CFG_NAME_PIN);
            if (endStop.containsKey(DALHAL_COMMON_CFG_NAME_PIN_ACTIVE_HIGH)) {
                pinFeedbackResetActiveHigh = endStop[DALHAL_COMMON_CFG_NAME_PIN_ACTIVE_HIGH];
            } else {
                pinFeedbackResetActiveHigh = true;
            }
        } else {
            pinFeedbackReset = gpio_num_t::GPIO_NUM_NC;
            pinFeedbackResetActiveHigh = true;
        }

        if (jsonObj.containsKey("timeoutMs") && jsonObj["timeoutMs"].is<uint32_t>()) {
            timeoutMs = jsonObj["timeoutMs"].as<uint32_t>();
        } else {
            timeoutMs = 10000; // default 10 seconds
        }

        setup();
    }

    LatchingRelay::~LatchingRelay() {
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
    }

    void LatchingRelay::setup() {

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
        driveToReset();
        stopDrive();
    }

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

    

    HALOperationResult LatchingRelay::exec_drive_to_reset(Device* device) {
        static_cast<LatchingRelay*>(device)->stopDrive(); // direct call no vtable
        static_cast<LatchingRelay*>(device)->driveToReset(); // direct call no vtable
#if HAS_REACTIVE_EXEC(RELAY_LATCHING)
        static_cast<LatchingRelay*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult LatchingRelay::exec_drive_to_set(Device* device) {
        static_cast<LatchingRelay*>(device)->stopDrive(); // direct call no vtable
        static_cast<LatchingRelay*>(device)->driveToSet(); // direct call no vtable
#if HAS_REACTIVE_EXEC(RELAY_LATCHING)
        static_cast<LatchingRelay*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult LatchingRelay::exec_stop(Device* device) {
        static_cast<LatchingRelay*>(device)->stopDrive(); // direct call no vtable
#if HAS_REACTIVE_EXEC(RELAY_LATCHING)
        static_cast<LatchingRelay*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult LatchingRelay::exec_resetMode(Device* device) {
        static_cast<LatchingRelay*>(device)->driveToReset();
        static_cast<LatchingRelay*>(device)->stopDrive();
#if HAS_REACTIVE_EXEC(RELAY_LATCHING)
        static_cast<LatchingRelay*>(device)->triggerExec();
#endif
        return HALOperationResult::Success;
    }


    Device::Exec_FuncType LatchingRelay::GetExec_Function(ZeroCopyString& zcFuncName) {
        if (zcFuncName == DALHAL_DEVICE_LATCHING_RELAY_CMD_CLOSE || zcFuncName == DALHAL_DEVICE_LATCHING_RELAY_CMD_TO_MIN) {
            return exec_drive_to_reset;
        } else if (zcFuncName == DALHAL_DEVICE_LATCHING_RELAY_CMD_OPEN || zcFuncName == DALHAL_DEVICE_LATCHING_RELAY_CMD_TO_MAX) {
            return exec_drive_to_set;
        } else if (zcFuncName == DALHAL_DEVICE_LATCHING_RELAY_CMD_STOP) {
            return exec_stop;
        } else if (zcFuncName == DALHAL_DEVICE_LATCHING_RELAY_CMD_RESET) {
            return exec_resetMode;
        } else {
            return nullptr;
        }
    }

    HALOperationResult LatchingRelay::exec(const ZeroCopyString& cmd) {
        if (cmd == DALHAL_DEVICE_LATCHING_RELAY_CMD_CLOSE || cmd == DALHAL_DEVICE_LATCHING_RELAY_CMD_TO_MIN) {
            driveToReset();

        } else if (cmd == DALHAL_DEVICE_LATCHING_RELAY_CMD_OPEN || cmd == DALHAL_DEVICE_LATCHING_RELAY_CMD_TO_MAX) {
            driveToSet();

        } else if (cmd == DALHAL_DEVICE_LATCHING_RELAY_CMD_STOP) {
            stopDrive();

        } else if (cmd == DALHAL_DEVICE_LATCHING_RELAY_CMD_RESET) {
            resetMode();
            stopDrive();
            
        } else {
            return HALOperationResult::UnsupportedCommand;
        }
#if HAS_REACTIVE_EXEC(RELAY_LATCHING)
        triggerExec();
#endif
        return HALOperationResult::Success;
    }


    HALOperationResult LatchingRelay::write(const HALValue& val) {
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
            driveToReset();
        } else {
            driveToSet();
        }
#if HAS_REACTIVE_WRITE(ACTUATOR)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult LatchingRelay::read(const HALReadStringRequestValue& val) {
        if (val.cmd == "endstops") {
            val.out_value += "\"min\":";
            val.out_value += resetActive() ? "true":"false";
            val.out_value += ',';
            val.out_value += "\"max\":";
            val.out_value += setActive() ? "true":"false";
#if HAS_REACTIVE(RELAY_LATCHING, READ)
        triggerRead();
#endif
            return HALOperationResult::Success;
        } else {
            return HALOperationResult::UnsupportedCommand;
        }
    }

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
        if (pinFeedbackReset != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_disable(pinFeedbackReset);
        }
        if (pinFeedbackSet != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_disable(pinFeedbackSet);
        }
    }

    void LatchingRelay::stopDrive() {
        if (state != State::TimeoutFault)
            state = State::Idle;
        motionStartMs = 0;
        disableFeedbackSignalInterrupts();

        if (mode == DriveMode::Direct) {
            gpio_set_level(pins.direct.a, 0);
            gpio_set_level(pins.direct.b, 0);
        } else if (mode == DriveMode::DataEnable) {
            gpio_set_level(pins.data_enable.enable, 0);
        }
    }

    void LatchingRelay::driveToReset() {
        if (resetActive()) return;
        disableFeedbackSignalInterrupts();
        state = State::DrivingReset;
        isr_data.location = Location::Unknown;
        if (mode == DriveMode::Direct) {
            gpio_set_level(pins.direct.b, 0);
            gpio_set_level(pins.direct.a, 1);            
            configureISRData(pins.direct.a, GpioRegType::Clear);
        } else if (mode == DriveMode::DataEnable) {
            gpio_set_level(pins.data_enable.data, 0);
            gpio_set_level(pins.data_enable.enable, 1);
            configureISRData(pins.data_enable.enable, GpioRegType::Clear);
        }
        motionStartMs = millis();
        //gpio_intr_disable(pinMaxEndStop); allready disabled in disableEndstopInterrupts
        if (pinFeedbackReset != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_enable(pinFeedbackReset);
        }
    }

    void LatchingRelay::driveToSet() {
        if (setActive()) return;
        disableFeedbackSignalInterrupts();
        state = State::DrivingSet;
        isr_data.location = Location::Unknown;
        if (mode == DriveMode::Direct) {
            gpio_set_level(pins.direct.a, 0);
            gpio_set_level(pins.direct.b, 1);
            configureISRData(pins.direct.b, GpioRegType::Clear);
        } else if (mode == DriveMode::DataEnable) {
            gpio_set_level(pins.data_enable.data, 0);
            gpio_set_level(pins.data_enable.enable, 1);
            configureISRData(pins.data_enable.enable, GpioRegType::Clear);
        }
        motionStartMs = millis();
        // gpio_intr_disable(pinMinEndStop); allready disabled in disableEndstopInterrupts
        if (pinFeedbackSet != gpio_num_t::GPIO_NUM_NC) {
            gpio_intr_enable(pinFeedbackSet);
        }
    }

    bool LatchingRelay::resetActive() const {
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
    }

    bool LatchingRelay::setActive() const {
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
    }

    String LatchingRelay::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\",";
        if (mode == DriveMode::Direct) {
            ret += "\"mode\":\"Direct\"";
            ret += ',';
            ret += "\"pin a\":";
            ret += std::to_string(pins.direct.a).c_str();
            ret += ',';
            ret += "\"pin b\":";
            ret += std::to_string(pins.direct.b).c_str();
        } else if (mode == DriveMode::DataEnable) {
            ret += "\"mode\":\"data_enable\"";
            ret += ',';
            ret += "\"pin dir\":";
            ret += std::to_string(pins.data_enable.data).c_str();
            ret += ',';
            ret += "\"pin enable\":";
            ret += std::to_string(pins.data_enable.enable).c_str();
        }
        
        ret += ',';
        ret += "\"pinEndMin\":";
        ret += std::to_string(pinFeedbackReset).c_str();
        ret += ',';
        ret += "\"pinEndMax\":";
        ret += std::to_string(pinFeedbackSet).c_str();
        return ret;
    }

} // namespace DALHAL
