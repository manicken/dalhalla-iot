#pragma once

#include <cstdint>
#include <cstddef>
#include "../IPAddress/IPAddress.h"

// Arduino-style Client base class (simplified)
class Client {
public:
    virtual ~Client() {}
    virtual int connect(const char* host, uint16_t port) = 0;
    virtual int connect(const IPAddress& ip, uint16_t port) {
    return connect("127.0.0.1", port); // fake connection for PC
}
    virtual void flush() {}
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int read(uint8_t* buf, size_t size) = 0;
    virtual size_t write(uint8_t data) = 0;
    virtual size_t write(const uint8_t* buf, size_t size) = 0;
    virtual void stop() = 0;
    virtual uint8_t connected() = 0;
    virtual operator bool() = 0;
    
};