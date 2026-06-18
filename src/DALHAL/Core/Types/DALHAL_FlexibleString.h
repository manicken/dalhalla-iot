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

#include <cctype>
#include <string>
#include <cstring>
#include <cstdint>

#include <pgmspace.h>
#include <WString.h> // __FlashStringHelper

#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>

namespace DALHAL {

    class FlexibleString {
    public:
        enum class Type : uint8_t {
            Null,
            Flash,
            Const,
            Mutable
        };

    private:
        Type type = Type::Null;

        union {
#if !(defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
            const __FlashStringHelper* flash;
#endif
            const char* cstr;
            char* mutableStr;
        } data = {};  // Value-initialize union to zero

        size_t len = 0;

    public:
        FlexibleString() = default;
#if !(defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
        FlexibleString(const __FlashStringHelper* s)
            : type(Type::Flash) {
            data.flash = s;
            len = strlen_P((PGM_P)data.flash);
        }
#endif
        FlexibleString(const char* s)
            : type(Type::Const) {
            data.cstr = s;
            len = strlen(s);
        }

        FlexibleString(FlexibleString&& other) noexcept;

        // Deleted copy constructor/assignment to prevent accidental copies of mutable strings
        FlexibleString(const FlexibleString&) = delete;
        FlexibleString& operator=(const FlexibleString&) = delete;

        FlexibleString& operator=(const char* s);
#if !(defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
        FlexibleString& operator=(const __FlashStringHelper* s);
#endif
        FlexibleString& operator=(FlexibleString&& other) noexcept;

        void SetCopy(const char* s);
        static FlexibleString Copy(const char* s);

        void SetCopy(const ZeroCopyString& s);
        static FlexibleString Copy(const ZeroCopyString& s);

        ~FlexibleString();

        void clear();
        size_t length() const;
        void appendTo(std::string& out) const;

        inline bool isFlash() const {
            return type == Type::Flash;
        }

        inline bool isMutable() const {
            return type == Type::Mutable;
        }

        inline bool isNull() const {
            return type == Type::Null;
        }

        inline const char* c_str() const {
            if (type == Type::Const || type == Type::Mutable) {
                return data.cstr;
            }
            return nullptr;
        }
#if !(defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
        inline const __FlashStringHelper* flashStr() const {
            return type == Type::Flash ? data.flash : nullptr;
        }
#endif
    };
}