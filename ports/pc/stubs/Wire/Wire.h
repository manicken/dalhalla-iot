#pragma once
#include <cstdint>
#include <cstddef>

class TwoWire {
public:
    TwoWire();
    ~TwoWire();

    // ESP32/ESP8266 variants
    void begin();                              // no args
    void begin(uint8_t address);               // with address
    void begin(uint8_t sda, uint8_t scl, uint32_t freq); // ESP32 variant

    void beginTransmission(uint8_t address);
    uint8_t endTransmission();                 // original
    uint8_t endTransmission(bool sendStop);   // ESP32 variant

    uint8_t requestFrom(uint8_t address, uint8_t quantity);

    size_t write(uint8_t data);
    size_t write(const uint8_t* data, size_t quantity);

    int available();
    int read();

    void onReceive(void (*callback)(int));
    void onRequest(void (*callback)());

    void setClock(uint32_t speed);            // ESP32 variant

private:
    uint8_t currentAddress;
};

// Global instance
extern TwoWire Wire;
