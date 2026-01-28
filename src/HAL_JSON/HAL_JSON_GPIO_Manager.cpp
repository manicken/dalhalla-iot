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

#include "HAL_JSON_GPIO_Manager.h"

#define MAKE_PIN_MASK_4(a, b, c, d) (static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(a) | static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(b) | static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(c) | static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(d))
#define MAKE_PIN_MASK_3(a, b, c)    (static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(a) | static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(b) | static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(c))
#define MAKE_PIN_MASK_2(a, b)       (static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(a) | static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(b))
#define MAKE_PIN_MASK_1(a)          (static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(a))

namespace HAL_JSON {

    namespace GPIO_manager
    {
        const gpio_pin available_gpio_list[] {
    #if defined(ESP8266)
        
            {0, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)},  // (reserved for programming) only safe to use as a output
            {1, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::SpecialAtBoot)},  // TXD0 (reserved for programming/UART) only safe to use as a output
            {2, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::SpecialAtBoot)},  // TXD1 (reserved for debug) only safe to use as a output
            //{3, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // RXD0 (reserved for programming/UART)
            {4, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},    // I2C SDA
            {5, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},    // I2C SCL
            //{6, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // SDCLK (reserved for spi flash)
            //{7, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // SDD0 (reserved for spi flash)
            //{8, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // SDD1 (reserved for spi flash)
            //{9, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // SDD2 (reserved for spi flash)
            //{10, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // SDD3 (reserved for spi flash)
            //{11, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // SDCMD (reserved for spi flash)
            {12, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // SPI MISO
            {13, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // SPI MOSI
            {14, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // SPI SCLK
            {15, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // SPI CS/TXD2
    
    #elif defined(ESP32DEV) || defined(_WIN32) || defined(__linux__) || defined(__MAC__)
            {0, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)}, // ADC2_1/TOUCH1 (reserved for programming, better to just keep it a output)
            {1, MAKE_PIN_MASK_3(PinMode::Reserved, PinMode::OUT, PinMode::SpecialAtBoot)}, // U0_TXD (reserved for programmer/debug)
            {2, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::LOW2BOOT)}, // ADC2_2/TOUCH2/SD_DATA0 (must be LOW during boot/is connected to onboard LED, could be a output function only pin)
            {3, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // U0_RXD (reserved for programmer/debug cannot be shared directly)
            {4, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},  // ADC2_0/TOUCH0/SD_DATA1 (ADC2 cannot be used together with WiFi)
            {5, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)},  // VSPI_CS (must be HIGH during boot better to keep it a output only)
      
            {6, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U1_CTS/SPI_CLK (reserved for flash)
            {7, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U2_RTS/SPI_MISO (reserved for flash)
            {8, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U2_CTS/SPI_MOSI (reserved for flash)
            {9, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U1_RXD/SPI_HD (reserved for flash)
            {10, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // U1_TXD/SPI_WP (reserved for flash)
            {11, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // U1_RTX/SPI_CS (reserved for flash)

            {12, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::LOW2BOOT, PinMode::SpecialAtBoot)}, // ADC2_5/TOUCH5/HSPI_MISO/SD_DATA2 (must be LOW during boot, could be a output function only pin)
            {13, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // ADC2_4/TOUCH4/HSPI_MOSI/SD_DATA3 (ADC2 cannot be used together with WiFi)
            {14, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // ADC2_6/TOUCH6/HSPI_CLK/SD_CLK (ADC2 cannot be used together with WiFi)
            {15, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)}, // ADC2_3/TOUCH3/HSPI_CS/SD_CMD (must be HIGH during boot, could be a output function only pin)
            {16, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // U2_RXD
            {17, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // U2_TXD     
            {18, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // VSPI_CLK
            {19, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // VSPI_MISO
            {21, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // I2C_SDA
            {22, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // I2C_SCL
            {23, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // VSPI_MOSI
            {25, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AOUT)}, // ADC2_8/DAC1 (ADC2 cannot be used together with WiFi)
            {26, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AOUT)}, // ADC2_9/DAC2 (ADC2 cannot be used together with WiFi)
            {27, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // ADC2_7/TOUCH7 (ADC2 cannot be used together with WiFi)
            {32, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC1_4/TOUCH9/XTAL32
            {33, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC1_5/TOUCH8/XTAL32
            {34, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}, // ADC1_6 (input only)
            {35, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}, // ADC1_7 (input only)
            {36, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}, // ADC1_0/SensVP (input only)
            {39, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)},  // ADC1_3/SensVN (input only)
            {255, 0x00} // terminator item
            
    #elif defined(ESP32WROVER_E_IE)
            {0, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)}, // ADC2_1/TOUCH1 (reserved for programming, better to just keep it a output)
            {1, MAKE_PIN_MASK_3(PinMode::Reserved, PinMode::OUT, PinMode::SpecialAtBoot)}, // U0_TXD (reserved for programmer/debug)
            {2, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::LOW2BOOT)}, // ADC2_2/TOUCH2/SD_DATA0 (must be LOW during boot/is connected to onboard LED, could be a output function only pin)
            {3, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // U0_RXD (reserved for programmer/debug cannot be shared directly)
            {4, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},  // ADC2_0/TOUCH0/SD_DATA1 (ADC2 cannot be used together with WiFi)
            {5, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)},  // VSPI_CS (must be HIGH during boot better to keep it a output only)
            // GPIO 6-11 are not available on the module they are internally connected to the flash
            {6, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U1_CTS/SPI_CLK (reserved for flash)
            {7, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U2_RTS/SPI_MISO (reserved for flash)
            {8, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U2_CTS/SPI_MOSI (reserved for flash)
            {9, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U1_RXD/SPI_HD (reserved for flash)
            {10, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // U1_TXD/SPI_WP (reserved for flash)
            {11, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // U1_RTX/SPI_CS (reserved for flash)
 
            {12, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::LOW2BOOT, PinMode::SpecialAtBoot)}, // ADC2_5/TOUCH5/HSPI_MISO/SD_DATA2 (must be LOW during boot, could be a output function only pin)
            {13, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // ADC2_4/TOUCH4/HSPI_MOSI/SD_DATA3 (ADC2 cannot be used together with WiFi)
            {14, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // ADC2_6/TOUCH6/HSPI_CLK/SD_CLK (ADC2 cannot be used together with WiFi)
            {15, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)}, // ADC2_3/TOUCH3/HSPI_CS/SD_CMD (must be HIGH during boot, could be a output function only pin)
     
            {18, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // VSPI_CLK
            {19, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // VSPI_MISO
            {21, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // I2C_SDA
            {22, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // I2C_SCL
            {23, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // VSPI_MOSI
            {25, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AOUT)}, // ADC2_8/DAC1 (ADC2 cannot be used together with WiFi)
            {26, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AOUT)}, // ADC2_9/DAC2 (ADC2 cannot be used together with WiFi)
            {27, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // ADC2_7/TOUCH7 (ADC2 cannot be used together with WiFi)
            {32, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC1_4/TOUCH9/XTAL32
            {33, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC1_5/TOUCH8/XTAL32
            {34, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}, // ADC1_6 (input only)
            {35, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}, // ADC1_7 (input only)
            {36, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}, // ADC1_0/SensVP (input only)
            {39, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)},  // ADC1_3/SensVN (input only)
            {255, 0x00} // terminator item
    #elif defined(waveshare_esp32c3_zero)
            {0, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            {1, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            {2, MAKE_PIN_MASK_4(PinMode::OUT, PinMode::LOW2BOOT, PinMode::SpecialAtBoot, PinMode::AIN)}, // strapping pin
            {3, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            {4, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            {5, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            {6, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {7, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {8, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {9, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)}, // strapping pin
            {10, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // onboard ws2812 LED
        //#if ARDUINO_USB_CDC_ON_BOOT || ARDUINO_USB_MODE
            {18, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // USB_DP
            {19, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // USP_DM
        //#else
        //    {18, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // USB_DP
        //    {19, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // USP_DM
        //#endif
            {20, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::SpecialAtBoot)}, // flash standard UART RxD
            {21, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::SpecialAtBoot)}, // flash standard UART TxD
    #elif defined(seeed_xiao_esp32c3)

            {2, MAKE_PIN_MASK_4(PinMode::OUT, PinMode::LOW2BOOT, PinMode::SpecialAtBoot, PinMode::AIN)}, // strapping pin
            {3, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            {4, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            {5, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            {6, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {7, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {8, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {9, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)}, // strapping pin
            {10, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {20, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::SpecialAtBoot)}, // flash standard UART RxD
            {21, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::SpecialAtBoot)}, // flash standard UART TxD
    #elif defined(waveshare_esp32c6_zero)
            {0, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            {1, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            {2, MAKE_PIN_MASK_4(PinMode::OUT, PinMode::LOW2BOOT, PinMode::SpecialAtBoot, PinMode::AIN)}, // strapping pin
            {3, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            {4, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            {5, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},
            // underside pins 6-9, 12, 13
            {6, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::UNDERSIDE)},
            {7, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::UNDERSIDE)},
            {8, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::UNDERSIDE)}, // onboard ws2812 LED
            {9, MAKE_PIN_MASK_4(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot, PinMode::UNDERSIDE)}, // strapping pin
            {12, MAKE_PIN_MASK_3(PinMode::Reserved, PinMode::SpecialAtBoot, PinMode::UNDERSIDE)}, // USB_DM
            {13, MAKE_PIN_MASK_3(PinMode::Reserved, PinMode::SpecialAtBoot, PinMode::UNDERSIDE)}, // USP_DP
            
            {14, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {15, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {16, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::SpecialAtBoot)}, // flash standard UART TxD
            {17, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::SpecialAtBoot)}, // flash standard UART RxD
            {18, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {19, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {20, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {21, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            {22, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},
            // underside pin
            {23, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::UNDERSIDE)}, 

    #endif
        }; // const gpio_pin available_gpio_list[] {

        int available_gpio_list_lenght = -1; // not set yet
        void set_available_gpio_list_length() {
            int len = 0;
            while (available_gpio_list[len].pin != 0xFF) ++len;
            available_gpio_list_lenght = len;
        }

        const PinModeDef PinModeStrings[] = {
            {"Reserved", static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(PinMode::Reserved)},
            {"SpecialAtBoot", static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(PinMode::SpecialAtBoot)},
            {"LOW2BOOT", static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(PinMode::LOW2BOOT)},
            {"HIGH2BOOT", static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(PinMode::HIGH2BOOT)},
            {"OUT", static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(PinMode::OUT)},
            {"IN", static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(PinMode::IN)},
            {"AIN", static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(PinMode::AIN)},
            {"AOUT", static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(PinMode::AOUT)},
            {"UNDERSIDE", static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(PinMode::UNDERSIDE)},
            {nullptr, 0} // terminator item
        };

        int PinModeStrings_length = -1;
        void set_PinModeStrings_length() {
            int len = 0;
            while (PinModeStrings[len].Name != nullptr) ++len;
            PinModeStrings_length = len;
        }
        
        uint8_t* reservedPins = nullptr;

        std::string describePinMode(HAL_JSON_GPIO_MGR_PINMODE_TYPE mask) {
            std::string result;
            int i = 0;
            while (true) {
                const PinModeDef& pinModeDef = PinModeStrings[i];
                if (pinModeDef.Name == nullptr) break;
                if (mask & pinModeDef.mode) {
                    if (!result.empty()) result += "|";
                    result += pinModeDef.Name;
                }
                i++;
            }
            return result.empty() ? "None" : result;
        }

        bool CheckIfPinAvailable(uint8_t pin, HAL_JSON_GPIO_MGR_PINMODE_TYPE pinMode) {
            for (int i = 0; available_gpio_list[i].pin != 255; i++) {
                const gpio_pin& pinDef = available_gpio_list[i];
                if (pinDef.pin != pin) continue;
                
                if (reservedPins[i] == 1) {
                    std::string msg = std::to_string(pin);
                    GlobalLogger.Error(F("CheckIfPinAvailable error - pin allready reserved: "),msg.c_str());
                    return false;
                }
                if ((pinMode & pinDef.mode) == pinMode)
                    return true;
                std::string errStr = Convert::toBin(pinMode) + " & " + Convert::toBin(pinDef.mode);
                GlobalLogger.Error(F("CheckIfPinAvailable error - pinmode mismatch: "),errStr.c_str());
                return false;
            }
            std::string msg = std::to_string(pin);
            GlobalLogger.Error(F("Pin to reserve - not found: "),msg.c_str());
            return false;
        }

        bool CheckIfPinAvailableAndReserve(uint8_t pin, HAL_JSON_GPIO_MGR_PINMODE_TYPE pinMode) {
            if (CheckIfPinAvailable(pin, pinMode)) {
                ReservePin(pin);
                return true;
            }
            return false;
        }

        bool ValidateJsonAndCheckIfPinAvailableAndReserve(const JsonVariant& jsonObj, HAL_JSON_GPIO_MGR_PINMODE_TYPE pinMode) {
            if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN)); return false; }
            if (jsonObj[HAL_JSON_KEYNAME_PIN].is<uint8_t>() == false)  { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PIN)); return false; }
            uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>(); 
            return GPIO_manager::CheckIfPinAvailableAndReserve(pin, static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(GPIO_manager::PinMode::OUT));
        }

        bool ValidateJsonAndCheckIfPinAvailableAndReserve(const JsonVariant& jsonObj, const char* NAME, HAL_JSON_GPIO_MGR_PINMODE_TYPE pinMode) {
            if (jsonObj.containsKey(NAME) == false) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(), NAME); return false; }
            if (jsonObj[NAME].is<uint8_t>() == false)  { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(), NAME); return false; }
            uint8_t pin = jsonObj[NAME].as<uint8_t>(); 
            return GPIO_manager::CheckIfPinAvailableAndReserve(pin, static_cast<HAL_JSON_GPIO_MGR_PINMODE_TYPE>(GPIO_manager::PinMode::OUT));
        }

        void ClearAllReservations() {
            if (available_gpio_list_lenght == -1) set_available_gpio_list_length();
            if (reservedPins == nullptr)
                reservedPins = new uint8_t[available_gpio_list_lenght];
            for (int i=0;i<available_gpio_list_lenght;i++)
                reservedPins[i] = 0;
        }
        /** it's recommended to call CheckIfPinAvailable prior to using this function,
         * this function is very basic and do only set the actual pin to reserved state, 
         * so calling it many times on the same pin have no effect */
        void ReservePin(uint8_t pin) {
            for (int i=0;i<available_gpio_list_lenght;i++) {
                if (available_gpio_list[i].pin == pin) {
                    reservedPins[i] = 1;
                    return;
                }
            }
        }

        std::string GetList(ZeroCopyString& zcMode)
        {
            GPIO_manager::PrintListMode listMode = GPIO_manager::PrintListMode::Hex; // set the default here

            ZeroCopyString zcPrintMode = zcMode.SplitOffHead('/');
            if (zcPrintMode.NotEmpty()) {
                if (zcPrintMode == HAL_JSON_CMD_EXEC_GPIO_LIST_MODE_STRING)
                    listMode = GPIO_manager::PrintListMode::String;
                else if (zcPrintMode == HAL_JSON_CMD_EXEC_GPIO_LIST_MODE_BINARY)
                    listMode = GPIO_manager::PrintListMode::Binary;
                else if (zcPrintMode == HAL_JSON_CMD_EXEC_GPIO_LIST_MODE_HEX)
                    listMode = GPIO_manager::PrintListMode::Hex;
                //else // default set above
                //    listMode = HAL_JSON_CMD_EXEC_GPIO_LIST_MODE_DEFAULT; // default
            }
            std::string strList;// = "{";
            
    #if defined(ESP8266)
            strList.append("\"MCU\":\"ESP8266\",");
    #elif defined(ESP32)
            strList.append("\"MCU\":\"ESP32\",");
    #elif defined(_WIN32) || defined(__linux__)
            strList.append("\"MCU\":\"PC_SIM\",");
    #endif
            if (listMode != PrintListMode::String) {
                strList.append("\"PinModes\":{");
                if (PinModeStrings_length == -1) set_PinModeStrings_length();
                for (int i=0;i<PinModeStrings_length;i++)
                {
                    strList.append("\"");
                    uint8_t modeMask = PinModeStrings[i].mode;
                    if (listMode == PrintListMode::Binary) {
                        strList.append(Convert::toBin(modeMask));
                    }
                    else {
                        strList.append(Convert::toHex(modeMask));
                    }
                    strList.append("\":\"");
                    strList.append(PinModeStrings[i].Name);
                    strList.append("\"");
                    if (i<(PinModeStrings_length-1))
                        strList.append(",");
                }
                strList.append("},");
            }
            strList.append("\"list\":{");
            bool first = true;
            if (available_gpio_list_lenght == -1) set_available_gpio_list_length();
            for (int i=0;i<available_gpio_list_lenght;i++)
            {
                if (first == false)
                    strList.append(",");
                else
                    first = false;

                strList.append("\"");
                strList.append(std::to_string(available_gpio_list[i].pin));
                strList.append("\":\"");
                uint8_t modeMask = available_gpio_list[i].mode;
                if (listMode == PrintListMode::String)
                    strList.append(describePinMode(modeMask).c_str());
                else if (listMode == PrintListMode::Binary) {
                    strList.append(Convert::toBin(modeMask));
                }
                else { // (listMode == PrintListMode::Hex) 
                    strList.append(Convert::toHex(modeMask));
                }
                strList.append("\"");                        
            }
            strList.append("}");
            //strList.append("}");
            return strList;
        }
    }
}