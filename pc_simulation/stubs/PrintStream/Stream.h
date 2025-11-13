#pragma once

#include "Print.h"
#include <iostream>
#include <iomanip>
#include <string>

#ifndef DEC
#define DEC 10
#endif
#ifndef HEX
#define HEX 16
#endif

class Stream : public Print {
    std::ostream& out;

public:
    explicit Stream(std::ostream& stream = std::cout);

    bool available();
    int read();
    int read(uint8_t* buf, size_t size);
    int peek();

    void print(const std::string& s);
    void print(const char* s);
    void print(char c);
    void print(int n, int format = DEC);

    void println();
    void println(const std::string& s);
    void println(const char* s);

    size_t write(uint8_t c) override;
    size_t write(const uint8_t* buf, size_t size) override;
    void flush() override;
};
