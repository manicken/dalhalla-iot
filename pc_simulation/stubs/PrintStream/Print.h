#pragma once
#include <cstdint>
#include <cstddef>

class Print {
public:
    virtual ~Print() {}

    virtual size_t write(uint8_t c) = 0;
    virtual size_t write(const uint8_t* buffer, size_t size) {
        size_t n = 0;
        for (size_t i = 0; i < size; i++) {
            n += write(buffer[i]);
        }
        return n;
    }

    virtual void flush() {}
};
