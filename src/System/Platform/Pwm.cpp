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

#include "Pwm.h"

#if defined(DALHAL_BACKEND_ARDUINO)
#include <Arduino.h>
#endif
#if defined(DALHAL_PLATFORM_ESP32) && defined(DALHAL_BACKEND_ARDUINO)
#include <esp_arduino_version.h>
#elif defined(DALHAL_PLATFORM_ESP32) && defined(DALHAL_BACKEND_ESPIDF)
#include <driver/ledc.h>
#elif defined(DALHAL_PLATFORM_PC)
#include <cstdio>
#endif

namespace Platform {
namespace Pwm {

#if defined(DALHAL_PLATFORM_ESP32) && defined(DALHAL_BACKEND_ARDUINO) && (ESP_ARDUINO_VERSION_MAJOR >= 3)

    bool Attach(const Output& out, uint32_t freqHz, uint8_t resolutionBits) {
        return ledcAttachChannel(out.pin, freqHz, resolutionBits, out.channel);
    }

    void Write(const Output& out, uint32_t duty) {
        ledcWrite(out.pin, duty);
    }

    void Detach(const Output& out) {
        ledcDetach(out.pin);
    }

#elif defined(DALHAL_PLATFORM_ESP32) && defined(DALHAL_BACKEND_ESPIDF)

    // LEDC low speed mode exists on every ESP32 chip, channels share timers in pairs as on arduino-esp32
    static ledc_channel_t ToChannel(uint8_t channel) { return (ledc_channel_t)(channel % LEDC_CHANNEL_MAX); }
    static ledc_timer_t ToTimer(uint8_t channel) { return (ledc_timer_t)((ToChannel(channel) / 2) % LEDC_TIMER_MAX); }

    bool Attach(const Output& out, uint32_t freqHz, uint8_t resolutionBits) {
        ledc_timer_config_t timerCfg = {};
        timerCfg.speed_mode = LEDC_LOW_SPEED_MODE;
        timerCfg.duty_resolution = (ledc_timer_bit_t)resolutionBits;
        timerCfg.timer_num = ToTimer(out.channel);
        timerCfg.freq_hz = freqHz;
        timerCfg.clk_cfg = LEDC_AUTO_CLK;
        if (ledc_timer_config(&timerCfg) != ESP_OK) {
            return false;
        }
        ledc_channel_config_t chCfg = {};
        chCfg.gpio_num = out.pin;
        chCfg.speed_mode = LEDC_LOW_SPEED_MODE;
        chCfg.channel = ToChannel(out.channel);
        chCfg.intr_type = LEDC_INTR_DISABLE;
        chCfg.timer_sel = ToTimer(out.channel);
        chCfg.duty = 0;
        chCfg.hpoint = 0;
        return ledc_channel_config(&chCfg) == ESP_OK;
    }

    void Write(const Output& out, uint32_t duty) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, ToChannel(out.channel), duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, ToChannel(out.channel));
    }

    void Detach(const Output& out) {
        ledc_stop(LEDC_LOW_SPEED_MODE, ToChannel(out.channel), 0);
        gpio_reset_pin((gpio_num_t)out.pin);
    }

#elif defined(DALHAL_PLATFORM_ESP32)

    bool Attach(const Output& out, uint32_t freqHz, uint8_t resolutionBits) {
        if (ledcSetup(out.channel, freqHz, resolutionBits) == 0) {
            return false;
        }
        ledcAttachPin(out.pin, out.channel);
        return true;
    }

    void Write(const Output& out, uint32_t duty) {
        ledcWrite(out.channel, duty);
    }

    void Detach(const Output& out) {
        ledcDetachPin(out.pin);
    }

#elif defined(DALHAL_PLATFORM_ESP8266)

    bool Attach(const Output& out, uint32_t freqHz, uint8_t resolutionBits) {
        pinMode(out.pin, OUTPUT);
        analogWriteFreq(freqHz);
        analogWriteResolution(resolutionBits);
        return true;
    }

    void Write(const Output& out, uint32_t duty) {
        analogWrite(out.pin, duty);
    }

    void Detach(const Output& out) {
        pinMode(out.pin, INPUT);
    }

#elif defined(DALHAL_PLATFORM_PC)

    bool Attach(const Output& out, uint32_t freqHz, uint8_t resolutionBits) {
        printf("\r\n Pwm::Attach pin:%u ch:%u freq:%u bits:%u\r\n", out.pin, out.channel, (unsigned)freqHz, resolutionBits);
        return true;
    }

    void Write(const Output& out, uint32_t duty) {
        printf("\r\n Pwm::Write pin:%u duty:%u\r\n", out.pin, (unsigned)duty);
    }

    void Detach(const Output& out) {
        (void)out;
    }

#endif

}
}
