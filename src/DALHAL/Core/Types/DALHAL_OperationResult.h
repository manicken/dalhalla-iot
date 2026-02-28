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

namespace DALHAL {

    enum class HALOperationResult {
        Success = 0,
        DeviceNotFound = 1,
        UnsupportedOperation = 2,  // was OperationNotSupported
        UnsupportedCommand = 3,   // was InvalidCommand
        ExecutionFailed = 4,       // was OperationFail
        StringRequestParameterError = 5,
        WriteValueNaN = 6,
        WriteValueNotUintOrInt = 7,
        WriteValueOutOfRange = 8,
        InvalidArgument = 9,
        HardwareFault = 10,
        Timeout = 11,
        ReactiveEventsNotSupported = 12,
        ReactiveEventByNameNotFound = 13,

        /** script engine specific status */
        IfConditionTrue = 20,
        /** script engine specific status */
        IfConditionFalse = 21,
        /** script engine specific error */
        StackUnderflow = 22,
        /** script engine specific error */
        DivideByZero = 23,
        /** script engine specific error */
        StackOverflow = 24,
        /** script engine specific error */
        ResultGetFail = 25,
        /** script engine specific error */
        HandlerWasNullPtr = 26,
        /** script engine specific error */
        HandlerWasDummy = 27,
        /** script engine specific error, 
         * this should only be fired from 
         * within a handler function 
         * if it's required to be set */
        ContextWasNullPtr = 28,
        /** script engine specific error */
        BracketOpSubscriptOutOffRange = 29,
        /** script engine specific error */
        BracketOpSubscriptInvalid = 30,

        NotSet = 99,
    };
    const char* HALOperationResultToString(HALOperationResult result);

  }