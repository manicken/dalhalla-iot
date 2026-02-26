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

namespace DALHAL {
    struct HAL_UID {
        static constexpr size_t Size = 8;
        static constexpr uint64_t UID_INVALID = 1;
        static constexpr uint64_t UID_NOT_SET = 0;
        union
        {
            char str[Size];
            uint64_t val;
            //uint32_t values[Size/4];
        };
        
        HAL_UID();
        HAL_UID(uint64_t v);
        /** basically: val == HAL_UID::UID_NOT_SET */
        bool NotSet();
        /** basically: (val != HAL_UID::UID_NOT_SET) && (val != HAL_UID::UID_INVALID) */
        bool IsSet();
        /** basically: val == HAL_UID::UID_INVALID */
        bool Invalid();
        HAL_UID& operator=(const HAL_UID& v);

    };
    bool operator==(const HAL_UID& lhs, const HAL_UID& rhs);
    bool operator!=(const HAL_UID& lhs, const HAL_UID& rhs);

    // Encode UID from ASCII string (up to 8 chars)
    HAL_UID encodeUID(const char* str);
    HAL_UID encodeUID(const ZeroCopyString& zcStr);
    HAL_UID encodeUID(const char* str, uint32_t count);
    std::string decodeUID(HAL_UID& uid);
}