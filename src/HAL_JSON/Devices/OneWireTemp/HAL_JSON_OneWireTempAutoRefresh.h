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


#include <Arduino.h> // Needed for String class
#include <functional>

#define HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS 1000
#define HAL_JSON_ONE_WIRE_TEMP_CONVERSION_TIME_MS 1000

namespace HAL_JSON {

    class OneWireTempAutoRefresh {
        typedef void (*Callback)();

    private:
        enum class State { IDLE, WAITING_FOR_CONVERSION };
        
        uint32_t lastUpdateMs = 0;

        State state = State::IDLE;
        uint32_t lastStart = 0;

        std::function<void()> requestTemperatures;
        std::function<void()> readAll;

    protected:
        uint32_t refreshTimeMs = HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS;


    public:
        OneWireTempAutoRefresh() = delete;
        OneWireTempAutoRefresh(OneWireTempAutoRefresh&) = delete;
        OneWireTempAutoRefresh(std::function<void()> _requestTemperatures, std::function<void()> _readAll, uint32_t _refreshTimeMs);
        void SetRefreshTimeMs(uint32_t _refreshTimeMs);
        
        void loop();

        String ToString();
    };

    
}