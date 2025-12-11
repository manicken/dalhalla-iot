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

#include "Logger.h"

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream>
#endif

Logger GlobalLogger;

#define LOGGER_GET_TIME time(nullptr)

//    ██       ██████   ██████      ███████ ███    ██ ████████ ██████  ██    ██ 
//    ██      ██    ██ ██           ██      ████   ██    ██    ██   ██  ██  ██  
//    ██      ██    ██ ██   ███     █████   ██ ██  ██    ██    ██████    ████   
//    ██      ██    ██ ██    ██     ██      ██  ██ ██    ██    ██   ██    ██    
//    ███████  ██████   ██████      ███████ ██   ████    ██    ██   ██    ██    

LogEntry::LogEntry() : timestamp(0),
      level(Loglevel::Info),
      errorCode(0),
      text(nullptr),
      isCode(true),
      source(nullptr) {}

    bool LogEntry::isEqual(Loglevel lvl, uint32_t err, const __FlashStringHelper* msg, const char* txt, bool codeFlag) const 
    {
        if (level != lvl) return false;
        if (isCode != codeFlag) return false;

        if (isCode) {
            if (errorCode != err) return false;
        } else {
            if (message != msg) return false;        // pointer compare, efficient
        }

        // Compare text (both null or both equal)
        if (txt == nullptr && text == nullptr) return true;
        if (txt == nullptr || text == nullptr) return false;
        return strcmp(text, txt) == 0;
    }
    bool LogEntry::isEqual(Loglevel lvl, uint32_t err, const __FlashStringHelper* msg, const HAL_JSON::ZeroCopyString& zcStr, bool codeFlag) const 
    {
        if (level != lvl) return false;
        if (isCode != codeFlag) return false;

        if (isCode) {
            if (errorCode != err) return false;
        } else {
            if (message != msg) return false;        // pointer compare, efficient
        }

        // Compare text (both null or both equal)
        if (zcStr.IsEmpty() && text == nullptr) return true;
        if (zcStr.IsEmpty() || text == nullptr) return false;
        return zcStr.Equals(text);
    }

void LogEntry::Set(time_t time, Loglevel _level, uint32_t _errorCode) {
    timestamp = time;
    level = _level;
    errorCode = _errorCode;
    if (text != nullptr) { free(text); text = nullptr; }
    isCode = true;
    isNew = true;
    source = nullptr;
    repeatCount = 0;
}
void LogEntry::Set(time_t time, Loglevel _level, const __FlashStringHelper* _message) {
    timestamp = time;
    level = _level;
    message = _message;
    if (text != nullptr) { free(text); text = nullptr; }
    isCode = false;
    isNew = true;
    source = nullptr;
    repeatCount = 0;
}
void LogEntry::Set(time_t time, Loglevel _level, uint32_t _errorCode, const char* _text) {
    timestamp = time;
    level = _level;
    errorCode = _errorCode;
    if (text != nullptr) { free(text); text = nullptr; }
    if (_text) {
        text = strdup(_text);  // <-- heap-allocate and copy the string
    } else {
        text = nullptr;
    }
    isCode = true;
    isNew = true;
    source = nullptr;
    repeatCount = 0;
}
void LogEntry::Set(time_t time, Loglevel _level, const __FlashStringHelper* _message, const char* _text) {
    timestamp = time;
    level = _level;
    message = _message;
    if (text != nullptr) { free(text); text = nullptr; }
    if (_text) {
        text = strdup(_text);  // <-- heap-allocate and copy the string
    } else {
        text = nullptr;
    }
    isCode = false;
    isNew = true;
    source = nullptr;
    repeatCount = 0;
}
void LogEntry::Set(time_t time, Loglevel _level, uint32_t _errorCode, const HAL_JSON::ZeroCopyString& zcStr) {
    timestamp = time;
    level = _level;
    errorCode = _errorCode;
    if (text != nullptr) { free(text); text = nullptr; }

    // safe to just use the following as when zcStr is empty it returns nullptr
    text = zcStr.ToCharString();  // <-- heap-allocate using malloc and copy the string

    isCode = true;
    isNew = true;
    source = nullptr;
    repeatCount = 0;
}
void LogEntry::Set(time_t time, Loglevel _level, const __FlashStringHelper* _message, const HAL_JSON::ZeroCopyString& zcStr) {
    timestamp = time;
    level = _level;
    message = _message;
    if (text != nullptr) { free(text); text = nullptr; }

    // safe to just use the following as when zcStr is empty it returns nullptr
    text = zcStr.ToCharString();  // <-- heap-allocate using malloc and copy the string

    isCode = false;
    isNew = true;
    source = nullptr;
    repeatCount = 0;
}
LogEntry::~LogEntry() {
    if (text) {
        free(text);
        text = nullptr;
    }
}
String LogEntry::MessageToString() const {
    String result;
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
    if (source != nullptr)
        std::cout << "source:" << source << "\n";
#endif
    result += (source != nullptr) ? ("[" + String(source) + "]") : "";
    result += ((message != nullptr) ? String(message) : "<entry error>");
    result += (text != nullptr) ? String(text) : "";
    return result;
}

