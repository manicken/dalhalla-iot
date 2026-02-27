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



#include <ArduinoJson.h>
#include <stdlib.h>

#include "../Types/DALHAL_ZeroCopyString.h"

/*
    this file is only intended to manage which functions that are assigned to a specific GPIO pin
    that table is stored into a JSON file
    this will also contain the webserver functions, such as retrival of available GPIO pins

    in the client UI:
    GPIO pin selection should be from a predefined list of available GPIO:s
    so that GPIO:s that are assigned to fixed hardware functions such as
    I2C,UART,SPI,SD-card, etc.
    do not collide 
*/

#include <Arduino.h> // Needed for String class

#undef OUT
#undef IN

#define DALHAL_CMD_EXEC_GPIO_LIST_MODE_STRING   "string"
#define DALHAL_CMD_EXEC_GPIO_LIST_MODE_HEX      "hex"
#define DALHAL_CMD_EXEC_GPIO_LIST_MODE_BINARY   "binary"
#define DALHAL_GPIO_MGR_PINFUNC_TYPE            uint16_t

namespace DALHAL {
    namespace GPIO_manager
    {
        

        enum class PinFunc : DALHAL_GPIO_MGR_PINFUNC_TYPE {
            Reserved = 0x01,
            SpecialAtBoot = 0x02,
            LOW2BOOT = 0x04,
            HIGH2BOOT = 0x08,        
            OUT = 0x10,
            IN = 0x20,
            AIN = 0x40,
            AOUT = 0x80,
            UNDERSIDE = 0x100,
            UARTFLASH = 0x200,
            JTAG = 0x400,
        };

        typedef struct {
            const char* Name;
            DALHAL_GPIO_MGR_PINFUNC_TYPE func;
        } PinFuncDef;

        enum class PrintListMode {
            String,
            Hex,
            Binary
        };

        typedef struct {
            uint8_t pin;
            DALHAL_GPIO_MGR_PINFUNC_TYPE mode;
        } gpio_pin;

        extern const gpio_pin available_gpio_list[];
        extern int available_gpio_list_lenght;
        void set_available_gpio_list_length();

        extern const PinFuncDef PinModeStrings[];
        extern int PinModeStrings_length;
        void set_PinModeStrings_length();

        std::string describePinFunctions(DALHAL_GPIO_MGR_PINFUNC_TYPE pinFuncMask);
        
        bool CheckIfPinAvailableAndReserve(uint8_t pin, DALHAL_GPIO_MGR_PINFUNC_TYPE pinFunc);
        /** this is a nice function that can be used */
        bool ValidateJsonAndCheckIfPinAvailableAndReserve(const JsonVariant& jsonObj, DALHAL_GPIO_MGR_PINFUNC_TYPE pinFuncMask);
        bool ValidateJsonAndCheckIfPinAvailableAndReserve(const JsonVariant& jsonObj, const char* NAME, DALHAL_GPIO_MGR_PINFUNC_TYPE pinFuncMask);
        bool CheckIfPinAvailable(uint8_t pin, DALHAL_GPIO_MGR_PINFUNC_TYPE pinFuncMask);
        void ClearAllReservations();
        /** it's recommended to call CheckIfPinAvailable prior to using this function,
         * this function is very basic and do only set the actual pin to reserved state, 
         * so calling it many times on the same pin do not matter */
        void ReservePin(uint8_t pin);

        void triStateAvailablePins();

        std::string GetList(ZeroCopyString& zcMode);
    }
}