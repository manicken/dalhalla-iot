#include "Wire.h"
#include <iostream>

TwoWire Wire;

TwoWire::TwoWire() : currentAddress(0) {}
TwoWire::~TwoWire() {}

void TwoWire::begin() {}
void TwoWire::begin(uint8_t address) { currentAddress = address; }
void TwoWire::begin(uint8_t sda, uint8_t scl, uint32_t freq) {
    (void)sda; (void)scl; (void)freq; // stub
}

void TwoWire::beginTransmission(uint8_t address) { currentAddress = address; }
uint8_t TwoWire::endTransmission() { return 0; }
uint8_t TwoWire::endTransmission(bool sendStop) { (void)sendStop; return 0; }

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity) {
    (void)address;
    return quantity;
}

size_t TwoWire::write(uint8_t data) { (void)data; return 1; }
size_t TwoWire::write(const uint8_t* data, size_t quantity) { (void)data; return quantity; }

int TwoWire::available() { return 0; }
int TwoWire::read() { return -1; }

void TwoWire::onReceive(void (*callback)(int)) { (void)callback; }
void TwoWire::onRequest(void (*callback)()) { (void)callback; }

void TwoWire::setClock(uint32_t speed) { (void)speed; }
