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

#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h> // need complete definition

#pragma once

namespace DALHAL {

    // forward declarations
    enum class HALOperationResult;
    class Device;
    class HALValue;
    class StringBuilderStreamer;

    namespace FunctionTypes {

        using Exec = HALOperationResult (*)(Device*);
        using ReadToHALValue = HALOperationResult (*)(Device* device, HALValue& outValue);
        using WriteHALValue = HALOperationResult (*)(Device* device, const HALValue& value);
        using BracketOpRead = HALOperationResult (*)(Device* device, const HALValue& subscriptValue, HALValue& outValue);
        using BracketOpWrite = HALOperationResult (*)(Device* device, const HALValue& subscriptValue, const HALValue& value);
        using ReadString = HALOperationResult (*)(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs);
        using WriteString = HALOperationResult (*)(Device* device, const ZeroCopyString& zcStrParameters, StringBuilderStreamer& sbs);

    }
}