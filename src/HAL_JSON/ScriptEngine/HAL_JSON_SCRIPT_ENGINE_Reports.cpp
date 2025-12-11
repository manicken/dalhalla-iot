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

#include "HAL_JSON_SCRIPT_ENGINE_Reports.h"
#include "../../Support/Logger.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        void ReportError(const char* msg) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << "Error: " << msg << std::endl;
#else
            GlobalLogger.Error(F("Report:"), msg);
#endif
        }

        void ReportWarning(const char* msg, const char* param) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << "Warning: " << msg << " " << ((param!=nullptr)?param:"") << std::endl;
#else
            GlobalLogger.Warn(F("Report:"), msg);
#endif
        }

#if (defined(_WIN32) || defined(__linux__) || defined(__APPLE__)) && defined(DEBUG_PRINT_SCRIPT_ENGINE) || defined(DEBUG_PRINT_SCRIPT_ENGINE)
        void ReportInfo(std::string msg) {
            printf("%s", msg.c_str());
        }
#endif
    }
}