//    ██       ██████   ██████   ██████  ███████ ██████  
//    ██      ██    ██ ██       ██       ██      ██   ██ 
//    ██      ██    ██ ██   ███ ██   ███ █████   ██████  
//    ██      ██    ██ ██    ██ ██    ██ ██      ██   ██ 
//    ███████  ██████   ██████   ██████  ███████ ██   ██ 

Logger::Logger() {

}

void Logger::Error(uint32_t code) {
    if (UpdateLastEntryIfEqual(Loglevel::Error, code, nullptr, nullptr, true))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, code);
    advance();
}
void Logger::Error(const __FlashStringHelper* msg) {
    if (UpdateLastEntryIfEqual(Loglevel::Error, 0, msg, nullptr, false))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, msg);
    advance();
}
void Logger::Error(uint32_t code, const char* text) {
    if (UpdateLastEntryIfEqual(Loglevel::Error, code, nullptr, text, true))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, code, text);
    advance();
}
void Logger::Error(const __FlashStringHelper* msg, const char* text) {
    if (UpdateLastEntryIfEqual(Loglevel::Error, 0, msg, text, false))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, msg, text);
    advance();
}
void Logger::Error(const __FlashStringHelper* msg, const HAL_JSON::ZeroCopyString& zcStr) {
    if (UpdateLastEntryIfEqual(Loglevel::Error, 0, msg, zcStr, false))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, msg, zcStr);
    advance();
}

void Logger::Info(uint32_t code) {
    if (UpdateLastEntryIfEqual(Loglevel::Info, code, nullptr, nullptr, true))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, code);
    advance();
}
void Logger::Info(const __FlashStringHelper* msg) {
    if (UpdateLastEntryIfEqual(Loglevel::Info, 0, msg, nullptr, false))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, msg);
    advance();
}
void Logger::Info(uint32_t code, const char* text) {
    if (UpdateLastEntryIfEqual(Loglevel::Info, code, nullptr, text, true))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, code, text);
    advance();
}
void Logger::Info(const __FlashStringHelper* msg, const char* text) {
    if (UpdateLastEntryIfEqual(Loglevel::Info, 0, msg, text, false))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, msg, text);
    advance();
}
void Logger::Info(const __FlashStringHelper* msg, const HAL_JSON::ZeroCopyString& zcStr) {
    if (UpdateLastEntryIfEqual(Loglevel::Info, 0, msg, zcStr, false))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, msg, zcStr);
    advance();
}

void Logger::Warn(uint32_t code) {
    if (UpdateLastEntryIfEqual(Loglevel::Warn, code, nullptr, nullptr, true))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, code);
    advance();
}
void Logger::Warn(const __FlashStringHelper* msg) {
    if (UpdateLastEntryIfEqual(Loglevel::Warn, 0, msg, nullptr, false))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, msg);
    advance();
}
void Logger::Warn(uint32_t code, const char* text) {
    if (UpdateLastEntryIfEqual(Loglevel::Warn, code, nullptr, text, true))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, code, text);
    advance();
}
void Logger::Warn(const __FlashStringHelper* msg, const char* text) {
    if (UpdateLastEntryIfEqual(Loglevel::Warn, 0, msg, text, false))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, msg, text);
    advance();
}
void Logger::Warn(const __FlashStringHelper* msg, const HAL_JSON::ZeroCopyString& zcStr) {
    if (UpdateLastEntryIfEqual(Loglevel::Warn, 0, msg, zcStr, false))
        return;
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, msg, zcStr);
    advance();
}


