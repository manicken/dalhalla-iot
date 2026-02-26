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

#define HAL_JSON_NOCOPY_NOMOVE(T) \
    T(const T&) = delete; \
    T& operator=(const T&) = delete; \
    T(T&&) = delete; T& operator=(T&&) = delete;

namespace HAL_JSON {

        template<typename T>
        void DeleteAs(void* ptr) {
            delete static_cast<T*>(ptr);
        }
        template<typename T1, typename T2>
        void DeleteAs(void* ptr1, void* ptr2) {
            delete static_cast<T1*>(ptr1);
            delete static_cast<T2*>(ptr2);
        }
        template<typename T>
        void DeleteAsArray(void* ptr) {
            delete[] static_cast<T*>(ptr);
        }
        
        typedef void (*Deleter)(void* context);
        typedef void (*Deleter2x)(void* context1,void* context2);
}