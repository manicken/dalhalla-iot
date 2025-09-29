#include "WS2812FX.h"
#include <iostream>

WS2812FX::WS2812FX(uint16_t num_leds, uint8_t pin, uint8_t type)
    : numLEDs(num_leds), pin(pin), type(type), color(0xFFFFFF), mode(FX_MODE_STATIC), speed(100), brightness(255)
{
#ifdef WS2812FX_DEBUG
    std::cout << "WS2812FX constructor: numLEDs=" << numLEDs << " pin=" << int(pin) << " type=" << int(type) << std::endl;
#endif
}

void WS2812FX::init() {
#ifdef WS2812FX_DEBUG
    std::cout << "WS2812FX.init()" << std::endl;
#endif
}

void WS2812FX::service() {
#ifdef WS2812FX_DEBUG
    std::cout << "WS2812FX.service()" << std::endl;
#endif
}

void WS2812FX::start() {
#ifdef WS2812FX_DEBUG
    std::cout << "WS2812FX.start()" << std::endl;
#endif
}

void WS2812FX::stop() {
#ifdef WS2812FX_DEBUG
    std::cout << "WS2812FX.stop()" << std::endl;
#endif
}

void WS2812FX::setBrightness(uint8_t b) {
    brightness = b;
#ifdef WS2812FX_DEBUG
    std::cout << "WS2812FX.setBrightness(" << int(b) << ")" << std::endl;
#endif
}

void WS2812FX::setColor(uint32_t c) {
    color = c;
#ifdef WS2812FX_DEBUG
    std::cout << "WS2812FX.setColor(0x" << std::hex << color << std::dec << ")" << std::endl;
#endif
}

void WS2812FX::setMode(uint8_t m) {
    mode = m;
#ifdef WS2812FX_DEBUG
    std::cout << "WS2812FX.setMode(" << int(m) << ")" << std::endl;
#endif
}

void WS2812FX::setSpeed(uint8_t s) {
    speed = s;
#ifdef WS2812FX_DEBUG
    std::cout << "WS2812FX.setSpeed(" << int(s) << ")" << std::endl;
#endif
}

uint8_t WS2812FX::getMode() { return mode; }
uint32_t WS2812FX::getColor() { return color; }
uint8_t WS2812FX::getBrightness() { return brightness; }
uint8_t WS2812FX::getSpeed() { return speed; }
