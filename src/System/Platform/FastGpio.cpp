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

#include "FastGpio.h"

#if defined(DALHAL_PLATFORM_ESP32)
#include <driver/gpio.h>
#include <hal/gpio_ll.h>
#include <soc/gpio_struct.h>
#include <stdio.h>
#endif

namespace Platform {
namespace FastGpio {

#if defined(DALHAL_PLATFORM_ESP32)

    static void Configure(Pin pin, gpio_mode_t mode, gpio_int_type_t intrType) {
        gpio_config_t io_conf{};
        io_conf.pin_bit_mask = (1ULL << pin);
        io_conf.mode = mode;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.intr_type = intrType;
        esp_err_t res = gpio_config(&io_conf);
        if (res != ESP_OK) {
            printf("\r\nFastGpio gpio_config error:%d pin:%d\r\n", (int)res, (int)pin);
        }
    }

    void ConfigureOutput(Pin pin) {
        if (pin == NC) { return; }
        Configure(pin, GPIO_MODE_OUTPUT, GPIO_INTR_DISABLE);
    }

    void Release(Pin pin) {
        if (pin == NC) { return; }
        Configure(pin, GPIO_MODE_INPUT, GPIO_INTR_DISABLE);
    }

    void Write(Pin pin, bool level) {
        if (pin == NC) { return; }
        gpio_set_level((gpio_num_t)pin, level ? 1 : 0);
    }

    bool Read(Pin pin) {
        if (pin == NC) { return false; }
        return gpio_get_level((gpio_num_t)pin) != 0;
    }

    bool AttachEdgeIsr(Pin pin, Edge edge, IsrHandler handler, void* arg) {
        if (pin == NC) { return false; }
        static bool isrServiceInstalled = false;
        if (isrServiceInstalled == false) {
            gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
            isrServiceInstalled = true;
        }
        Configure(pin, GPIO_MODE_INPUT, (edge == Edge::Rising) ? GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE);
        return gpio_isr_handler_add((gpio_num_t)pin, handler, arg) == ESP_OK;
    }

    void DetachIsr(Pin pin) {
        if (pin == NC) { return; }
        gpio_intr_disable((gpio_num_t)pin);
        gpio_isr_handler_remove((gpio_num_t)pin);
    }

    void IsrEnable(Pin pin) {
        if (pin == NC) { return; }
        gpio_intr_enable((gpio_num_t)pin);
    }

    void PLATFORM_ISR_ATTR IsrDisable(Pin pin) {
        if (pin == NC) { return; }
        gpio_intr_disable((gpio_num_t)pin);
    }

    void PLATFORM_ISR_ATTR IsrWrite(Pin pin, bool level) {
        // inline register write (out_w1ts/out_w1tc, out1_* for pin >= 32), valid for all ESP32 variants
        gpio_ll_set_level(&GPIO, (gpio_num_t)pin, level ? 1 : 0);
    }

#else // ESP8266 (not implemented yet), PC

    void ConfigureOutput(Pin pin) { (void)pin; }

    void Release(Pin pin) {
#if defined(DALHAL_PLATFORM_ESP8266)
        if (pin != NC) { pinMode((uint8_t)pin, INPUT); }
#else
        (void)pin;
#endif
    }

    void Write(Pin pin, bool level) { (void)pin; (void)level; }
    bool Read(Pin pin) { (void)pin; return false; }
    bool AttachEdgeIsr(Pin pin, Edge edge, IsrHandler handler, void* arg) { (void)pin; (void)edge; (void)handler; (void)arg; return false; }
    void DetachIsr(Pin pin) { (void)pin; }
    void IsrEnable(Pin pin) { (void)pin; }
    void PLATFORM_ISR_ATTR IsrDisable(Pin pin) { (void)pin; }
    void PLATFORM_ISR_ATTR IsrWrite(Pin pin, bool level) { (void)pin; (void)level; }

#endif

}
}