void Logger::printAllLogs(Stream &out, bool onlyPrintNew) {
    size_t start = wrapped ? head : 0;
    size_t count = wrapped ? LOG_BUFFER_SIZE : head;

    for (size_t i = 0; i < count; ++i) {
        size_t index = (start + i) % LOG_BUFFER_SIZE;
        LogEntry& entry = buffer[index];

        if (onlyPrintNew && entry.isNew == false) continue;
        entry.isNew = false;

        //char strTime[32]; // Enough for asctime output
        struct tm* timeinfo = localtime(&entry.timestamp);
        
        out.print('[');
        out.print(timeinfo->tm_mday);
        out.print('/');
        out.print(timeinfo->tm_mon+1);
        out.print(' ');
        if (timeinfo->tm_hour < 10) out.print('0');
        out.print(timeinfo->tm_hour);
        out.print(':');
        if (timeinfo->tm_min < 10) out.print('0');
        out.print(timeinfo->tm_min);
        out.print(':');
        if (timeinfo->tm_sec < 10) out.print('0');
        out.print(timeinfo->tm_sec);
        out.print(']');

        switch (entry.level) {
            case Loglevel::Info: out.print(F("[INFO] ")); break;
            case Loglevel::Warn: out.print(F("[WARN] ")); break;
            case Loglevel::Error: out.print(F("[ERR] ")); break;
        }
        if (entry.source != nullptr) {
            out.print('[');
            out.print(entry.source);
            out.print(']');
            out.print(' ');
        }
        if (entry.repeatCount > 0) {
            out.print('(');
            out.print(entry.repeatCount);
            out.print(')');
        }

        if (entry.isCode) {
            out.print(F("Error Code: 0x"));
            out.print(entry.errorCode, HEX);
        } else {
            out.print(entry.message);
        }
        if (entry.text != nullptr)
            out.print(entry.text);
        out.println();
    }
}

void Logger::advance() {
    head = (head + 1) % LOG_BUFFER_SIZE;
    if (head == 0) wrapped = true;
}

void Logger::setLastEntrySource(const char* src) {
    if (!wrapped && head == 0) {
        // No entries yet, handle appropriately (return a dummy or assert)
        return;
    }
    size_t lastIndex = (head + LOG_BUFFER_SIZE - 1) % LOG_BUFFER_SIZE;
    buffer[lastIndex].source = src;
}

const LogEntry& Logger::getLastEntry() const {
    if (!wrapped && head == 0) {
        // No entries yet, handle appropriately (return a dummy or assert)
        static LogEntry dummy;
        return dummy;
    }
    size_t lastIndex = (head + LOG_BUFFER_SIZE - 1) % LOG_BUFFER_SIZE;
    return buffer[lastIndex];
}
bool Logger::UpdateLastEntryIfEqual(Loglevel lvl, uint32_t err, const __FlashStringHelper* msg, const char* txt, bool codeFlag) {
    if (!wrapped && head == 0) {
        return false;
    }
    size_t lastIndex = (head + LOG_BUFFER_SIZE - 1) % LOG_BUFFER_SIZE;
    LogEntry& entry = buffer[lastIndex];
    if (entry.isEqual(lvl, err, msg, txt, codeFlag)) {
        entry.timestamp = LOGGER_GET_TIME;
        entry.repeatCount++;
        entry.isNew = true;
        return true;
    }
    return false;
}
bool Logger::UpdateLastEntryIfEqual(Loglevel lvl, uint32_t err, const __FlashStringHelper* msg, const HAL_JSON::ZeroCopyString& zcStr, bool codeFlag) {
    if (!wrapped && head == 0) {
        return false;
    }
    size_t lastIndex = (head + LOG_BUFFER_SIZE - 1) % LOG_BUFFER_SIZE;
    LogEntry& entry = buffer[lastIndex];
    if (entry.isEqual(lvl, err, msg, zcStr, codeFlag)) {
        entry.timestamp = LOGGER_GET_TIME;
        entry.repeatCount++;
        entry.isNew = true;
        return true;
    }
    return false;
}
