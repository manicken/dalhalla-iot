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

#include <cstdint>
#include "HAL_JSON_Device.h"

#define HAL_JSON_DEVICE_SIMPLE_EVENT_DEVICE

namespace HAL_JSON
{
    class SimpleEventDevice : public Device {
    public:
        struct Context {
            uint32_t lastSeen;
            uint32_t& current;
            Context(uint32_t& current);
        };

    protected:
        uint32_t eventCounter;      // private to event devices only

    public:


        SimpleEventDevice(const char* type) : Device(type) {}
        static bool EventCheck(void* context);
        HALOperationResult Get_DeviceEvent(ZeroCopyString& zcStrFuncName, DeviceEvent** deviceEvent) override;
    
        inline void triggerEvent() {
            ++eventCounter;
        }

        inline uint32_t getEventCounter() const {
            return eventCounter;
        }
    };
} // namespace HAL_JSON
