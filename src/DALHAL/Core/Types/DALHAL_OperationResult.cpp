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

#include <cstdio>

#include "DALHAL_OperationResult.h"
#include <Arduino.h>

namespace DALHAL {

    const __FlashStringHelper* HALOperationResultToString(HALOperationResult result) {
        switch (result) {
            case HALOperationResult::Success: return F("Success");
            case HALOperationResult::DeviceNotFound: return F("DeviceNotFound");
            case HALOperationResult::UnsupportedOperation: return F("UnsupportedOperation");
            case HALOperationResult::UnsupportedCommand: return F("UnsupportedCommand");
            case HALOperationResult::ExecutionFailed: return F("ExecutionFailed");
            case HALOperationResult::IfConditionTrue: return F("IfConditionTrue");
            case HALOperationResult::IfConditionFalse: return F("IfConditionFalse");
            case HALOperationResult::StackUnderflow: return F("StackUnderflow");
            case HALOperationResult::DivideByZero: return F("DivideByZero");
            case HALOperationResult::StackOverflow: return F("StackOverflow");
            case HALOperationResult::ResultGetFail: return F("ResultGetFail");
            case HALOperationResult::HandlerWasNullPtr: return F("HandlerWasNullPtr");
            case HALOperationResult::ContextWasNullPtr: return F("ContextWasNullPtr");
            case HALOperationResult::BracketOpSubscriptOutOffRange: return F("BracketOpSubscriptOutOffRange");
            case HALOperationResult::StringRequestParameterError: return F("StringRequestParameterError");
            case HALOperationResult::WriteValueNaN: return F("WriteValueNaN");
            case HALOperationResult::WriteValueNotUintOrInt: return F("WriteValueNotUintOrInt");
            case HALOperationResult::WriteValueOutOfRange: return F("WriteValueOutOfRange");
            case HALOperationResult::InvalidArgument: return F("InvalidArgument");
            case HALOperationResult::HardwareFault: return F("HardwareFault");
            case HALOperationResult::Timeout: return F("Timeout");
            case HALOperationResult::NotSet: return F("NotSet");
            case HALOperationResult::ReactiveEventsNotSupported: return F("ReactiveEventsNotSupported");
            case HALOperationResult::ReactiveEventByNameNotFound: return F("ReactiveEventByNameNotFound");
            
            default:
                Serial.print("");// force the compiler to not use lockup table which consume 400 bytes ram
                return F("Unknown");
        }
    }
}