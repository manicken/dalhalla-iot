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
    
    void DeviceFunctionTable::PrintTo(DALHAL::StreamWriter& sw) const{
        
        sw.write("\"readValue\":"); GetDeviceFunctions(readValue, sw);
        sw.write(",\"writeValue\":"); GetDeviceFunctions(writeValue, sw);
        sw.write(",\"exec\":"); GetDeviceFunctions(exec, sw);
        sw.write(",\"readString\":"); GetDeviceFunctions(readString, sw);
        sw.write(",\"writeString\":"); GetDeviceFunctions(writeString, sw);
        sw.write(",\"bracketOpRead\":"); GetDeviceFunctions(bracketOpRead, sw);
        sw.write(",\"bracketOpWrite\":"); GetDeviceFunctions(bracketOpWrite, sw);
    }

    void FunctionValueType::PrintTo(DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE mask, DALHAL::StreamWriter& sw) {

        bool notFirst = false;
        sw.write('[');
        if (FunctionValueType::HasFlag(mask, FunctionValueType::_UInt_)) { notFirst = true; sw.write("\"uint\""); }
        if (notFirst) { sw.write(','); } if (FunctionValueType::HasFlag(mask, FunctionValueType::_Int_)) { notFirst = true; sw.write("\"int\""); }
        if (notFirst) { sw.write(','); } if (FunctionValueType::HasFlag(mask, FunctionValueType::_Float_)) { notFirst = true; sw.write("\"float\""); }
        if (notFirst) { sw.write(','); } if (FunctionValueType::HasFlag(mask, FunctionValueType::_Bool_)) { notFirst = true; sw.write("\"bool\""); }
        sw.write(']');
    }
}