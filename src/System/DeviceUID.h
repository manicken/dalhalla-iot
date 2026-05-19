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

#pragma once
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

struct DeviceUID {
private:  
  static char g_deviceUID[13];
  static bool initialized;
public:
    static void Init();
    static void Set(const char* uid);
    inline static const char* Get() {
        return g_deviceUID;
    }
};

struct DALHAL_DeviceUID {
    uint32_t unitDeviceUID_MSB = 0;
    uint32_t unitDeviceUID_LSB = 0;

    DALHAL_DeviceUID(uint64_t uid) : 
      unitDeviceUID_MSB((uint32_t)((uid>>32) & 0x0000FFFF)), 
      unitDeviceUID_LSB((uint32_t)(uid & 0xFFFFFFFF))
      {}
};
DALHAL_DeviceUID getDALHAL_DeviceUID();
// Returns 48-bit device UID packed in uint64_t (upper 16 bits zero)
uint64_t getDeviceUID();


/** Converts the UID to a 12-character hex string (stack-only), 
 * can be used as a example how it should be converted 
 * as it's preffered to convert the uid value directly using a "bigger" sprintf
 */
void uidToHex(uint64_t uid, char* buffer); // buffer must be at least 13 bytes

#ifdef __cplusplus
}
#endif