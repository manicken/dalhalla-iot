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

#include <string>

#include "DALHAL_Value.h"
#include "DALHAL_UID_Path.h"

namespace DALHAL {

    struct HALWriteValueByCmd {
        const HALValue& value;
        const ZeroCopyString& cmd;
        HALWriteValueByCmd(const HALValue& value, const ZeroCopyString& cmd): value(value), cmd(cmd) {}
    };

    struct HALReadValueByCmd {
        HALValue& out_value;
        const ZeroCopyString& cmd;
        HALReadValueByCmd(HALValue& out, const ZeroCopyString& cmd): out_value(out), cmd(cmd) {}
    };

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

}