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

#include "DALHAL_PWM_Servo.h"
#include <DALHAL/Support/DALHAL_Logger.h>

// need to go hardcore on this stuff as arduino is only for beginners
#if defined(ESP32)
#include <driver/ledc.h> // esp-idf
#include <esp_err.h> // esp-idf
#elif defined(ESP8266)

#endif

#include "DALHAL_PWM_Servo_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase PWM_Servo::RegistryDefine = {
        Create,
        &JsonSchema::PWM_Servo::Root,
        DALHAL_REACTIVE_EVENT_TABLE(PWM_SERVO)
    };
    //volatile const void* keep_PWM_Servo = &DALHAL::PWM_Servo::RegistryDefine;

    Device* PWM_Servo::Create(DeviceCreateContext& context) {
        return new PWM_Servo(context);
    }

    PWM_Servo::PWM_Servo(DeviceCreateContext& context) : PWM_Servo_DeviceBase(context.deviceType) {
        JsonSchema::PWM_Servo::Extractors::Apply(context, this);

        // debug prints
        /*
        Serial.printf("\r\n MinVal:%f\r\n", minVal);
        Serial.printf("\r\n MaxVal:%f\r\n", maxVal);
        Serial.printf("\r\n minPulseLength:%d\r\n", minPulseLength);
        Serial.printf("\r\n maxPulseLength:%d\r\n", maxPulseLength);
        Serial.printf("\r\n startPulseLength:%d\r\n", startPulseLength);

        */
    }

    PWM_Servo::~PWM_Servo() {
#if defined(ESP32)
        ledcDetachPin(pin);
        //ledcWrite(ledcChannel, 0);
#elif defined(ESP8266)

#endif
    }

    void PWM_Servo::begin() {
        printf("PWM_Servo::begin\r\n");
#if defined(ESP32)
        // setup LEDC channel hardcore
        // Configure the timer first
        /* ledc_timer_config_t timer_conf = {
                .speed_mode       = LEDC_LOW_SPEED_MODE,  // or HIGH_SPEED_MODE if needed
                .duty_resolution  = (ledc_timer_bit_t)DALHAL_LEDC_SERVO_RESOLUTION_BITS,
                .timer_num        = (ledc_timer_t)(ledcChannel / 2), // simple mapping
                .freq_hz          = DALHAL_LEDC_SERVO_PWM_FREQ,
                .clk_cfg          = LEDC_AUTO_CLK
            };

            esp_err_t err = ledc_timer_config(&timer_conf);
            if (err != ESP_OK) {
                printf("ledc_timer_config failed: %d\n", err);
                
            }

            // Configure the channel
            ledc_channel_config_t ch_conf = {
                .gpio_num       = pin,
                .speed_mode     = LEDC_LOW_SPEED_MODE,
                .channel        = ledcChannel,
                .intr_type      = LEDC_INTR_DISABLE,
                .timer_sel      = timer_conf.timer_num,
                .duty           = 0,  // start with zero
                .hpoint         = 0
            };

            err = ledc_channel_config(&ch_conf);
            if (err != ESP_OK) {
                printf("ledc_channel_config failed: %d\n", err);
                //return err;
            }

            // Start the duty at zero
            ledc_set_duty(ch_conf.speed_mode, ch_conf.channel, 0) ||
                ledc_update_duty(ch_conf.speed_mode, ch_conf.channel);*/
        //channels_resolution[chan] = bit_num;
        //return ledc_get_freq(group,timer);

        // Setup LEDC channel
        if (ledcSetup(pwmChannel, DALHAL_LEDC_SERVO_PWM_FREQ, DALHAL_LEDC_SERVO_RESOLUTION_BITS) == 0) {
            printf("ledcSetup fail\r\n");
        } 
        ledcAttachPin(pin, pwmChannel);
        // Initialize to center
        uint32_t pulseUs = startPulseLength + pulseLengthOffset;
        uint32_t duty = (startPulseLength * DALHAL_LEDC_SERVO_RESOLUTION_MAX_VAL) / DALHAL_LEDC_SERVO_PWM_DUTY_US;
        ledcWrite(pwmChannel, duty);
#elif defined(ESP8266)

#endif
    }

    void PWM_Servo::loop() {
        if (autoOffActive == false) return; // this will only be set to true when writing and autoOffAfterMs != 0
        uint32_t now = millis();
        if ((now - lastWriteMs) > autoOffAfterMs) {
            autoOffActive = false;
#if defined(ESP32)
            ledcWrite(pwmChannel, 0); // turn pwm off
#elif defined(ESP8266)

#endif
        }
    }

    HALOperationResult PWM_Servo::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;

        uint32_t pulseUs = 0;
        if (valueType == ServoValueType::Ratio) {
            float fVal = val.toFloat();
            if (fVal < minVal || fVal > maxVal) {
                return HALOperationResult::WriteValueOutOfRange;
            }
            printf("\r\n PWM_Servo write fVal:%f\r\n", fVal);
            pulseUs = ratioValueTypeToPulse(fVal, true);
        } else if (valueType == ServoValueType::PulseUS) {
            uint32_t uiVal = val.toUInt();
            if (uiVal < minPulseLength || uiVal > maxPulseLength) {
                return HALOperationResult::WriteValueOutOfRange;
            }
            pulseUs = uiVal;
        } else { // should never happend
            return HALOperationResult::UnsupportedOperation;
        }
        pulseUs += pulseLengthOffset;
        printf("\r\n PWM_Servo write pulseUs:%d\r\n", pulseUs);
