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

#include "HAL_JSON_DCServo.h"

namespace HAL_JSON {

    Device_DCServo::Device_DCServo(const JsonVariant &jsonObj, const char* type) : Device(type), state(State::Idle), location(Location::Unknown) {
        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);

        pinForward = GetAsUINT8(jsonObj, "pinForward");
        pinBackward = GetAsUINT8(jsonObj, "pinBackward");
        pinEndMax = GetAsUINT8(jsonObj, "pinEndMax");
        pinEndMin = GetAsUINT8(jsonObj, "pinEndMin");

        if (jsonObj.containsKey("pinEndMinActiveHigh") && jsonObj["pinEndMinActiveHigh"].is<bool>()) {
            pinEndMinActiveHigh = jsonObj["pinEndMinActiveHigh"].as<bool>();
        } else {
            pinEndMinActiveHigh = true; // default
        }
        if (jsonObj.containsKey("pinEndMaxActiveHigh") && jsonObj["pinEndMaxActiveHigh"].is<bool>()) {
            pinEndMaxActiveHigh = jsonObj["pinEndMaxActiveHigh"].as<bool>();
        } else {
            pinEndMaxActiveHigh = true; // default
        }

        if (jsonObj.containsKey("timeoutMs") && jsonObj["timeoutMs"].is<uint32_t>()) {
            timeoutMs = jsonObj["timeoutMs"].as<uint32_t>();
        } else {
            timeoutMs = 10000; // default 10 seconds
        }


        pinMode(pinForward, OUTPUT);
        pinMode(pinBackward, OUTPUT);

        if (pinEndMinActiveHigh) {
            pinMode(pinEndMin, INPUT_PULLDOWN);
        } else {
            pinMode(pinEndMin, INPUT_PULLUP);
        }

