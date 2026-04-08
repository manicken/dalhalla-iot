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

#pragma once



#include <ArduinoJson.h>
#include <stdlib.h>

#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>

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

#include <cstdint>

#undef OUT
#undef IN

#define DALHAL_CMD_EXEC_GPIO_LIST_MODE_STRING   "string"
#define DALHAL_CMD_EXEC_GPIO_LIST_MODE_HEX      "hex"
#define DALHAL_CMD_EXEC_GPIO_LIST_MODE_BINARY   "binary"
#define DALHAL_GPIO_MGR_PINFUNC_TYPE             uint16_t

namespace DALHAL {
    namespace GPIO_manager
    {
        namespace PinFunc {
           constexpr DALHAL_GPIO_MGR_PINFUNC_TYPE Reserved = 0x01;
           constexpr DALHAL_GPIO_MGR_PINFUNC_TYPE SpecialAtBoot = 0x02;
           constexpr DALHAL_GPIO_MGR_PINFUNC_TYPE LOW2BOOT = 0x04;
           constexpr DALHAL_GPIO_MGR_PINFUNC_TYPE HIGH2BOOT = 0x08;        
           constexpr DALHAL_GPIO_MGR_PINFUNC_TYPE OUT = 0x10;
           constexpr DALHAL_GPIO_MGR_PINFUNC_TYPE IN = 0x20;
           constexpr DALHAL_GPIO_MGR_PINFUNC_TYPE AIN = 0x40;
           constexpr DALHAL_GPIO_MGR_PINFUNC_TYPE AOUT = 0x80;
           constexpr DALHAL_GPIO_MGR_PINFUNC_TYPE UNDERSIDE = 0x100;
           constexpr DALHAL_GPIO_MGR_PINFUNC_TYPE UARTFLASH = 0x200;
           constexpr DALHAL_GPIO_MGR_PINFUNC_TYPE JTAG = 0x400;
        }

        /*enum class PinFunc : DALHAL_GPIO_MGR_PINFUNC_TYPE {
            
        };*/

        typedef struct {
            const char* Name;
            DALHAL_GPIO_MGR_PINFUNC_TYPE func;
        } PinFuncDef;

        enum class PrintListMode {
            String,
            Hex,
            Binary
        };

        enum class CheckPinResult {
            Success,
            NotFound,
            InUse,
            ModeMismatch
        };

        struct CheckPinResultError {
            const char* baseMsg;
            std::string msg;
            CheckPinResultError(const char* baseMsg, std::string msg) : baseMsg(baseMsg), msg(msg) {}
        };

        typedef struct {
            int8_t pin; // int8 should be enought for most targets as that mean 127 pins
            DALHAL_GPIO_MGR_PINFUNC_TYPE func;
        } gpio_pin;

        std::string describePinFunctions(DALHAL_GPIO_MGR_PINFUNC_TYPE pinFuncMask);
        
        CheckPinResult CheckIfPinAvailableAndIsFree_ThenReserve(uint8_t pin, DALHAL_GPIO_MGR_PINFUNC_TYPE pinFuncMask);
        /** this is a nice function that can be used */
        CheckPinResult CheckIfPinAvailableAndIsFree(uint8_t pin, DALHAL_GPIO_MGR_PINFUNC_TYPE pinFuncMask);
        CheckPinResultError GetCheckPinResultError(CheckPinResult res, uint8_t pin, DALHAL_GPIO_MGR_PINFUNC_TYPE pinFuncMask);
        void ClearAllReservations();
        /** it's recommended to call CheckIfPinAvailable prior to using this function,
         * this function is very basic and do only set the actual pin to reserved state, 
         * so calling it many times on the same pin do not matter */
        void ReservePin(uint8_t pin);

        void triStateAvailablePins();

        std::string GetList(ZeroCopyString& zcMode);
    }
}