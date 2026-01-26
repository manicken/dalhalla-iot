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
#include "HAL_JSON_template.h"
#include <Arduino.h>


#define HAL_JSON_LEDC_SERVO_PWM_FREQ 50
#define HAL_JSON_LEDC_SERVO_PWM_DUTY_US (1000000u / HAL_JSON_LEDC_SERVO_PWM_FREQ)
#define HAL_JSON_LEDC_SERVO_RESOLUTION_BITS 16
#define HAL_JSON_LEDC_SERVO_RESOLUTION_MAX_VAL ((1U << (HAL_JSON_LEDC_SERVO_RESOLUTION_BITS))-1u)

namespace HAL_JSON {

class LEDC_Servo : public Device {
private:
    uint8_t pin = 0;
    uint8_t ledcChannel = 0;

    HALValue lastValue = 0.0f;
    uint32_t minPulseLength = 1000;
    uint32_t maxPulseLength = 2000;
    uint32_t centerPulseLength = 1500;

    uint32_t autoOffAfterMs = 0; // set to 0 mean this function is off otherwise the pwm is turned off after the given value
    uint32_t lastWriteMs = 0;
    bool autoOffActive = false;

    bool useNormValue = false; // if percent input is 0.0f-1.0f as 0-100% range and value >= 2 is threated as pulse length in uS

public:
    LEDC_Servo(const JsonVariant &jsonObj, const char* type);

    static bool VerifyJSON(const JsonVariant &jsonObj);
    static Device* Create(const JsonVariant &jsonObj, const char* type);
    static constexpr DeviceRegistryDefine RegistryDefine = {
        UseRootUID::Mandatory,
        Create,
        VerifyJSON
    };

    void begin() override;
    void loop() override;

    HALOperationResult write(const HALValue& val) override;
    HALOperationResult write(const HALWriteValueByCmd& val) override;

    HALOperationResult read(HALValue& val) override;

    String ToString() override;

private:
    uint32_t percentToPulseLength_uS(float percent);
    uint32_t normToPulseLength_uS(float norm);
};

} // namespace HAL_JSON
