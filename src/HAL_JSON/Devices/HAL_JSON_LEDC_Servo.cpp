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
    if (jsonObj.containsKey("autoOffAfterMs")) {
        autoOffAfterMs = jsonObj["autoOffAfterMs"];
    }
    if (jsonObj.containsKey("useNormValue")) {
        useNormValue = jsonObj["useNormValue"];
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
    ledcSetup(ledcChannel, HAL_JSON_LEDC_SERVO_PWM_FREQ, HAL_JSON_LEDC_SERVO_RESOLUTION_BITS);
    ledcAttachPin(pin, ledcChannel);
    // Initialize to center
    uint32_t duty = (centerPulseLength * HAL_JSON_LEDC_SERVO_RESOLUTION_MAX_VAL) / HAL_JSON_LEDC_SERVO_PWM_DUTY_US;
    ledcWrite(ledcChannel, duty);
}

void LEDC_Servo::loop() {
    if (autoOffActive == false) return; // this will only be set to true when writing and autoOffAfterMs != 0
    uint32_t now = millis();
    if ((now - lastWriteMs) > autoOffAfterMs) {
        autoOffActive = false;
        ledcWrite(ledcChannel, 0); // turn pwm off
    }
}

HALOperationResult LEDC_Servo::write(const HALValue& val) {
    if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
    if (val.isNaN()) return HALOperationResult::WriteValueNaN;
    uint32_t pulseUs = 0;
    float fVal = val.asFloat();
    lastValue = val;
    if (fVal >= 0 && (useNormValue?(fVal <= 1.0f):(fVal <= 100))) {
        // Treat as percent
        if (useNormValue) {
            pulseUs = normToPulseLength_uS(fVal);
        } else {
            pulseUs = percentToPulseLength_uS(fVal);
        }
    } 
    else if (fVal >= minPulseLength && fVal <= maxPulseLength) {
        // Treat as direct microseconds
        pulseUs = val.asUInt();
    }
    else {
        return HALOperationResult::WriteValueOutOfRange;
    } 

    uint32_t duty = (pulseUs * HAL_JSON_LEDC_SERVO_RESOLUTION_MAX_VAL) / HAL_JSON_LEDC_SERVO_PWM_DUTY_US;

    ledcWrite(ledcChannel, duty);
    if (autoOffAfterMs != 0) {
        autoOffActive = true;
        lastWriteMs = millis();
    }

    return HALOperationResult::Success;
}

HALOperationResult LEDC_Servo::write(const HALWriteValueByCmd& val) {
    if (val.cmd == "percent") {
        return write(val.value);
    } else if (val.cmd == "us") {
        uint32_t pulseUs = val.value.asInt();
        uint32_t duty = (pulseUs * HAL_JSON_LEDC_SERVO_RESOLUTION_MAX_VAL) / HAL_JSON_LEDC_SERVO_PWM_DUTY_US;

        ledcWrite(ledcChannel, duty);

        if (autoOffAfterMs != 0) {
            autoOffActive = true;
            lastWriteMs = millis();
        }
    } else {
        HALOperationResult::UnsupportedCommand;
    }
    return HALOperationResult::Success;
}

HALOperationResult LEDC_Servo::read(HALValue& val) {
    val = lastValue;
    return HALOperationResult::Success;
}

uint32_t LEDC_Servo::percentToPulseLength_uS(float percent) {
    constexpr float CENTER_EPS_PERCENT = 0.001f;  // 0.1%

    // center is defined precicely
    if (fabsf(percent - 50.0f) < CENTER_EPS_PERCENT) {
        return centerPulseLength;
    }
        
    if (percent < 50.0f) {
        // Map 0–50% → min → center
        return minPulseLength +
            (percent / 50.0f) * (centerPulseLength - minPulseLength);
    } else {
        // Map 50–100% → center → max
        return centerPulseLength +
            ((percent - 50.0f) / 50.0f) * (maxPulseLength - centerPulseLength);
    }
}

uint32_t LEDC_Servo::normToPulseLength_uS(float norm) {
    constexpr float CENTER_EPS_NORM    = 0.00001f;

    // center is defined precicely
    if (fabsf(norm - 0.5f) < CENTER_EPS_NORM) {
        return centerPulseLength;
    }

    norm = constrain(norm, 0.0f, 1.0f);

    if (norm < 0.5f) {
        return minPulseLength +
            (norm / 0.5f) * (centerPulseLength - minPulseLength);
    } else {
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
