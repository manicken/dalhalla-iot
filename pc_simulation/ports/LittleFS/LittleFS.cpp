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

#include "LittleFS.h"


#ifdef _WIN32
    #include <windows.h>
    
#else
    #include <sys/stat.h>
    
#endif

LittleFS_class::LittleFS_class() {}

bool LittleFS_class::exists(const char* path) {
    std::ifstream file(path);
    return file.good();    
}

bool LittleFS_class::mkdir(const char *path) {
#ifdef _WIN32
    CreateDirectoryA(path, NULL);
#else // linux
    mkdir(path, 0755);
#endif
}

LittleFS_class LittleFS;