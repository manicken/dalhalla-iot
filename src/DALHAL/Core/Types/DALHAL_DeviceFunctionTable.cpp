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

#include "DALHAL_DeviceFunctionTable.h"

namespace DALHAL {
    
    std::string DeviceFunctionTable::ToString() const{
        std::string ret;
        ret += "\"readValue\":"; ret += GetDeviceFunctions(readValue);
        ret += ",\"writeValue\":"; ret += GetDeviceFunctions(writeValue);
        ret += ",\"exec\":"; ret += GetDeviceFunctions(exec);
        ret += ",\"readString\":"; ret += GetDeviceFunctions(readString);
        ret += ",\"writeString\":"; ret += GetDeviceFunctions(writeString);
        ret += ",\"bracketOpRead\":"; ret += GetDeviceFunctions(bracketOpRead);
        ret += ",\"bracketOpWrite\":"; ret += GetDeviceFunctions(bracketOpWrite);
        return ret;
    }

    std::string FunctionValueType::ToString(DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE mask) {
        std::string ret;
        bool notFirst = false;
        ret += '[';
        if (FunctionValueType::HasFlag(mask, FunctionValueType::_UInt_)) { notFirst = true; ret += "\"uint\""; }
        if (notFirst) { ret += ','; } if (FunctionValueType::HasFlag(mask, FunctionValueType::_Int_)) { notFirst = true; ret += "\"int\""; }
        if (notFirst) { ret += ','; } if (FunctionValueType::HasFlag(mask, FunctionValueType::_Float_)) { notFirst = true; ret += "\"float\""; }
        if (notFirst) { ret += ','; } if (FunctionValueType::HasFlag(mask, FunctionValueType::_Bool_)) { notFirst = true; ret += "\"bool\""; }
        ret += ']';
        return ret;
    }
}