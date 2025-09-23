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

#include <TimeLib.h>
#include <stdlib.h>
#include <string>
#include "Time_ext.h"

namespace Time_ext
{
    std::string formatNumber(int num) {
        return (num < 10) ? "0" + std::to_string(num) : std::to_string(num);
    }

    std::string formatTime(int hour, int minute, int second) {
        return formatNumber(hour) + ":" + formatNumber(minute) + ":" + formatNumber(second);
    }

    std::string GetTimeAsString(time_t time)
    {
        tmElements_t now2;
        breakTime(time, now2);
        std::string nowstr = std::to_string(now2.Year + 1970) + "-" +
                        std::to_string(now2.Month) + "-" +
                        std::to_string(now2.Day) + " " +
                        formatTime(now2.Hour, now2.Minute, now2.Second);
                        
        return nowstr;
    }
}