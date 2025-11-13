#include "Stream.h"

Stream::Stream(std::ostream& stream) : out(stream) {}

bool Stream::available() { return false; }

int Stream::read() { return 0; }

int Stream::read(uint8_t* buf, size_t size) {
    (void)buf; (void)size;
    return 0;
}

int Stream::peek() { return -1; }

void Stream::print(const std::string& s) { out << s; }
void Stream::print(const char* s)        { out << s; }
void Stream::print(char c)               { out << c; }

void Stream::print(int n, int format) {
    if (format == HEX)
        out << "0x" << std::hex << n << std::dec;
    else
        out << n;
}

void Stream::println() { out << std::endl; }
void Stream::println(const std::string& s) { out << s << std::endl; }
void Stream::println(const char* s)        { out << s << std::endl; }

size_t Stream::write(uint8_t c) {
    out.put(static_cast<char>(c));
    return 1;
}

size_t Stream::write(const uint8_t* buf, size_t size) {
    out.write(reinterpret_cast<const char*>(buf), size);
    return size;
}

void Stream::flush() {
    out.flush();
}
