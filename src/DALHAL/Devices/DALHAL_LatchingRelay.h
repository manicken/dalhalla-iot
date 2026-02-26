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
#include "../Core/DALHAL_Device.h"
#include "DALHAL_DeviceTypesRegistry.h"

// for raw h-bridge control using forward and backward pins
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_A        "pinA"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_B        "pinB"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_OPEN     "pinOpen"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_CLOSE    "pinClose"
// for dir/enable/(optional break) pin mode
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_DIR      "pinDir"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_ENABLE   "pinEnable"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_BREAK    "pinBreak"

// 
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_MIN_END_STOP "pinMinEndStop"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_MAX_END_STOP "pinMaxEndStop"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_MIN_END_STOP_ACTIVE_HIGH "pinMinEndStopActiveHigh"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_MAX_END_STOP_ACTIVE_HIGH "pinMaxEndStopActiveHigh"

#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_TIMEOUT_MS "timeoutMs"

#define DALHAL_DEVICE_LATCHING_RELAY_CMD_OPEN   "open"
#define DALHAL_DEVICE_LATCHING_RELAY_CMD_CLOSE  "close"
#define DALHAL_DEVICE_LATCHING_RELAY_CMD_TO_MIN "toMin"
#define DALHAL_DEVICE_LATCHING_RELAY_CMD_TO_MAX "toMax"
#define DALHAL_DEVICE_LATCHING_RELAY_CMD_STOP   "stop"
#define DALHAL_DEVICE_LATCHING_RELAY_CMD_RESET  "reset"

namespace DALHAL {

class LatchingRelay : public Device {
public:
    static void IRAM_ATTR endstop_isr(void* arg);
    
    enum class State : uint32_t {
        Idle,
        DrivingReset,
        DrivingSet,
        TimeoutFault
    };
    enum class Location : int32_t { Unknown = -1, Reset = 0, Set = 1 };

    static bool VerifyJSON(const JsonVariant &jsonObj);
    static Device* Create(const JsonVariant &jsonObj, const char* type);
    static constexpr DeviceRegistryDefine RegistryDefine = {
        UseRootUID::Mandatory,
        Create,
        VerifyJSON
    };

    LatchingRelay(const JsonVariant &jsonObj, const char* type);
    ~LatchingRelay();

    void setup();
    virtual void loop() override;

    virtual HALOperationResult write(const HALValue& val) override;
    virtual HALOperationResult read(HALValue& val) override;

    virtual HALOperationResult read(const HALReadStringRequestValue& val);

    static HALOperationResult exec_drive_to_reset(Device* device);
    static HALOperationResult exec_drive_to_set(Device* device);
    static HALOperationResult exec_stop(Device* device);
    static HALOperationResult exec_resetMode(Device* device);

    virtual Exec_FuncType GetExec_Function(ZeroCopyString& zcFuncName);
    /** Executes a device action with a provided command string, only used when doing remote cmd:s, i.e. not used by script. */
    virtual HALOperationResult exec(const ZeroCopyString& cmd);

    virtual String ToString() override;

private:
    union DrivePins {
        struct { gpio_num_t a, b; } hbridge;
        struct { gpio_num_t data, enable; } data_enable;
    };
    enum class DriveMode : uint8_t {
        HBridge,      // set / reset
        DataEnable     // Data + Enable
    };
    enum class GpioRegType {
        Set,
        Clear
    };

    void resetMode();
    void stopDrive();
    void driveToReset();
    void driveToSet();
    bool resetActive() const;
    bool setActive() const;

    void disableFeedbackSignalInterrupts();
    void configureISRData(gpio_num_t& somePin, GpioRegType regType);

public:

    struct ISR_DATA {
        volatile gpio_num_t gpio_currentPin = gpio_num_t::GPIO_NUM_NC;
        void (* volatile gpio_reg_func)(uint32_t) = nullptr;
        volatile uint32_t gpio_currentActivePinMask = 0;
        volatile bool handled = false;
        volatile bool driveOn = false;
        volatile Location location = Location::Unknown;
    };

private:
    ISR_DATA isr_data;

    State state = State::Idle;

    DrivePins pins;
    DriveMode mode;

    gpio_num_t pinFeedbackReset;
    gpio_num_t pinFeedbackSet;

    bool pinFeedbackResetActiveHigh;
    bool pinFeedbackSetActiveHigh;

    uint32_t motionStartMs;
    uint32_t timeoutMs;

};

} // namespace DALHAL
