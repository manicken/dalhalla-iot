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
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"
#include "HAL_JSON_DeviceTypesRegistry.h"

#define HAL_JSON_DEVICE_DCSERVO_CMD_FORWARD "fw"
#define HAL_JSON_DEVICE_DCSERVO_CMD_BACKWARD "bw"
#define HAL_JSON_DEVICE_DCSERVO_CMD_STOP "stop"
#define HAL_JSON_DEVICE_DCSERVO_CMD_RESET "reset"

namespace HAL_JSON {

class Device_DCServo : public Device {
public:
    enum class State : uint32_t {
        Idle,
        MovingToMin,
        MovingToMax,
        TimeoutFault
    };
    enum class Location : int32_t { Unknown = -1, Min = 0, Max = 1 };

    static bool VerifyJSON(const JsonVariant &jsonObj);
    static Device* Create(const JsonVariant &jsonObj, const char* type);
    static constexpr DeviceRegistryDefine RegistryDefine = {
        UseRootUID::Mandatory,
        Create,
        VerifyJSON
    };

    Device_DCServo(const JsonVariant &jsonObj, const char* type);
    ~Device_DCServo();

    virtual void begin() override;
    virtual void loop() override;

    virtual HALOperationResult write(const HALValue& val) override;
    virtual HALOperationResult read(HALValue& val) override;

    static HALOperationResult exec_fw(Device* device);
    static HALOperationResult exec_bw(Device* device);
    static HALOperationResult exec_stop(Device* device);
    static HALOperationResult exec_reset(Device* device);

    virtual Exec_FuncType GetExec_Function(ZeroCopyString& zcFuncName);
    /** Executes a device action with a provided command string, only used when doing remote cmd:s, i.e. not used by script. */
    virtual HALOperationResult exec(const ZeroCopyString& cmd);

    virtual String ToString() override;

private:
    void reset();
    void stopMotor();
    void driveForward();
    void driveBackward();
    bool endMinActive() const;
    bool endMaxActive() const;

private:
    uint8_t pinForward;
    uint8_t pinBackward;
    uint8_t pinEndMin;
    uint8_t pinEndMax;

    bool pinEndMinActiveHigh;
    bool pinEndMaxActiveHigh;

    State state;
    Location location = Location::Unknown;

    uint32_t motionStartMs;
    uint32_t timeoutMs;

};

} // namespace HAL_JSON
