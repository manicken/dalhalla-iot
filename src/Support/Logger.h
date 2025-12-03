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

#include <Arduino.h>
#include "../HAL_JSON/HAL_JSON_ZeroCopyString.h"

#include <time.h>

enum class Loglevel : uint8_t {
    Info = 0,
    Warn = 1,
    Error = 2
};

struct LogEntry {
      time_t timestamp;
      Loglevel level;
      union {
          uint32_t errorCode;
          const __FlashStringHelper* message;
      };
      char* text;
      const char* source;
      bool isCode;
      bool isNew = false;
      LogEntry();
      void Set(time_t time, Loglevel level, uint32_t errorCode);
      void Set(time_t time, Loglevel level, const __FlashStringHelper* message);
      void Set(time_t time, Loglevel level, uint32_t errorCode, const char* text);
      void Set(time_t time, Loglevel level, const __FlashStringHelper* message, const char* text);
      void Set(time_t time, Loglevel level, uint32_t errorCode, const HAL_JSON::ZeroCopyString& zcStr);
      void Set(time_t time, Loglevel level, const __FlashStringHelper* message, const HAL_JSON::ZeroCopyString& zcStr);

      ~LogEntry();

      // Delete copy constructor and copy assignment
      LogEntry(const LogEntry&) = delete;
      LogEntry& operator=(const LogEntry&) = delete;

      String MessageToString() const;
  };

class Logger {

    
  public:
    Logger();
    void Error(uint32_t code);
    void Error(const __FlashStringHelper* msg);
    void Error(uint32_t code, const char* text);
    void Error(const __FlashStringHelper* msg, const char* text);
    void Error(const __FlashStringHelper* msg, const HAL_JSON::ZeroCopyString& zcStr);
    void Info(uint32_t code);
    void Info(const __FlashStringHelper* msg);
    void Info(uint32_t code, const char* text);
    void Info(const __FlashStringHelper* msg, const char* text);
    void Info(const __FlashStringHelper* msg, const HAL_JSON::ZeroCopyString& zcStr);
    void Warn(uint32_t code);
    void Warn(const __FlashStringHelper* msg);
    void Warn(uint32_t code, const char* text);
    void Warn(const __FlashStringHelper* msg, const char* text);
    void Warn(const __FlashStringHelper* msg, const HAL_JSON::ZeroCopyString& zcStr);
    void printAllLogs(Stream &out = Serial, bool onlyPrintNew = false);

    
    const LogEntry& getLastEntry() const;
    void setLastEntrySource(const char* src);

  private:
    static constexpr size_t LOG_BUFFER_SIZE = 128;
    LogEntry buffer[LOG_BUFFER_SIZE];
    size_t head = 0;
    bool wrapped = false;
    void addAndAdvance(LogEntry&& entry);
    void advance();
};
/*
class PrintStreamAdapter : public Stream {
public:
  PrintStreamAdapter(Print &p) : _print(p) {}

  size_t write(uint8_t b) override { return _print.write(b); }
  size_t write(const uint8_t *buffer, size_t size) override { return _print.write(buffer, size); }

  int available() override { return 0; }
  int read() override { return -1; }
  int peek() override { return -1; }

private:
  Print &_print;
};
*/

// Declare a global instance
extern Logger GlobalLogger;

