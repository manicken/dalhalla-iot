#pragma once
#include <cstdint>
#include <string>

class IPAddress {
public:
    IPAddress();
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

    uint8_t operator[](size_t i) const;
    uint8_t& operator[](size_t i);

    std::string toString() const;

private:
    uint8_t octets[4];
};
