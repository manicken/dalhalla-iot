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


#include "HAL_JSON_LEDC_Servo.h"

namespace HAL_JSON {

LEDC_Servo::LEDC_Servo(const JsonVariant &jsonObj, const char* type)
    : Device(type)
{
    const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);
    uid = encodeUID(uidStr);
    pin = jsonObj["pin"];
    ledcChannel = jsonObj["ledc_ch"];
    if (jsonObj.containsKey("minPulseLength")) {
        minPulseLength = jsonObj["minPulseLength"];
    }
    if (jsonObj.containsKey("maxPulseLength")) {
        maxPulseLength = jsonObj["maxPulseLength"];
    }
    if (jsonObj.containsKey("centerPulseLength")) {
        centerPulseLength = jsonObj["centerPulseLength"];
    }
}

bool LEDC_Servo::VerifyJSON(const JsonVariant &jsonObj) {
    bool anyError = false;
    if (!jsonObj.containsKey("pin")) {
        GlobalLogger.Error(F("cfg have no pin"));
        GlobalLogger.setLastEntrySource("LEDC_Servo::VerifyJSON");
        anyError = true;
    }
    if (!jsonObj.containsKey("ledc_ch")) {
        GlobalLogger.Error(F("cfg have no ledc_ch"));
        GlobalLogger.setLastEntrySource("LEDC_Servo::VerifyJSON");
        anyError = true;
    }
    anyError = GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN))) == false;

    return (anyError == false);
}

Device* LEDC_Servo::Create(const JsonVariant &jsonObj, const char* type) {
    return new LEDC_Servo(jsonObj, type);
}

void LEDC_Servo::begin() {
    // Setup LEDC channel
    ledcSetup(ledcChannel, HAL_JSON_LEDC_SERVO_PWM_FREQ, pwmResolution);
    ledcAttachPin(pin, ledcChannel);
    // Initialize to center
    ledcWrite(ledcChannel, centerPulseLength);
}

HALOperationResult LEDC_Servo::write(const HALValue& val) {
    if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
    if (val.isNaN()) return HALOperationResult::WriteValueNaN;
    uint32_t pulseUs = 0;
    int32_t iVal = val.asInt();
    lastValue = val;
    if (iVal >= 0 && iVal <= 100) {
        // Treat as percent
        pulseUs = percentToPulseLength_uS(val.asFloat());
    } 
    else if (iVal > 100 && iVal <= HAL_JSON_LEDC_SERVO_PWM_DUTY_US) {
        // Treat as direct microseconds
        pulseUs = iVal;
    }
    else {
        return HALOperationResult::WriteValueOutOfRange;
    } 

    uint32_t duty = (pulseUs * 65535u) / HAL_JSON_LEDC_SERVO_PWM_DUTY_US;

    ledcWrite(ledcChannel, duty);

    return HALOperationResult::Success;
}

HALOperationResult LEDC_Servo::write(const HALWriteValueByCmd& val) {
    if (val.cmd == "percent") {

    } else if (val.cmd == "us") {
        uint32_t pulseUs = val.value.asInt();
        uint32_t duty = (pulseUs * 65535u) / HAL_JSON_LEDC_SERVO_PWM_DUTY_US;

        ledcWrite(ledcChannel, duty);
    } else {
        HALOperationResult::UnsupportedCommand;
    }
    return HALOperationResult::Success;
}

uint32_t LEDC_Servo::percentToPulseLength_uS(float percent) {
    if (percent < 50.0f) {
        // Map 0–50% → min → center
        return minPulseLength +
            (percent / 50.0f) * (centerPulseLength - minPulseLength);
    } else if (percent == 50.0f) {
        return centerPulseLength;
    } else {
        // Map 50–100% → center → max
        return centerPulseLength +
            ((percent - 50.0f) / 50.0f) * (maxPulseLength - centerPulseLength);
    }
}

uint32_t LEDC_Servo::normToPulseLength_uS(float norm) {
    norm = constrain(norm, 0.0f, 1.0f);

    if (norm < 0.5f) {
        return minPulseLength +
            (norm / 0.5f) * (centerPulseLength - minPulseLength);
    } else if (norm == 0.5f) {
        return centerPulseLength;
    }
    else {
        return centerPulseLength +
            ((norm - 0.5f) / 0.5f) * (maxPulseLength - centerPulseLength);
    }
}

String LEDC_Servo::ToString() {
    String ret;
    ret += DeviceConstStrings::uid;
    ret += decodeUID(uid).c_str();
    ret += "\",";
    ret += DeviceConstStrings::type;
    ret += type;
    ret += "\",";
    ret += DeviceConstStrings::pin;
    ret += std::to_string(pin).c_str();
    ret += ", ledc_ch:";
    ret += std::to_string(ledcChannel).c_str();
    return ret;
}

} // namespace HAL_JSON