        if (pinEndMaxActiveHigh) {
            pinMode(pinEndMax, INPUT_PULLDOWN);
        } else {
            pinMode(pinEndMax, INPUT_PULLUP);
        }
    }

    Device_DCServo::~Device_DCServo() {
        pinMode(pinForward, INPUT); // "free" the pin
        pinMode(pinBackward, INPUT); // "free" the pin
        pinMode(pinEndMin, INPUT);
        pinMode(pinEndMax, INPUT);
    }

    bool Device_DCServo::VerifyJSON(const JsonVariant &jsonObj) {

        uint8_t pinForward = GetAsUINT8(jsonObj, "pinForward");
        uint8_t pinBackward = GetAsUINT8(jsonObj, "pinBackward");
        uint8_t pinEndMax = GetAsUINT8(jsonObj, "pinEndMax");
        uint8_t pinEndMin = GetAsUINT8(jsonObj, "pinEndMin");

        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);

        bool anyError = false;

        if (GPIO_manager::CheckIfPinAvailableAndReserve(pinForward, static_cast<uint8_t>(GPIO_manager::PinMode::OUT)) == false) {
            SET_ERR_LOC("DCServo_VJ_pinForward");
            anyError = true;
        }
        if (GPIO_manager::CheckIfPinAvailableAndReserve(pinBackward, static_cast<uint8_t>(GPIO_manager::PinMode::OUT)) == false) {
            SET_ERR_LOC("DCServo_VJ_pinBackward");
            anyError = true;
        }
        if (GPIO_manager::CheckIfPinAvailableAndReserve(pinEndMax, static_cast<uint8_t>(GPIO_manager::PinMode::IN)) == false) {
            SET_ERR_LOC("DCServo_VJ_pinEndMax");
            anyError = true;
        }
        if (GPIO_manager::CheckIfPinAvailableAndReserve(pinEndMin, static_cast<uint8_t>(GPIO_manager::PinMode::IN)) == false) {
            SET_ERR_LOC("DCServo_VJ_pinEndMin");
            anyError = true;
        }

        if (jsonObj.containsKey("pinEndMinActiveHigh") && jsonObj["pinEndMinActiveHigh"].is<bool>() == false) {
            GlobalLogger.Error(F("pinEndMinActiveHigh is not a bool"), uidStr);
            SET_ERR_LOC("DCServo_VJ_pinEndMinActiveHigh");
            anyError = true;
        }

        if (jsonObj.containsKey("pinEndMaxActiveHigh") && jsonObj["pinEndMaxActiveHigh"].is<bool>() == false) {
            GlobalLogger.Error(F("pinEndMaxActiveHigh is not a bool"), uidStr);
            SET_ERR_LOC("DCServo_VJ_pinEndMaxActiveHigh");
            anyError = true;
        }

        if (jsonObj.containsKey("timeoutMs") && jsonObj["timeoutMs"].is<uint32_t>() == false) {
            GlobalLogger.Error(F("timeoutMs is not uint32"), uidStr);
            SET_ERR_LOC("DCServo_VJ_timeoutMs");
            anyError = true;
        }


        return anyError == false;
    }

    Device* Device_DCServo::Create(const JsonVariant &jsonObj, const char* type) {
        return new Device_DCServo(jsonObj, type);
    }

    void Device_DCServo::begin() {
        pinMode(pinForward, OUTPUT);
        pinMode(pinBackward, OUTPUT);

        pinMode(pinEndMin, INPUT);
        pinMode(pinEndMax, INPUT);

        stopMotor();
    }

    void Device_DCServo::loop() {
        if (state == State::Idle) return;
        bool moving = false;
        switch (state) {

            case State::MovingToMin:
                if (endMinActive()) {
                    stopMotor();
                    location = Location::Min;
                    return;
                }
                moving = true;
                break;

            case State::MovingToMax:
                if (endMaxActive()) {
                    stopMotor();
                    location = Location::Max;
                    return;
                }
                moving = true;
                break;;
        }

        if (moving) {
            if ((millis() - motionStartMs) > timeoutMs) {
                stopMotor();
                state = State::TimeoutFault;
                location = Location::Unknown;
                GlobalLogger.Error(F("DCServo motion timeout"), decodeUID(uid).c_str());
                return;
            }
        }
    }

    HALOperationResult Device_DCServo::exec_fw(Device* device) {
        static_cast<Device_DCServo*>(device)->stopMotor(); // direct call no vtable
        static_cast<Device_DCServo*>(device)->driveForward(); // direct call no vtable
        return HALOperationResult::Success;
    }

    HALOperationResult Device_DCServo::exec_bw(Device* device) {
        static_cast<Device_DCServo*>(device)->stopMotor(); // direct call no vtable
        static_cast<Device_DCServo*>(device)->driveBackward(); // direct call no vtable
        return HALOperationResult::Success;
    }

    HALOperationResult Device_DCServo::exec_stop(Device* device) {
        static_cast<Device_DCServo*>(device)->stopMotor(); // direct call no vtable
        return HALOperationResult::Success;
    }

    HALOperationResult Device_DCServo::exec_reset(Device* device) {
        auto* d = static_cast<Device_DCServo*>(device);
        d->state = State::Idle;
        d->location = Location::Unknown;
        return HALOperationResult::Success;
    }


    Device::Exec_FuncType Device_DCServo::GetExec_Function(ZeroCopyString& zcFuncName) {
        if (zcFuncName == HAL_JSON_DEVICE_DCSERVO_CMD_FORWARD) {
            return exec_fw;
        } else if (zcFuncName == HAL_JSON_DEVICE_DCSERVO_CMD_BACKWARD) {
            return exec_bw;
        } else if (zcFuncName == HAL_JSON_DEVICE_DCSERVO_CMD_STOP) {
            return exec_stop;
        } else if (zcFuncName == HAL_JSON_DEVICE_DCSERVO_CMD_RESET) {
            return exec_stop;
        } else {
            return nullptr;
        }
    }

    HALOperationResult Device_DCServo::exec(const ZeroCopyString& cmd) {
        if (cmd == HAL_JSON_DEVICE_DCSERVO_CMD_FORWARD) {
            driveForward();
            return HALOperationResult::Success;
        } else if (cmd == HAL_JSON_DEVICE_DCSERVO_CMD_BACKWARD) {
            driveBackward();
            return HALOperationResult::Success;
        } else if (cmd == HAL_JSON_DEVICE_DCSERVO_CMD_STOP) {
            stopMotor();
            return HALOperationResult::Success;
        } else if (cmd == HAL_JSON_DEVICE_DCSERVO_CMD_RESET) {
            state = State::Idle;
            location = Location::Unknown;
            return HALOperationResult::Success;
        }
        return HALOperationResult::UnsupportedCommand;
    }


    HALOperationResult Device_DCServo::write(const HALValue& val) {
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
        stopMotor();

        if (v == 0) {
            driveBackward();
        } else {
            driveForward();
        }

        return HALOperationResult::Success;
    }

    HALOperationResult Device_DCServo::read(HALValue& val) {
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
            val = (int32_t)location;
        }
        return HALOperationResult::Success;
    }

    void Device_DCServo::stopMotor() {
        if (state != State::TimeoutFault)
            state = State::Idle;
        digitalWrite(pinForward, LOW);
        digitalWrite(pinBackward, LOW);
    }

    void Device_DCServo::driveForward() {
        if (endMaxActive()) return;
        state = State::MovingToMax;
        location = Location::Unknown;
        digitalWrite(pinBackward, LOW);
        digitalWrite(pinForward, HIGH);
        motionStartMs = millis();
    }

    void Device_DCServo::driveBackward() {
        if (endMinActive()) return;
        state = State::MovingToMin;
        location = Location::Unknown;
        digitalWrite(pinForward, LOW);
        digitalWrite(pinBackward, HIGH);
        motionStartMs = millis();
    }

    bool Device_DCServo::endMinActive() const {
        int v = digitalRead(pinEndMin);
        return pinEndMinActiveHigh ? (v == HIGH) : (v == LOW);
    }

    bool Device_DCServo::endMaxActive() const {
        int v = digitalRead(pinEndMax);
        return pinEndMaxActiveHigh ? (v == HIGH) : (v == LOW);
    }

    String Device_DCServo::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += "\"pinForward\":";
        ret += std::to_string(pinForward).c_str();
        ret += ',';
        ret += "\"pinBackward\":";
        ret += std::to_string(pinBackward).c_str();
        ret += ',';
        ret += "\"pinEndMin\":";
        ret += std::to_string(pinEndMin).c_str();
        ret += ',';
        ret += "\"pinEndMax\":";
        ret += std::to_string(pinEndMax).c_str();
        return ret;
    }

} // namespace HAL_JSON
