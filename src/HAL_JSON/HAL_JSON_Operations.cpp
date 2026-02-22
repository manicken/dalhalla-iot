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

#include "HAL_JSON_Operations.h"

namespace HAL_JSON {
    const char* HALOperationResultToString(HALOperationResult result) {
        switch (result) {
            case HALOperationResult::Success: return "Success";
            case HALOperationResult::DeviceNotFound: return "DeviceNotFound";
            case HALOperationResult::UnsupportedOperation: return "UnsupportedOperation";
            case HALOperationResult::UnsupportedCommand: return "UnsupportedCommand";
            case HALOperationResult::ExecutionFailed: return "ExecutionFailed";
            case HALOperationResult::IfConditionTrue: return "IfConditionTrue";
            case HALOperationResult::IfConditionFalse: return "IfConditionFalse";
            case HALOperationResult::StackUnderflow: return "StackUnderflow";
            case HALOperationResult::DivideByZero: return "DivideByZero";
            case HALOperationResult::StackOverflow: return "StackOverflow";
            case HALOperationResult::ResultGetFail: return "ResultGetFail";
            case HALOperationResult::HandlerWasNullPtr: return "HandlerWasNullPtr";
            case HALOperationResult::ContextWasNullPtr: return "ContextWasNullPtr";
            case HALOperationResult::BracketOpSubscriptOutOffRange: return "BracketOpSubscriptOutOffRange";
            case HALOperationResult::StringRequestParameterError: return "StringRequestParameterError";
            case HALOperationResult::WriteValueNaN: return "WriteValueNaN";
            case HALOperationResult::WriteValueNotUintOrInt: return "WriteValueNotUintOrInt";
            case HALOperationResult::WriteValueOutOfRange: return "WriteValueOutOfRange";
            case HALOperationResult::InvalidArgument: return "InvalidArgument";
            case HALOperationResult::HardwareFault: return "HardwareFault";
            case HALOperationResult::Timeout: return "Timeout";
            case HALOperationResult::NotSet: return "NotSet";
            case HALOperationResult::DeviceEventsNotSupported: return "DeviceEventsNotSupported";
            case HALOperationResult::DeviceEventByNameNotFound: return "DeviceEventByNameNotFound";
            
            default:
                printf("unknown HALOperationResult: %d\n", (int)result);
                return "Unknown";
        }
    }
}