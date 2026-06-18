/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  
  Copyright (C) 2026 Jannik Svensson
  Licensed under GNU General Public License v3
*/

#include "DALHAL_FlexibleString.h"

namespace DALHAL {

    FlexibleString::FlexibleString(FlexibleString&& other) noexcept
        : type(other.type),
          len(other.len) {
        
        // Copy the union data
        data = other.data;
        
        // Reset source object
        other.type = Type::Null;
        other.len = 0;
        other.data.cstr = nullptr;  // Clear the union
    }

    FlexibleString FlexibleString::Copy(const char* s) {
        FlexibleString fs;

        if (!s) {
            return fs;
        }

        fs.type = Type::Mutable;
        fs.len = strlen(s);

        fs.data.mutableStr = new char[fs.len + 1];
        memcpy(fs.data.mutableStr, s, fs.len + 1);

        return fs;
    }

    void FlexibleString::SetCopy(const char* s) {
        clear();

        if (!s) {
            return;
        }

        type = Type::Mutable;
        len = strlen(s);

        data.mutableStr = new char[len + 1];
        memcpy(data.mutableStr, s, len + 1);
    }

    FlexibleString FlexibleString::Copy(const ZeroCopyString& s) {
        FlexibleString fs;
        size_t sLen = s.Length();
        if (sLen == 0) {
            return fs;
        }

        fs.type = Type::Mutable;
        fs.len = sLen;

        fs.data.mutableStr = new char[sLen + 1];
        memcpy(fs.data.mutableStr, s.start, sLen);
        fs.data.mutableStr[sLen] = '\0';

        return fs;
    }

    void FlexibleString::SetCopy(const ZeroCopyString& s) {
        clear();

        size_t sLen = s.Length();
        if (sLen == 0) {
            return;
        }

        type = Type::Mutable;
        len = sLen;

        data.mutableStr = new char[sLen + 1];
        memcpy(data.mutableStr, s.start, sLen);
        data.mutableStr[sLen] = '\0';
    }

    FlexibleString::~FlexibleString() {
        clear();
    }

    void FlexibleString::clear() {
        if (type == Type::Mutable && data.mutableStr != nullptr) {
            delete[] data.mutableStr;
        }

        type = Type::Null;
        data.cstr = nullptr;
        len = 0;
    }

    size_t FlexibleString::length() const {
        switch (type) {
#if !(defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
            case Type::Flash:
                return strlen_P((PGM_P)data.flash);
#endif
            case Type::Const:
            case Type::Mutable:
                return len;

            case Type::Null:
            default:
                return 0;
        }
    }

    void FlexibleString::appendTo(std::string& out) const {
        switch (type) {
#if !(defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
            case Type::Flash: {
#if defined(ARDUINO_ARCH_AVR) || defined(ESP8266)
                PGM_P p = reinterpret_cast<PGM_P>(data.flash);
                char c;
                size_t i = 0;

                while ((c = (char)pgm_read_byte(p + i)) != 0) {
                    out.push_back(c);
                    ++i;
                }
#else
                // On ESP32/Teensy flash is usually memory-mapped
                const char* p = reinterpret_cast<const char*>(data.flash);
                if (p != nullptr) {
                    out.append(p);
                }
#endif
                break;
            }
#endif
            case Type::Const:
            case Type::Mutable: {
                if (data.cstr != nullptr) {
                    out.append(data.cstr);
                }
                break;
            }

            case Type::Null:
            default:
                break;
        }
    }

    FlexibleString& FlexibleString::operator=(const char* s) {
        clear();

        if (s != nullptr) {
            type = Type::Const;
            data.cstr = s;
            len = strlen(s);
        }

        return *this;
    }
#if !(defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
    FlexibleString& FlexibleString::operator=(const __FlashStringHelper* s) {
        clear();

        if (s != nullptr) {
            type = Type::Flash;
            data.flash = s;
            len = strlen_P((PGM_P)data.flash);
        }

        return *this;
    }
#endif
    FlexibleString& FlexibleString::operator=(FlexibleString&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        clear();

        type = other.type;
        len = other.len;
        data = other.data;

        // Reset source object
        other.type = Type::Null;
        other.len = 0;
        other.data.cstr = nullptr;

        return *this;
    }

}