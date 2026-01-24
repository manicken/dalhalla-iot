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
    ledcSetup(ledcChannel, pwmFreq, pwmResolution);
    ledcAttachPin(pin, ledcChannel);
    // Initialize to 0%
    ledcWrite(ledcChannel, percentToDuty(0.0f));
}

HALOperationResult LEDC_Servo::write(const HALValue& val) {
    if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
    if (val.isNaN()) return HALOperationResult::WriteValueNaN;
    float pulseUs = 0.0f;
    float f = val.asFloat();
    if (f >= 0.0f && f <= 100.0f) {
        // Treat as percent
        pulseUs = 1000.0f + f * 10.0f; // 0% -> 1000us, 100% -> 2000us
    } 
    else if (f < 0.0f && f > 20000.0f) {
        return HALOperationResult::WriteValueOutOfRange;
    }
    else/* if (f >= 1000.0f && f <= 2000.0f)*/ {
        // Treat as direct microseconds
        pulseUs = f;
    } 
    /*else {
        return HALOperationResult::WriteValueOutOfRange;
    }*/

    lastPercent = (pulseUs - 1000.0f) / 10.0f; // store as percent internally
    uint32_t duty = (uint32_t)((pulseUs / 20000.0f) * 65535.0f); // 50Hz period = 20ms
    ledcWrite(ledcChannel, duty);

    return HALOperationResult::Success;
}

uint32_t LEDC_Servo::percentToDuty(float percent) {
    // Map 0–100% -> 1000–2000 µs
    float pulseUs = 1000.0f + percent * 10.0f; // 0% -> 1000, 100% -> 2000
    return (uint32_t)((pulseUs / 20000.0f) * 65535.0f); // 50Hz period = 20ms
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
