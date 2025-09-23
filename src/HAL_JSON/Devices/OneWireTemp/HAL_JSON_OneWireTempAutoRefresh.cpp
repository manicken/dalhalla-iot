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

#include "HAL_JSON_OneWireTempAutoRefresh.h"

namespace HAL_JSON {

    OneWireTempAutoRefresh::OneWireTempAutoRefresh(std::function<void()> _requestTemperatures, std::function<void()> _readAll, uint32_t _refreshTimeMs)
        : requestTemperatures(_requestTemperatures), readAll(_readAll) {
            SetRefreshTimeMs(_refreshTimeMs); // to include "convertion"
            if (!requestTemperatures || !readAll) {
                Serial.println(F("ERR@OWTAR")); // short error that can be searched for in code
                //abort();  // or throw if exceptions are enabled
            }
    }

    void OneWireTempAutoRefresh::SetRefreshTimeMs(uint32_t _refreshTimeMs) {
        if (_refreshTimeMs > HAL_JSON_ONE_WIRE_TEMP_CONVERSION_TIME_MS)
            refreshTimeMs = _refreshTimeMs - HAL_JSON_ONE_WIRE_TEMP_CONVERSION_TIME_MS;
        else
            refreshTimeMs = 0; // or some minimum value to avoid underflow
    }

	void OneWireTempAutoRefresh::loop() {
		uint32_t now = millis();
        switch (state) {
            case State::IDLE:
                if (now - lastUpdateMs >= refreshTimeMs) {
                    if (requestTemperatures != nullptr)
                        requestTemperatures();
                    //else
                    //    Serial.println(F("W@OWTAR - rtFP"));
                    state = State::WAITING_FOR_CONVERSION;
                    lastStart = now;
                }
                break;
            case State::WAITING_FOR_CONVERSION:
                if (now - lastStart >= HAL_JSON_ONE_WIRE_TEMP_CONVERSION_TIME_MS) {
                    if (readAll != nullptr)
                        readAll();
                    //else
                    //    Serial.println(F("W@OWTAR - raFP"));
                    lastUpdateMs = now;
                    state = State::IDLE;
                }
                break;
        }
	}

    

    String OneWireTempAutoRefresh::ToString() {
        String ret;
        ret += DeviceConstStrings::refreshTimeMs;
        ret += std::to_string(refreshTimeMs + HAL_JSON_ONE_WIRE_TEMP_CONVERSION_TIME_MS).c_str();
        return ret;
    }
}