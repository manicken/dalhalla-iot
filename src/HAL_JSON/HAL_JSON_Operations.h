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

#include <Arduino.h>

#include <string>

#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"

namespace HAL_JSON {
    struct HALWriteValueByCmd {
        const HALValue& value;
        //const std::string& cmd;
        const ZeroCopyString& cmd;
        HALWriteValueByCmd(const HALValue& value, const ZeroCopyString& cmd): value(value), cmd(cmd) {}
    };
    /*struct HALWriteValueByCmdReq { // obsolete
        UIDPath& path;
        const HALWriteValueByCmd& valByCmd;
        HALWriteValueByCmdReq(UIDPath& p, const HALWriteValueByCmd& valByCmd) : path(p), valByCmd(valByCmd) {}
    };*/
    enum class HALOperationResult {
        Success = 0,
        DeviceNotFound = 1,
        UnsupportedOperation = 2,  // was OperationNotSupported
        UnsupportedCommand = 3,   // was InvalidCommand
        ExecutionFailed = 4,       // was OperationFail
        StringRequestParameterError = 5,
        WriteValueNaN = 6,

        /** script engine specific status */
        IfConditionTrue = 10,
        /** script engine specific status */
        IfConditionFalse = 11,
        /** script engine specific error */
        StackUnderflow = 19,
        /** script engine specific error */
        DivideByZero = 20,
        /** script engine specific error */
        StackOverflow = 21,
        /** script engine specific error */
        ResultGetFail = 22,
        /** script engine specific error */
        HandlerWasNullPtr = 23,
        /** script engine specific error */
        HandlerWasDummy = 24,
        /** script engine specific error, 
         * this should only be fired from 
         * within a handler function 
         * if it's required to be set */
        ContextWasNullPtr = 25,
        /** script engine specific error */
        BracketOpSubscriptOutOffRange = 26,
        /** script engine specific error */
        BracketOpSubscriptInvalid = 27,

        NotSet = 99,
    };
    const char* HALOperationResultToString(HALOperationResult result);

    struct HALReadValueByCmd {
        HALValue& out_value;
        //const std::string& cmd;
        const ZeroCopyString& cmd;
        HALReadValueByCmd(HALValue& out, const ZeroCopyString& cmd): out_value(out), cmd(cmd) {}
    };

    /*struct HALReadValueByCmdReq { // obsolete
        UIDPath& path; // need to be non const as it's mutable
        const HALReadValueByCmd& valByCmd;
        HALReadValueByCmdReq(UIDPath& p, const HALReadValueByCmd& valByCmd) : path(p), valByCmd(valByCmd) {}
    };*/

    /*struct HALReadRequest { // obsolete
        UIDPath& path; // need to be non const as it's mutable
        HALValue& out_value;
        HALReadRequest(UIDPath& p, HALValue& out) : path(p), out_value(out) {}
    };*/

    /*struct HALWriteRequest { // obsolete
        UIDPath& path; // need to be non const as it's mutable
        const HALValue& value;
        HALWriteRequest(UIDPath& p, const HALValue& val) : path(p), value(val) {}
    };*/

    struct HALReadStringRequestValue {
        std::string& out_value;
        const ZeroCopyString& cmd;
        HALReadStringRequestValue(const ZeroCopyString& cmd, std::string& out_value): out_value(out_value), cmd(cmd) {}
    };

    struct HALWriteStringRequestValue {
        const ZeroCopyString& value;
        std::string& result;
        HALWriteStringRequestValue(const ZeroCopyString& value, std::string& result): value(value), result(result) {}
    };

    /*struct HALReadStringRequest { // obsolete
        UIDPath& path; // need to be non const as it's mutable
        HALReadStringRequestValue& value;
        HALReadStringRequest(UIDPath& p, HALReadStringRequestValue& value) : path(p), value(value) {}
    };*/

    /*struct HALWriteStringRequest { // obsolete
        UIDPath& path; // need to be non const as it's mutable
        HALWriteStringRequestValue& value;
        HALWriteStringRequest(UIDPath& p, HALWriteStringRequestValue& value) : path(p), value(value) {}
    };*/

    struct HALStringRequestValue {
        const ZeroCopyString& output;
        std::string& input;
        HALStringRequestValue(const ZeroCopyString& output, std::string& input): output(output), input(input) {}
    };

    /*struct HALStringRequest { // obsolete
        UIDPath& path; // need to be non const as it's mutable
        HALStringRequestValue& value;
        HALStringRequest(UIDPath& p, HALStringRequestValue& value) : path(p), value(value) {}
    };*/
}