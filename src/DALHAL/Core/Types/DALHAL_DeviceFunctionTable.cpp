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
    
    void DeviceFunctionTable::PrintTo(DALHAL::StringBuilderStreamer& sbs) const{
        
        sbs.write("\"readValue\":"); GetDeviceFunctions(readValue, sbs);
        sbs.write(",\"writeValue\":"); GetDeviceFunctions(writeValue, sbs);
        sbs.write(",\"exec\":"); GetDeviceFunctions(exec, sbs);
        sbs.write(",\"readString\":"); GetDeviceFunctions(readString, sbs);
        sbs.write(",\"writeString\":"); GetDeviceFunctions(writeString, sbs);
        sbs.write(",\"bracketOpRead\":"); GetDeviceFunctions(bracketOpRead, sbs);
        sbs.write(",\"bracketOpWrite\":"); GetDeviceFunctions(bracketOpWrite, sbs);
    }

    void FunctionValueType::PrintTo(DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE mask, DALHAL::StringBuilderStreamer& sbs) {

        bool notFirst = false;
        sbs.write('[');
        if (FunctionValueType::HasFlag(mask, FunctionValueType::_UInt_)) { notFirst = true; sbs.write("\"uint\""); }
        if (notFirst) { sbs.write(','); } if (FunctionValueType::HasFlag(mask, FunctionValueType::_Int_)) { notFirst = true; sbs.write("\"int\""); }
        if (notFirst) { sbs.write(','); } if (FunctionValueType::HasFlag(mask, FunctionValueType::_Float_)) { notFirst = true; sbs.write("\"float\""); }
        if (notFirst) { sbs.write(','); } if (FunctionValueType::HasFlag(mask, FunctionValueType::_Bool_)) { notFirst = true; sbs.write("\"bool\""); }
        sbs.write(']');
    }
}