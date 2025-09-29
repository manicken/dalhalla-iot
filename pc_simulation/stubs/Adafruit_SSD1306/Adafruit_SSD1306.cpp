#include "Adafruit_SSD1306.h"
#include <iostream>

Adafruit_SSD1306::Adafruit_SSD1306(int8_t w, int8_t h, void* wire, int8_t rst)
    : width(w), height(h), wireInterface(wire), resetPin(rst) {}

bool Adafruit_SSD1306::begin(uint8_t vcc, uint8_t i2caddr, bool reset) {
    (void)vcc; (void)i2caddr; (void)reset;
#ifdef SSD1306_DEBUG
    std::cout << "Adafruit_SSD1306::begin()" << std::endl;
#endif
    return true;
}

void Adafruit_SSD1306::display() {}
void Adafruit_SSD1306::clearDisplay() {}
void Adafruit_SSD1306::setTextSize(uint8_t s) { (void)s; }
void Adafruit_SSD1306::setTextColor(uint16_t c) { (void)c; }
void Adafruit_SSD1306::setCursor(int16_t x, int16_t y) { (void)x; (void)y; }
void Adafruit_SSD1306::print(const char* text) { (void)text; }
void Adafruit_SSD1306::println(const char* text) { (void)text; }

void Adafruit_SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
    (void)x; (void)y; (void)color;
}

void Adafruit_SSD1306::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    (void)x0; (void)y0; (void)x1; (void)y1; (void)color;
}

void Adafruit_SSD1306::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    (void)x; (void)y; (void)w; (void)h; (void)color;
}
