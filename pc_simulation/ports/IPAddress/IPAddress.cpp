#include "IPAddress.h"
#include <sstream>

IPAddress::IPAddress() : octets{0, 0, 0, 0} {}

IPAddress::IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    octets[0] = a;
    octets[1] = b;
    octets[2] = c;
    octets[3] = d;
}

uint8_t IPAddress::operator[](size_t i) const {
    return octets[i];
}

uint8_t& IPAddress::operator[](size_t i) {
    return octets[i];
}

std::string IPAddress::toString() const {
    return std::to_string(octets[0]) + "." + std::to_string(octets[1]) + "." +
           std::to_string(octets[2]) + "." + std::to_string(octets[3]);
}
