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

#include "DALHAL_Logger.h"

#include <DALHAL/API/DALHAL_CommandCallback.h>
#include <DALHAL/API/DALHAL_BlockStreamer.h>
#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream>
#endif

#if defined(ESP8266) || defined(ESP32)
#include <DALHAL/API/DALHAL_WebSocketAPI.h> // for SendMessage
#else
#include <DALHAL_WebSocketAPI_Windows.h> // for SendMessage
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
      source(nullptr),
      isCode(true)
       {}

    void LogEntry::PrintTo(DALHAL::StringBuilderStreamer& sbs) const {
        // build timestamp prefix
        struct tm* timeinfo = localtime(&timestamp);

        sbs.write_json_array_begin();
        sbs.write(timeinfo->tm_mday, "%02d");
        sbs.write('/');
        sbs.write(timeinfo->tm_mon+1, "%02d");
        sbs.write(' ');
        sbs.write(timeinfo->tm_hour, "%02d");
        sbs.write_char(':');
        sbs.write(timeinfo->tm_min, "%02d");
        sbs.write_char(':');
        sbs.write(timeinfo->tm_sec, "%02d");
        sbs.write_json_array_end();

        switch (level) {
            case Loglevel::Info: sbs.write(F("[INFO] ")); break;
            case Loglevel::Warn: sbs.write(F("[WARN] ")); break;
            case Loglevel::Error: sbs.write(F("[ERR] ")); break;
        }

        // append source
        if (source != nullptr) {
            sbs.write_json_object_begin();
            sbs.write(source);
            sbs.write_json_object_end();
            sbs.write(' ');
        }
        // append repeat count
        if (repeatCount > 0) {
            sbs.write('(');
            sbs.write(repeatCount);
            sbs.write(')');
            sbs.write(' ');
        }
        // append message or error code
        if (isCode) {
            sbs.write(F("Error Code: 0x"));
            sbs.write_asHex(errorCode);
        } else if (message != nullptr) {
            sbs.write(message);
        } else {
            sbs.write(F("<message null>"));
        }
        // append optional text
        if (text != nullptr) {
            sbs.write(text);
        }
        
    }

    void LogEntry::Print(Stream &out) const {
        //char strTime[32]; // Enough for asctime output
        struct tm* timeinfo = localtime(&timestamp);
        
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

        switch (level) {
            case Loglevel::Info: out.print(F("[INFO] ")); break;
            case Loglevel::Warn: out.print(F("[WARN] ")); break;
            case Loglevel::Error: out.print(F("[ERR] ")); break;
        }
        if (source != nullptr) {
            out.print('[');
            out.print(source);
            out.print(']');
            out.print(' ');
        }
        if (repeatCount > 0) {
            out.print('(');
            out.print((int)repeatCount);
            out.print(')');
        }

        if (isCode) {
            out.print(F("Error Code: 0x"));
            out.print(errorCode, HEX);
        } else {
            out.print(message);
        }
        if (text != nullptr)
            out.print(text);
       
    }

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
    bool LogEntry::isEqual(Loglevel lvl, uint32_t err, const __FlashStringHelper* msg, const DALHAL::ZeroCopyString& zcStr, bool codeFlag) const 
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
void LogEntry::Set(time_t time, Loglevel _level, uint32_t _errorCode, const DALHAL::ZeroCopyString& zcStr) {
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
void LogEntry::Set(time_t time, Loglevel _level, const __FlashStringHelper* _message, const DALHAL::ZeroCopyString& zcStr) {
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

void LogEntry::MessageWriteTo(DALHAL::StringBuilderStreamer& sbs) const {

    if (source != nullptr) {
        sbs.write_json_array_begin();
        sbs.write(source);
        sbs.write_json_array_end();
    }
    if (message != nullptr) {
        sbs.write(message);
    } else {
        sbs.write(F("<entry error>"));
    }
    if (text != nullptr) {
        sbs.write(text);
    }

}

//    ██       ██████   ██████   ██████  ███████ ██████  
//    ██      ██    ██ ██       ██       ██      ██   ██ 
//    ██      ██    ██ ██   ███ ██   ███ █████   ██████  
//    ██      ██    ██ ██    ██ ██    ██ ██      ██   ██ 
//    ███████  ██████   ██████   ██████  ███████ ██   ██ 

Logger::Logger() {

}

void Logger::EmitLastEntry()
{
#ifndef ESP8266 // currently disabled for esp8266 to debug issuse
    DALHAL::BlockStreamer bs(DALHAL::WebSocketAPI::BroadcastCb, "log entry", DALHAL::BlockStreamer::DataType::PlainText);

    getLastEntry().PrintTo(bs.writer());

    /*std::string entryStr = getLastEntry().ToString();
    DALHAL::WebSocketAPI::Broadcast(entryStr);
    Serial.println(entryStr.c_str());*/
#endif
}

void Logger::Error(uint32_t code) {
    if (UpdateLastEntryIfEqual(Loglevel::Error, code, nullptr, nullptr, true) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, code);
        advance();
    }
    EmitLastEntry();
}
void Logger::Error(const __FlashStringHelper* msg) {
    if (UpdateLastEntryIfEqual(Loglevel::Error, 0, msg, nullptr, false) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, msg);
        advance();
    }
    EmitLastEntry();
}
void Logger::Error(uint32_t code, const char* text) {
    if (UpdateLastEntryIfEqual(Loglevel::Error, code, nullptr, text, true) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, code, text);
        advance();
    }
    EmitLastEntry();
}
void Logger::Error(const __FlashStringHelper* msg, const char* text) {
    if (UpdateLastEntryIfEqual(Loglevel::Error, 0, msg, text, false) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, msg, text);
        advance();
    }
    EmitLastEntry();
}
void Logger::Error(const __FlashStringHelper* msg, const DALHAL::ZeroCopyString& zcStr) {
    if (UpdateLastEntryIfEqual(Loglevel::Error, 0, msg, zcStr, false) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, msg, zcStr);
        advance();
    }
    EmitLastEntry();
}