#if defined(ESP32)
        uint32_t duty = (pulseUs * DALHAL_LEDC_SERVO_RESOLUTION_MAX_VAL) / DALHAL_LEDC_SERVO_PWM_DUTY_US;
        //Serial.printf("\r\n PWM_Servo write duty:%d\r\n", duty);

        ledcWrite(pwmChannel, duty);
#elif defined(ESP8266)

#endif
        if (autoOffAfterMs != 0) {
            autoOffActive = true;
            lastWriteMs = millis();
        }
        lastValue = val;
#if HAS_REACTIVE_WRITE(PWM_SERVO)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult PWM_Servo::write(const HALWriteStringRequestValue& val) {
        uint32_t pulseUs = 0;
        ZeroCopyString zcStr = val.value;
        ZeroCopyString zcCmd = zcStr.SplitOffHead('/');
        if (zcCmd.IsEmpty() || zcStr.IsEmpty() || (zcStr.ValidNumber() == false)) return HALOperationResult::InvalidArgument;


        if (zcCmd.EqualsIC(F("ratio"))) {
            float fVal = 0.0f;
            zcStr.ConvertTo_float(fVal);
            //Serial.printf("\r\n PWM_Servo write fVal:%f\r\n", fVal);
            pulseUs = ratioValueTypeToPulse(fVal, false);
        } else if (zcCmd.EqualsIC(F("pulse"))) {
            uint32_t uiVal = 0;
            zcStr.ConvertTo_uint32(uiVal);
            pulseUs = uiVal;
        } else {
            return HALOperationResult::UnsupportedCommand;
        }
        
    // Serial.printf("\r\n PWM_Servo write pulseUs:%d\r\n", pulseUs);
        uint32_t duty = (pulseUs * DALHAL_LEDC_SERVO_RESOLUTION_MAX_VAL) / DALHAL_LEDC_SERVO_PWM_DUTY_US;
        //Serial.printf("\r\n PWM_Servo write duty:%d\r\n", duty);
#if defined(ESP32)
        ledcWrite(pwmChannel, duty);
#elif defined(ESP8266)

#else
        printf("\r\n PWM_Servo write duty:%d\r\n", duty);
#endif

        if (autoOffAfterMs != 0) {
            autoOffActive = true;
            lastWriteMs = millis();
        }
#if HAS_REACTIVE_WRITE(PWM_SERVO)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    HALOperationResult PWM_Servo::read(HALValue& val) {
        val = lastValue;
#if HAS_REACTIVE(PWM_SERVO, READ)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }

    String PWM_Servo::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\",";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ", ledc_ch:";
        ret += std::to_string(pwmChannel).c_str();
        return ret;
    }
    uint32_t PWM_Servo::ratioValueTypeToPulse(float fVal, bool clamp/* = true*/) {
        float ratio = (fVal - minVal) / (maxVal - minVal);
#if defined(ESP8266) || defined(ESP32)
        if (clamp) ratio = constrain(ratio, 0.0f, 1.0f);
#endif
        return minPulseLength + ratio * (maxPulseLength - minPulseLength);
    }

} // namespace DALHAL
