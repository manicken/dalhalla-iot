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

#include "LittleFS_ext.h"
#include <iostream>
#include <chrono>

namespace LittleFS_ext {

    FileResult load_from_file(const char* file_name, char** outBuffer, size_t* outSize) {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::ifstream file(std::string(file_name), std::ios::binary | std::ios::ate);
        if (!file) {
            std::cout << "file not found:" << file_name << "\n";
            return FileResult::FileNotFound;
        } 

        *outSize = file.tellg();
        if (*outSize == 0) {
            std::cout << "file is empty:" << file_name << "\n";
            return FileResult::FileEmpty;
        }
        file.seekg(0);

        // Allocate mutable buffer (+1 for null terminator if needed)
        char* buffer = new char[*outSize + 1];
        
        file.read(buffer, *outSize);
        buffer[*outSize] = '\0'; // optional if you need null-terminated text
        *outBuffer = buffer;

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> duration = end - start;

        std::cout << " ***************** File read time: " << duration.count() << " us\n";
        return FileResult::Success;
    }
}