void Logger::Info(uint32_t code) {
    if (UpdateLastEntryIfEqual(Loglevel::Info, code, nullptr, nullptr, true) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, code);
        advance();
    }
    EmitLastEntry();
}
void Logger::Info(const __FlashStringHelper* msg) {
    if (UpdateLastEntryIfEqual(Loglevel::Info, 0, msg, nullptr, false) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, msg);
        advance();
    }
    EmitLastEntry();
}
void Logger::Info(uint32_t code, const char* text) {
    if (UpdateLastEntryIfEqual(Loglevel::Info, code, nullptr, text, true) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, code, text);
        advance();
    }
    EmitLastEntry();
}
void Logger::Info(const __FlashStringHelper* msg, const char* text) {
    if (UpdateLastEntryIfEqual(Loglevel::Info, 0, msg, text, false) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, msg, text);
        advance();
    }
    EmitLastEntry();
}
void Logger::Info(const __FlashStringHelper* msg, const DALHAL::ZeroCopyString& zcStr) {
    if (UpdateLastEntryIfEqual(Loglevel::Info, 0, msg, zcStr, false) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, msg, zcStr);
        advance();
    }
    EmitLastEntry();
}

void Logger::Warn(uint32_t code) {
    if (UpdateLastEntryIfEqual(Loglevel::Warn, code, nullptr, nullptr, true) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, code);
        advance();
    }
    EmitLastEntry();
}
void Logger::Warn(const __FlashStringHelper* msg) {
    if (UpdateLastEntryIfEqual(Loglevel::Warn, 0, msg, nullptr, false) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, msg);
        advance();
    }
    EmitLastEntry();
}
void Logger::Warn(uint32_t code, const char* text) {
    if (UpdateLastEntryIfEqual(Loglevel::Warn, code, nullptr, text, true) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, code, text);
        advance();
    }
    EmitLastEntry();
}
void Logger::Warn(const __FlashStringHelper* msg, const char* text) {
    if (UpdateLastEntryIfEqual(Loglevel::Warn, 0, msg, text, false) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, msg, text);
        advance();
    }
    EmitLastEntry();
}
void Logger::Warn(const __FlashStringHelper* msg, const DALHAL::ZeroCopyString& zcStr) {
    if (UpdateLastEntryIfEqual(Loglevel::Warn, 0, msg, zcStr, false) == false) {
        buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, msg, zcStr);
        advance();
    }
    EmitLastEntry();
}

void Logger::printAllLogs(DALHAL::StringBuilderStreamer& sbs, bool onlyPrintNew) {
    size_t start = wrapped ? head : 0;
    size_t count = wrapped ? LOG_BUFFER_SIZE : head;

    for (size_t i = 0; i < count; ++i) {
        size_t index = (start + i) % LOG_BUFFER_SIZE;
        LogEntry& entry = buffer[index];

        if (onlyPrintNew && entry.isNew == false) continue;
        entry.isNew = false;
        
        entry.PrintTo(sbs);
        sbs.write('\n');
    }
}


void Logger::printAllLogs(Stream &out, bool onlyPrintNew) {
    size_t start = wrapped ? head : 0;
    size_t count = wrapped ? LOG_BUFFER_SIZE : head;

    for (size_t i = 0; i < count; ++i) {
        size_t index = (start + i) % LOG_BUFFER_SIZE;
        LogEntry& entry = buffer[index];

        if (onlyPrintNew && entry.isNew == false) continue;
        entry.isNew = false;

        entry.Print(out);
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
bool Logger::UpdateLastEntryIfEqual(Loglevel lvl, uint32_t err, const __FlashStringHelper* msg, const DALHAL::ZeroCopyString& zcStr, bool codeFlag) {
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
