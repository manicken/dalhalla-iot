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

#include <Arduino.h> // TODO remove depend
#include <driver/ledc.h> // esp-idf

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Devices/_Registry/DALHAL_DevicesRegistry.h>


#define DALHAL_LEDC_SERVO_PWM_FREQ 50
#define DALHAL_LEDC_SERVO_PWM_DUTY_US (1000000u / DALHAL_LEDC_SERVO_PWM_FREQ)
#define DALHAL_LEDC_SERVO_RESOLUTION_BITS 12
#define DALHAL_LEDC_SERVO_RESOLUTION_MAX_VAL ((1U << (DALHAL_LEDC_SERVO_RESOLUTION_BITS))-1u)

#include <DALHAL/Core/Reactive/DALHAL_ReactiveTypes.h>
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(PWM_SERVO)
#include "DALHAL_PWM_Servo_Reactive.h"
using PWM_Servo_DeviceBase = DALHAL::PWM_Servo_Reactive;
#else
using PWM_Servo_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

class PWM_Servo : public PWM_Servo_DeviceBase {
private:
    enum class ServoValueType {
        Ratio,  // covers normalized [0..1], percent [0..100], degrees [-180..180] etc.
        PulseUS,     // raw microseconds
    };
    uint8_t pin = 0;
    ledc_channel_t pwmChannel = ledc_channel_t::LEDC_CHANNEL_0;

    HALValue lastValue = 0.0f;
    uint32_t minPulseLength = 1000;
    uint32_t maxPulseLength = 2000;
    int32_t pulseLengthOffset = 0;

    uint32_t startPulseLength = 1500;

    ServoValueType valueType = ServoValueType::PulseUS;
    float minVal;  // min for ratio type
    float maxVal;  // max for ratio type

    uint32_t autoOffAfterMs = 0; // set to 0 mean this function is off otherwise the pwm is turned off after the given value
    uint32_t lastWriteMs = 0;
    bool autoOffActive = false;

public:
    PWM_Servo(const JsonVariant &jsonObj, const char* type);
    ~PWM_Servo();

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
    HALOperationResult write(const HALWriteStringRequestValue& val) override;

    HALOperationResult read(HALValue& val) override;

    String ToString() override;
private:
    uint32_t ratioValueTypeToPulse(float fVal, bool clamp = true);
};

} // namespace DALHAL
