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

#include "DALHAL_SCRIPT_ENGINE_Token.h"

#include "../../Support/DALHAL_Logger.h"

namespace DALHAL {
    namespace ScriptEngine {
        Token::Token() {
            Set(nullptr, nullptr, -1, -1);
        }
        void Token::Set(const char* _start, const char* _end, int _line, int _column) {
            start = _start;
            end = _end;
            line = _line;
            column = _column;
        }
        Token::~Token() {
            // nothing to do here as no dynamic memory is allocated
        }
        void Token::ReportTokenInfo(const char* msg, const char* param) const {
            std::string message = " (line " + std::to_string(line) + ", col " + std::to_string(column) + "): " + msg;
            if (param != nullptr)
                message += param;
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << "Info " << message << std::endl;
    #else
            //GlobalLogger.Info(F("Token:"), message.c_str());
    #endif
        }
        void Token::ReportTokenError(const char* msg, const char* param) const {
            std::string message = " (line " + std::to_string(line) + ", col " + std::to_string(column) + "): " + msg;
            if (param != nullptr)
                message += param;
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cerr << "Error " << message << std::endl;
    #else
            GlobalLogger.Error(F("Token:"), message.c_str());
    #endif
        }
        void Token::ReportTokenWarning(const char* msg, const char* param) const {
            std::string message = " (line " + std::to_string(line) + ", col " + std::to_string(column) + "): " + msg;
            if (param != nullptr)
                message += param;
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << "Warning " << message << std::endl;
    #else
            GlobalLogger.Warn(F("Token:"), message.c_str());
    #endif
        }
    }
}