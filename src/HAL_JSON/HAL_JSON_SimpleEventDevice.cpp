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

#include "HAL_JSON_SimpleEventDevice.h"

namespace HAL_JSON
{
    bool SimpleEventDevice::EventCheck(Device* dev, uint32_t& lastSeen) {
        auto* d = static_cast<SimpleEventDevice*>(dev);
        if (d->eventCounter != lastSeen) {
            lastSeen = d->eventCounter; // update to current
            return true;
        }
        return false;
    }

    Device::EventCheck_FuncType SimpleEventDevice::Get_EventCheck_Function(ZeroCopyString&)
    {
        //return &SimpleEventDevice::EventCheck;
        return nullptr;
    }

} // namespace HAL_JSON