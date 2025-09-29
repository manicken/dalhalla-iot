#pragma once
#include <cstdint>

class Adafruit_SSD1306 {
public:
    Adafruit_SSD1306(int8_t w, int8_t h, void* wire = nullptr, int8_t rst = -1);

    bool begin(uint8_t vcc = 0x00, uint8_t i2caddr = 0x3C, bool reset = false);
    void display();
    void clearDisplay();
    void setTextSize(uint8_t s);
    void setTextColor(uint16_t c);
    void setCursor(int16_t x, int16_t y);
    void print(const char* text);
    void println(const char* text);

    // Optional graphics functions
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

private:
    int8_t width;
    int8_t height;
    int8_t resetPin;
    void* wireInterface;
};
