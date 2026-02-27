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


#include "DALHAL_LEDC_Servo.h"
#include <math.h>
// need to go hardcore on this stuff as arduino is only for beginners
#include "driver/ledc.h"
#include "esp_err.h"
#include <stdio.h>
#include "../Support/DALHAL_ArduinoJSON_ext.h"
#include "../Support/DALHAL_Logger.h"
#include "../Core/Manager/DALHAL_GPIO_Manager.h"

#define VERIFY_JSON_SOURCE "LEDC_Servo::VerifyJSON"

namespace DALHAL {

LEDC_Servo::LEDC_Servo(const JsonVariant &jsonObj, const char* type)
    : Device(type)
{
    const char* uidStr = GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID);
    uid = encodeUID(uidStr);
    pin = jsonObj["pin"];
    ledcChannel = jsonObj["ledc_ch"];

    minPulseLength = jsonObj.containsKey("minPulseLength") ? jsonObj["minPulseLength"] : 1000;
    maxPulseLength = jsonObj.containsKey("maxPulseLength") ? jsonObj["maxPulseLength"] : 2000;
    startPulseLength = jsonObj.containsKey("startPulseLength") ? jsonObj["startPulseLength"] : 1500;
    autoOffAfterMs = jsonObj.containsKey("autoOffAfterMs") ? jsonObj["autoOffAfterMs"] : 0;
    pulseLengthOffset = jsonObj.containsKey("pulseLengthOffset") ? jsonObj["pulseLengthOffset"] : 0;

    // minVal / maxVal optional
    bool hasMin = ValidateFloat(jsonObj, "minVal");
    bool hasMax = ValidateFloat(jsonObj, "maxVal");

    if (hasMin && hasMax) {
        minVal = jsonObj["minVal"].as<float>();
        maxVal = jsonObj["maxVal"].as<float>();
        valueType = ServoValueType::Ratio;
    } else {
        minVal = NAN; // not used in this mode
        maxVal = NAN; // not used in this mode
        valueType = ServoValueType::PulseUS;
    }

    Serial.printf("\r\n MinVal:%f\r\n", minVal);
    Serial.printf("\r\n MaxVal:%f\r\n", maxVal);
    Serial.printf("\r\n minPulseLength:%d\r\n", minPulseLength);
    Serial.printf("\r\n maxPulseLength:%d\r\n", maxPulseLength);
    Serial.printf("\r\n startPulseLength:%d\r\n", startPulseLength);
}

bool LEDC_Servo::VerifyJSON(const JsonVariant &jsonObj) {
    bool anyError = false;
    if (!jsonObj.containsKey("pin")) {
        GlobalLogger.Error(F("cfg have no pin"));
        GlobalLogger.setLastEntrySource(VERIFY_JSON_SOURCE);
        anyError = true;
    }
    if (!jsonObj.containsKey("ledc_ch")) {
        GlobalLogger.Error(F("cfg have no ledc_ch"));
        GlobalLogger.setLastEntrySource(VERIFY_JSON_SOURCE);
        anyError = true;
    }
    anyError |= GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinFunc::OUT) | static_cast<uint8_t>(GPIO_manager::PinFunc::IN))) == false;
    bool hasMin = ValidateFloat(jsonObj, "minVal");
    bool hasMax = ValidateFloat(jsonObj, "maxVal");

    if (hasMin != hasMax) {
        GlobalLogger.Error(F("cfg must define both minVal and maxVal"));
        GlobalLogger.setLastEntrySource(VERIFY_JSON_SOURCE);
        anyError = true;
    } else if (hasMin && hasMax) {
        float minVal = jsonObj["minVal"].as<float>();
        float maxVal = jsonObj["maxVal"].as<float>();
        if (minVal >= maxVal) {
            GlobalLogger.Error(F("minVal cannot be greater or equal to maxVal"));
            GlobalLogger.setLastEntrySource(VERIFY_JSON_SOURCE);
            anyError = true;
        }
    }
    return (anyError == false);
}

Device* LEDC_Servo::Create(const JsonVariant &jsonObj, const char* type) {
    return new LEDC_Servo(jsonObj, type);
}

LEDC_Servo::~LEDC_Servo() {
    ledcDetachPin(pin);
    //ledcWrite(ledcChannel, 0);
}

