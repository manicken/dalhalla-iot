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
        
        sbs.write(F("\"readValue\":")); GetDeviceFunctions(readValue, sbs);
        sbs.write(F(",\"writeValue\":")); GetDeviceFunctions(writeValue, sbs);
        sbs.write(F(",\"exec\":")); GetDeviceFunctions(exec, sbs);
        sbs.write(F(",\"readString\":")); GetDeviceFunctions(readString, sbs);
        sbs.write(F(",\"writeString\":")); GetDeviceFunctions(writeString, sbs);
        sbs.write(F(",\"bracketOpRead\":")); GetDeviceFunctions(bracketOpRead, sbs);
        sbs.write(F(",\"bracketOpWrite\":")); GetDeviceFunctions(bracketOpWrite, sbs);
    }

    void FunctionValueType::PrintTo(DALHAL_FUNCTIONTABLE_VALUETYPE_TYPE mask, DALHAL::StringBuilderStreamer& sbs) {

        bool notFirst = false;
        sbs.write_json_array_begin();
        if (FunctionValueType::HasFlag(mask, FunctionValueType::_UInt_)) { notFirst = true; sbs.write_jsonQuoted(F("uint")); }
        if (FunctionValueType::HasFlag(mask, FunctionValueType::_Int_)) { if (notFirst) { sbs.write_json_value_separator(); } notFirst = true; sbs.write_jsonQuoted(F("int")); }
        if (FunctionValueType::HasFlag(mask, FunctionValueType::_Float_)) { if (notFirst) { sbs.write_json_value_separator(); } notFirst = true; sbs.write_jsonQuoted(F("float")); }
        if (FunctionValueType::HasFlag(mask, FunctionValueType::_Bool_)) { if (notFirst) { sbs.write_json_value_separator(); } notFirst = true; sbs.write_jsonQuoted(F("bool")); }
        sbs.write_json_array_end();
    }
}