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
#include <cstring>
#include <string>

#include "DALHAL_ZeroCopyString.h"

#include "DALHAL_UID.h"

namespace DALHAL {

    class UIDPath {
    private:
        HAL_UID* items = nullptr;

        //HAL_UID* items = nullptr; // future implementation so that encodeUID and decodeUID won't be needed
        uint32_t itemCount = 0;
        uint32_t currentItemIndex = 0;

    public:
        enum class ToStringType {
            String,
            Raw
        };
        /** used to validate a uid path before create it */
        static bool Validate(const ZeroCopyString& uidzcStr);

        UIDPath();
        ~UIDPath();
        UIDPath(const char* uidStr);
        UIDPath(const ZeroCopyString& uidzcStr);
        UIDPath(const std::string& uidStr);
        UIDPath(const UIDPath& other) = delete; // Copy constructor
        UIDPath& operator=(const UIDPath& other) = delete; // Copy assignment
        UIDPath(UIDPath&& other) = delete; // Move constructor
        UIDPath& operator=(UIDPath&& other) = delete; // Move assignment
        /** Ideally, the program should be designed to avoid needing this function. 
         * but the only place where it should be used is at start of DeviceManager::findDevice 
         * that way we never need to check it while using other functions
         */
        bool empty() const;
        
        uint32_t count();
        HAL_UID getCurrentUID();
        HAL_UID resetAndGetFirst();
        void reset();
        HAL_UID getNextUID();
        HAL_UID peekNextUID();
        bool isLast();
        bool hasMore();

        std::string ToString(ToStringType type = ToStringType::String);

    };

}