void LEDC_Servo::begin() {
    printf("LEDC_Servo::begin\r\n");
    
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
    if (ledcSetup(ledcChannel, DALHAL_LEDC_SERVO_PWM_FREQ, DALHAL_LEDC_SERVO_RESOLUTION_BITS) == 0) {
        printf("ledcSetup fail\r\n");
    } 
    ledcAttachPin(pin, ledcChannel);
    // Initialize to center
    uint32_t pulseUs = startPulseLength + pulseLengthOffset;
    uint32_t duty = (startPulseLength * DALHAL_LEDC_SERVO_RESOLUTION_MAX_VAL) / DALHAL_LEDC_SERVO_PWM_DUTY_US;
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
    if (valueType == ServoValueType::Ratio) {
        float fVal = val.asFloat();
        if (fVal < minVal || fVal > maxVal) {
            return HALOperationResult::WriteValueOutOfRange;
        }
        printf("\r\n LEDC_Servo write fVal:%f\r\n", fVal);
        pulseUs = ratioValueTypeToPulse(fVal, true);
    } else if (valueType == ServoValueType::PulseUS) {
        uint32_t uiVal = val.asUInt();
        if (uiVal < minPulseLength || uiVal > maxPulseLength) {
            return HALOperationResult::WriteValueOutOfRange;
        }
        pulseUs = uiVal;
    } else { // should never happend
        return HALOperationResult::UnsupportedOperation;
    }
    pulseUs += pulseLengthOffset;
    printf("\r\n LEDC_Servo write pulseUs:%d\r\n", pulseUs);
    
    uint32_t duty = (pulseUs * DALHAL_LEDC_SERVO_RESOLUTION_MAX_VAL) / DALHAL_LEDC_SERVO_PWM_DUTY_US;
    //Serial.printf("\r\n LEDC_Servo write duty:%d\r\n", duty);
    ledcWrite(ledcChannel, duty);
    if (autoOffAfterMs != 0) {
        autoOffActive = true;
        lastWriteMs = millis();
    }
    lastValue = val;

    return HALOperationResult::Success;
}

HALOperationResult LEDC_Servo::write(const HALWriteStringRequestValue& val) {
    uint32_t pulseUs = 0;
    ZeroCopyString zcStr = val.value;
    ZeroCopyString zcCmd = zcStr.SplitOffHead('/');
    if (zcCmd.IsEmpty() || zcStr.IsEmpty() || (zcStr.ValidNumber() == false)) return HALOperationResult::InvalidArgument;


    if (zcCmd.EqualsIC("ratio")) {
        float fVal = 0.0f;
        zcStr.ConvertTo_float(fVal);
        //Serial.printf("\r\n LEDC_Servo write fVal:%f\r\n", fVal);
        pulseUs = pulseUs = ratioValueTypeToPulse(fVal, false);
    } else if (zcCmd.EqualsIC("pulse")) {
        uint32_t uiVal = 0;
        zcStr.ConvertTo_uint32(uiVal);
        pulseUs = uiVal;
    } else {
        return HALOperationResult::UnsupportedCommand;
    }
    
   // Serial.printf("\r\n LEDC_Servo write pulseUs:%d\r\n", pulseUs);
    uint32_t duty = (pulseUs * DALHAL_LEDC_SERVO_RESOLUTION_MAX_VAL) / DALHAL_LEDC_SERVO_PWM_DUTY_US;
    //Serial.printf("\r\n LEDC_Servo write duty:%d\r\n", duty);
    ledcWrite(ledcChannel, duty);

    if (autoOffAfterMs != 0) {
        autoOffActive = true;
        lastWriteMs = millis();
    }
    return HALOperationResult::Success;
}

HALOperationResult LEDC_Servo::read(HALValue& val) {
    val = lastValue;
    return HALOperationResult::Success;
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
uint32_t LEDC_Servo::ratioValueTypeToPulse(float fVal, bool clamp/* = true*/) {
    float ratio = (fVal - minVal) / (maxVal - minVal);
    if (clamp) ratio = constrain(ratio, 0.0f, 1.0f);
    return minPulseLength + ratio * (maxPulseLength - minPulseLength);
}

} // namespace DALHAL
