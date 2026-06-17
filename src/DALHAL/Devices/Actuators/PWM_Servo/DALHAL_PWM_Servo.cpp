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
        DALHAL_REACTIVE_EVENT_TABLE(PWM_SERVO),
        &PWM_Servo::FunctionTable
    };

    /* override */
    const Registry::DefineBase* PWM_Servo::GetRegistryDefine() {
        return &RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> PWM_Servo::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "read last written value defined by internal mode")
    };

    constexpr FunctionEntry<FunctionTypes::WriteHALValue> PWM_Servo::writeValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY_WITH_VAL_TYPE(HALValue_primary_write, "write value defined by internal mode", FunctionValueType::_Number_),
        DALHAL_FUNCTION_ENTRY_WITH_VAL_TYPE("ratio", writeAsRatio, "set value explicit as ratio", FunctionValueType::_Number_),
        DALHAL_FUNCTION_ENTRY_WITH_VAL_TYPE("pulse", writeAsPulseLength, "set value explicit as pulse", FunctionValueType::_UInt_)
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable PWM_Servo::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeValueFunctions),
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };

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
        pinMode(pin, INPUT);
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
        pinMode(pin, OUTPUT);        
        analogWriteFreq(DALHAL_LEDC_SERVO_PWM_FREQ);
        analogWriteResolution(DALHAL_LEDC_SERVO_RESOLUTION_BITS);
        //uint32_t pulseUs = startPulseLength + pulseLengthOffset;
        uint32_t duty = (startPulseLength * DALHAL_LEDC_SERVO_RESOLUTION_MAX_VAL) / DALHAL_LEDC_SERVO_PWM_DUTY_US;
        analogWrite(pin, duty);
#endif
    }

    /*virtual override*/
    void PWM_Servo::loop() {
        if (autoOffActive == false) return; // this will only be set to true when writing and autoOffAfterMs != 0
        uint32_t now = millis();
        if ((now - lastWriteMs) > autoOffAfterMs) {
            autoOffActive = false;
#if defined(ESP32)
            ledcWrite(pwmChannel, 0); // turn pwm off
#elif defined(ESP8266)
            analogWrite(pin, 0);
#endif
        }
    }

    /* static */
    HALOperationResult PWM_Servo::HALValue_primary_write(Device* device, const HALValue& val) {
        PWM_Servo& self = static_cast<PWM_Servo&>(*device);
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;

        if (self.valueType == ServoValueType::Ratio) {
            float fVal = val.toFloat();
            if (fVal < self.minVal || fVal > self.maxVal) {
                return HALOperationResult::WriteValueOutOfRange;
            }
            printf("\r\n PWM_Servo write fVal:%f\r\n", fVal);
            self.writeAsPulseLength(self.ratioValueTypeToPulse(fVal, true) + self.pulseLengthOffset);
        } else if (self.valueType == ServoValueType::PulseUS) {
            uint32_t uiVal = val.toUInt();
            if (uiVal < self.minPulseLength || uiVal > self.maxPulseLength) {
                return HALOperationResult::WriteValueOutOfRange;
            }
            self.writeAsPulseLength(uiVal + self.pulseLengthOffset);
        } else { // should never happend
            return HALOperationResult::UnsupportedOperation;
        }
        
        if (self.autoOffAfterMs != 0) {
            self.autoOffActive = true;
            self.lastWriteMs = millis();
        }
        self.lastValue = val;

        return HALOperationResult::Success;
    }

    HALOperationResult PWM_Servo::writeAsPulseLength(uint32_t pulseUs) {
        // Serial.printf("\r\n PWM_Servo write pulseUs:%d\r\n", pulseUs);
        uint32_t duty = (pulseUs * DALHAL_LEDC_SERVO_RESOLUTION_MAX_VAL) / DALHAL_LEDC_SERVO_PWM_DUTY_US;
        //Serial.printf("\r\n PWM_Servo write duty:%d\r\n", duty);
#if defined(ESP32)
        ledcWrite(pwmChannel, duty);
#elif defined(ESP8266)
        analogWrite(pin, duty);
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

    /*static*/
    HALOperationResult PWM_Servo::writeAsRatio(Device* device, const HALValue& val) {
        if (val.isNaN()) { return HALOperationResult::WriteValueNaN; }
        uint32_t uiVal = static_cast<PWM_Servo*>(device)->ratioValueTypeToPulse(val.toFloat(), false);
        return static_cast<PWM_Servo*>(device)->writeAsPulseLength(uiVal);
    }
    
    /*static*/
    HALOperationResult PWM_Servo::writeAsPulseLength(Device* device, const HALValue& val) {
        if (val.isNaN()) { return HALOperationResult::WriteValueNaN; }
        return static_cast<PWM_Servo*>(device)->writeAsPulseLength(val.toUInt());
    }

    /* static */
    HALOperationResult PWM_Servo::HALValue_primary_read(Device* device, HALValue& val) {
        PWM_Servo& self = static_cast<PWM_Servo&>(*device);
        val = self.lastValue;
#if HAS_REACTIVE(PWM_SERVO, READ)
        self.triggerRead();
#endif
        return HALOperationResult::Success;
    }

    /*virtual override*/
    void PWM_Servo::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
        
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pin"), pin);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pwmChannel"), pwmChannel);
    }

    uint32_t PWM_Servo::ratioValueTypeToPulse(float fVal, bool clamp/* = true*/) {
        float ratio = (fVal - minVal) / (maxVal - minVal);
#if defined(ESP8266) || defined(ESP32)
        if (clamp) ratio = constrain(ratio, 0.0f, 1.0f);
#endif
        return minPulseLength + ratio * (maxPulseLength - minPulseLength);
    }

} // namespace DALHAL
