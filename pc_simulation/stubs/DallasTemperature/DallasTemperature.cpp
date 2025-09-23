/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "DallasTemperature.h"

float getRandomValue() {
    // Create a random device and use it to seed a generator
    std::random_device rd;
    std::mt19937 gen(rd()); // Mersenne Twister engine

    // Create a distribution in the range [10.0, 40.0)
    std::uniform_real_distribution<float> dist(10.0f, 40.0f);

    // Generate a random number
    float randomNumber = dist(gen);
    return randomNumber;
}

DallasTemperature::DallasTemperature(OneWire* _oneWire) {}
void DallasTemperature::setWaitForConversion(bool val) {}
void DallasTemperature::requestTemperatures() {}
void DallasTemperature::setTempC(float value) {
    tempC = value;
}
float DallasTemperature::getTempC(const uint8_t* deviceAddress) {
    return tempC;
}
float DallasTemperature::getTempF(const uint8_t* deviceAddress) {
    return tempC * 9.0 / 5.0 + 32.0;
}