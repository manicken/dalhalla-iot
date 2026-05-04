/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2026 Jannik Svensson

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

#include "DALHAL_GPIO_Manager.h"
#include <DALHAL/Support/ConvertHelper.h> // for Convert::toBin & Convert::toHex
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <System/Info.h> // Info::getESPVariant()

namespace DALHAL {

    namespace GPIO_manager
    {
        static const gpio_pin NA_PIN = {-1, 0};

        constexpr gpio_pin available_gpio_list[] {
    #if defined(ESP8266)
        
            {0, (PinFunc::OUT | PinFunc::HIGH2BOOT | PinFunc::SpecialAtBoot)},  // (reserved for programming) only safe to use as a output
            {1, (PinFunc::OUT | PinFunc::SpecialAtBoot)},  // TXD0 (reserved for programming/UART) only safe to use as a output
            {2, (PinFunc::OUT | PinFunc::SpecialAtBoot)},  // TXD1 (reserved for debug) only safe to use as a output
            //{3, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // RXD0 (reserved for programming/UART)
            {4, (PinFunc::OUT | PinFunc::IN)},    // I2C SDA
            {5, (PinFunc::OUT | PinFunc::IN)},    // I2C SCL
            //{6, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // SDCLK (reserved for spi flash)
            //{7, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // SDD0 (reserved for spi flash)
            //{8, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // SDD1 (reserved for spi flash)
            //{9, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // SDD2 (reserved for spi flash)
            //{10, (PinFunc::Reserved | PinFunc::SpecialAtBoot)}, // SDD3 (reserved for spi flash)
            //{11, (PinFunc::Reserved | PinFunc::SpecialAtBoot)}, // SDCMD (reserved for spi flash)
            {12, (PinFunc::OUT | PinFunc::IN)}, // SPI MISO
            {13, (PinFunc::OUT | PinFunc::IN)}, // SPI MOSI
            {14, (PinFunc::OUT | PinFunc::IN)}, // SPI SCLK
            {15, (PinFunc::OUT | PinFunc::IN)}, // SPI CS/TXD2
    
    #elif defined(ESP32DEV) || defined(_WIN32) || defined(__linux__) || defined(__MAC__)
            {0, (PinFunc::OUT | PinFunc::HIGH2BOOT | PinFunc::SpecialAtBoot)}, // ADC2_1/TOUCH1 (reserved for programming, better to just keep it a output)
            {1, (PinFunc::Reserved | PinFunc::OUT | PinFunc::SpecialAtBoot)}, // U0_TXD (reserved for programmer/debug)
            {2, (PinFunc::OUT | PinFunc::LOW2BOOT)}, // ADC2_2/TOUCH2/SD_DATA0 (must be LOW during boot/is connected to onboard LED, could be a output function only pin)
            {3, (PinFunc::Reserved | PinFunc::SpecialAtBoot)}, // U0_RXD (reserved for programmer/debug cannot be shared directly)
            {4, (PinFunc::OUT | PinFunc::IN)},  // ADC2_0/TOUCH0/SD_DATA1 (ADC2 cannot be used together with WiFi)
            {5, (PinFunc::OUT | PinFunc::HIGH2BOOT | PinFunc::SpecialAtBoot)},  // VSPI_CS (must be HIGH during boot better to keep it a output only)
      
            {6, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // U1_CTS/SPI_CLK (reserved for flash)
            {7, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // U2_RTS/SPI_MISO (reserved for flash)
            {8, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // U2_CTS/SPI_MOSI (reserved for flash)
            {9, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // U1_RXD/SPI_HD (reserved for flash)
            {10, (PinFunc::Reserved | PinFunc::SpecialAtBoot)}, // U1_TXD/SPI_WP (reserved for flash)
            {11, (PinFunc::Reserved | PinFunc::SpecialAtBoot)}, // U1_RTX/SPI_CS (reserved for flash)

            {12, (PinFunc::OUT | PinFunc::LOW2BOOT | PinFunc::SpecialAtBoot)}, // ADC2_5/TOUCH5/HSPI_MISO/SD_DATA2 (must be LOW during boot, could be a output function only pin)
            {13, (PinFunc::OUT | PinFunc::IN)}, // ADC2_4/TOUCH4/HSPI_MOSI/SD_DATA3 (ADC2 cannot be used together with WiFi)
            {14, (PinFunc::OUT | PinFunc::IN)}, // ADC2_6/TOUCH6/HSPI_CLK/SD_CLK (ADC2 cannot be used together with WiFi)
            {15, (PinFunc::OUT | PinFunc::HIGH2BOOT | PinFunc::SpecialAtBoot)}, // ADC2_3/TOUCH3/HSPI_CS/SD_CMD (must be HIGH during boot, could be a output function only pin)
            {16, (PinFunc::OUT | PinFunc::IN)}, // U2_RXD
            {17, (PinFunc::OUT | PinFunc::IN)}, // U2_TXD     
            {18, (PinFunc::OUT | PinFunc::IN)}, // VSPI_CLK
            {19, (PinFunc::OUT | PinFunc::IN)}, // VSPI_MISO
            {21, (PinFunc::OUT | PinFunc::IN)}, // I2C_SDA
            {22, (PinFunc::OUT | PinFunc::IN)}, // I2C_SCL
            {23, (PinFunc::OUT | PinFunc::IN)}, // VSPI_MOSI
            {25, (PinFunc::OUT | PinFunc::IN | PinFunc::AOUT)}, // ADC2_8/DAC1 (ADC2 cannot be used together with WiFi)
            {26, (PinFunc::OUT | PinFunc::IN | PinFunc::AOUT)}, // ADC2_9/DAC2 (ADC2 cannot be used together with WiFi)
            {27, (PinFunc::OUT | PinFunc::IN)}, // ADC2_7/TOUCH7 (ADC2 cannot be used together with WiFi)
            {32, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)}, // ADC1_4/TOUCH9/XTAL32
            {33, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)}, // ADC1_5/TOUCH8/XTAL32
            {34, (PinFunc::IN | PinFunc::AIN)}, // ADC1_6 (input only)
            {35, (PinFunc::IN | PinFunc::AIN)}, // ADC1_7 (input only)
            {36, (PinFunc::IN | PinFunc::AIN)}, // ADC1_0/SensVP (input only)
            {39, (PinFunc::IN | PinFunc::AIN)},  // ADC1_3/SensVN (input only)
            
    #elif defined(ESP32WROVER_E_IE)
            {0, (PinFunc::OUT | PinFunc::HIGH2BOOT | PinFunc::SpecialAtBoot)}, // ADC2_1/TOUCH1 (reserved for programming, better to just keep it a output)
            {1, (PinFunc::Reserved | PinFunc::OUT | PinFunc::SpecialAtBoot)}, // U0_TXD (reserved for programmer/debug)
            {2, (PinFunc::OUT | PinFunc::LOW2BOOT)}, // ADC2_2/TOUCH2/SD_DATA0 (must be LOW during boot/is connected to onboard LED, could be a output function only pin)
            {3, (PinFunc::Reserved | PinFunc::SpecialAtBoot)}, // U0_RXD (reserved for programmer/debug cannot be shared directly)
            {4, (PinFunc::OUT | PinFunc::IN)},  // ADC2_0/TOUCH0/SD_DATA1 (ADC2 cannot be used together with WiFi)
            {5, (PinFunc::OUT | PinFunc::HIGH2BOOT | PinFunc::SpecialAtBoot)},  // VSPI_CS (must be HIGH during boot better to keep it a output only)
            // GPIO 6-11 are not available on the module they are internally connected to the flash
            {6, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // U1_CTS/SPI_CLK (reserved for flash)
            {7, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // U2_RTS/SPI_MISO (reserved for flash)
            {8, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // U2_CTS/SPI_MOSI (reserved for flash)
            {9, (PinFunc::Reserved | PinFunc::SpecialAtBoot)},  // U1_RXD/SPI_HD (reserved for flash)
            {10, (PinFunc::Reserved | PinFunc::SpecialAtBoot)}, // U1_TXD/SPI_WP (reserved for flash)
            {11, (PinFunc::Reserved | PinFunc::SpecialAtBoot)}, // U1_RTX/SPI_CS (reserved for flash)
 
            {12, (PinFunc::OUT | PinFunc::LOW2BOOT | PinFunc::SpecialAtBoot)}, // ADC2_5/TOUCH5/HSPI_MISO/SD_DATA2 (must be LOW during boot, could be a output function only pin)
            {13, (PinFunc::OUT | PinFunc::IN)}, // ADC2_4/TOUCH4/HSPI_MOSI/SD_DATA3 (ADC2 cannot be used together with WiFi)
            {14, (PinFunc::OUT | PinFunc::IN)}, // ADC2_6/TOUCH6/HSPI_CLK/SD_CLK (ADC2 cannot be used together with WiFi)
            {15, (PinFunc::OUT | PinFunc::HIGH2BOOT | PinFunc::SpecialAtBoot)}, // ADC2_3/TOUCH3/HSPI_CS/SD_CMD (must be HIGH during boot, could be a output function only pin)
     
            {18, (PinFunc::OUT | PinFunc::IN)}, // VSPI_CLK
            {19, (PinFunc::OUT | PinFunc::IN)}, // VSPI_MISO
            {21, (PinFunc::OUT | PinFunc::IN)}, // I2C_SDA
            {22, (PinFunc::OUT | PinFunc::IN)}, // I2C_SCL
            {23, (PinFunc::OUT | PinFunc::IN)}, // VSPI_MOSI
            {25, (PinFunc::OUT | PinFunc::IN | PinFunc::AOUT)}, // ADC2_8/DAC1 (ADC2 cannot be used together with WiFi)
            {26, (PinFunc::OUT | PinFunc::IN | PinFunc::AOUT)}, // ADC2_9/DAC2 (ADC2 cannot be used together with WiFi)
            {27, (PinFunc::OUT | PinFunc::IN)}, // ADC2_7/TOUCH7 (ADC2 cannot be used together with WiFi)
            {32, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)}, // ADC1_4/TOUCH9/XTAL32
            {33, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)}, // ADC1_5/TOUCH8/XTAL32
            {34, (PinFunc::IN | PinFunc::AIN)}, // ADC1_6 (input only)
            {35, (PinFunc::IN | PinFunc::AIN)}, // ADC1_7 (input only)
            {36, (PinFunc::IN | PinFunc::AIN)}, // ADC1_0/SensVP (input only)
            {39, (PinFunc::IN | PinFunc::AIN)},  // ADC1_3/SensVN (input only)
            
    #elif defined(waveshare_esp32c3_zero)
            {0, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)},
            {1, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)},
            {2, MAKE_PIN_MASK_4(PinFunc::OUT | PinFunc::LOW2BOOT | PinFunc::SpecialAtBoot | PinFunc::AIN)}, // strapping pin
            {3, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)},
            {4, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)},
            {5, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)},
            {6, (PinFunc::OUT | PinFunc::IN)},
            {7, (PinFunc::OUT | PinFunc::IN)},
            {8, (PinFunc::OUT | PinFunc::IN)},
            {9, (PinFunc::OUT | PinFunc::HIGH2BOOT | PinFunc::SpecialAtBoot)}, // strapping pin
            {10, (PinFunc::OUT | PinFunc::IN)}, // onboard ws2812 LED
        //#if ARDUINO_USB_CDC_ON_BOOT || ARDUINO_USB_MODE
            {18, (PinFunc::Reserved | PinFunc::SpecialAtBoot)}, // USB_DP
            {19, (PinFunc::Reserved | PinFunc::SpecialAtBoot)}, // USP_DM
        //#else
        //    {18, (PinFunc::OUT | PinFunc::IN)}, // USB_DP
        //    {19, (PinFunc::OUT | PinFunc::IN)}, // USP_DM
        //#endif
            {20, MAKE_PIN_MASK_4(PinFunc::OUT | PinFunc::IN | PinFunc::SpecialAtBoot | PinFunc::UARTFLASH)}, // flash standard UART RxD
            {21, MAKE_PIN_MASK_4(PinFunc::OUT | PinFunc::IN | PinFunc::SpecialAtBoot | PinFunc::UARTFLASH)}, // flash standard UART TxD
    #elif defined(seeed_xiao_esp32c3)

            {2, MAKE_PIN_MASK_4(PinFunc::OUT | PinFunc::LOW2BOOT | PinFunc::SpecialAtBoot | PinFunc::AIN)}, // strapping pin
            {3, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)},
            {4, MAKE_PIN_MASK_4(PinFunc::OUT | PinFunc::IN | PinFunc::AIN | PinFunc::JTAG)},
            {5, MAKE_PIN_MASK_4(PinFunc::OUT | PinFunc::IN | PinFunc::AIN | PinFunc::JTAG)},
            {6, (PinFunc::OUT | PinFunc::IN | PinFunc::JTAG)},
            {7, (PinFunc::OUT | PinFunc::IN | PinFunc::JTAG)},
            {8, (PinFunc::OUT | PinFunc::IN)},
            {9, (PinFunc::OUT | PinFunc::HIGH2BOOT | PinFunc::SpecialAtBoot)}, // strapping pin
            {10, (PinFunc::OUT | PinFunc::IN)},
            {20, MAKE_PIN_MASK_4(PinFunc::OUT | PinFunc::IN | PinFunc::SpecialAtBoot | PinFunc::UARTFLASH)}, // flash standard UART RxD
            {21, MAKE_PIN_MASK_4(PinFunc::OUT | PinFunc::IN | PinFunc::SpecialAtBoot | PinFunc::UARTFLASH)}, // flash standard UART TxD
    #elif defined(waveshare_esp32c6_zero)
            {0, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)},
            {1, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)},
            {2, MAKE_PIN_MASK_4(PinFunc::OUT | PinFunc::LOW2BOOT | PinFunc::SpecialAtBoot | PinFunc::AIN)}, // strapping pin
            {3, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)},
            {4, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)},
            {5, (PinFunc::OUT | PinFunc::IN | PinFunc::AIN)},
            // underside pins 6-9, 12, 13
            {6, (PinFunc::OUT | PinFunc::IN | PinFunc::UNDERSIDE)},
            {7, (PinFunc::OUT | PinFunc::IN | PinFunc::UNDERSIDE)},
            {8, (PinFunc::OUT | PinFunc::IN | PinFunc::UNDERSIDE)}, // onboard ws2812 LED
            {9, MAKE_PIN_MASK_4(PinFunc::OUT | PinFunc::HIGH2BOOT | PinFunc::SpecialAtBoot | PinFunc::UNDERSIDE)}, // strapping pin
            {12, (PinFunc::Reserved | PinFunc::SpecialAtBoot | PinFunc::UNDERSIDE)}, // USB_DM
            {13, (PinFunc::Reserved | PinFunc::SpecialAtBoot | PinFunc::UNDERSIDE)}, // USP_DP
            
            {14, (PinFunc::OUT | PinFunc::IN)},
            {15, (PinFunc::OUT | PinFunc::IN)},
            {16, MAKE_PIN_MASK_4(PinFunc::OUT | PinFunc::IN | PinFunc::SpecialAtBoot | PinFunc::UARTFLASH)}, // flash standard UART TxD
            {17, MAKE_PIN_MASK_4(PinFunc::OUT | PinFunc::IN | PinFunc::SpecialAtBoot | PinFunc::UARTFLASH)}, // flash standard UART RxD
            {18, (PinFunc::OUT | PinFunc::IN)},
            {19, (PinFunc::OUT | PinFunc::IN)},
            {20, (PinFunc::OUT | PinFunc::IN)},
            {21, (PinFunc::OUT | PinFunc::IN)},
            {22, (PinFunc::OUT | PinFunc::IN)},
            // underside pin
            {23, (PinFunc::OUT | PinFunc::IN | PinFunc::UNDERSIDE)}, 

    #endif
            //{255, 0x00} // terminator item
        }; // const gpio_pin available_gpio_list[] {
        constexpr size_t available_gpio_list_size = sizeof(available_gpio_list) / sizeof(available_gpio_list[0]);

        constexpr PinFuncDef PinModeStrings[] = {
            {"Reserved", (PinFunc::Reserved)},
            {"SpecialAtBoot", (PinFunc::SpecialAtBoot)},
            {"LOW2BOOT", (PinFunc::LOW2BOOT)},
            {"HIGH2BOOT", (PinFunc::HIGH2BOOT)},
            {"OUT", (PinFunc::OUT)},
            {"IN", (PinFunc::IN)},
            {"AIN", (PinFunc::AIN)},
            {"AOUT", (PinFunc::AOUT)},
            {"UNDERSIDE", (PinFunc::UNDERSIDE)},
            {"UARTFLASH", (PinFunc::UARTFLASH)},
            {"JTAG", (PinFunc::JTAG)},
            //{nullptr, 0} // terminator item
        };
        constexpr size_t PinModeStrings_size = sizeof(PinModeStrings) / sizeof(PinModeStrings[0]);
        
        PinReservationType reservedPins[available_gpio_list_size];

        std::string describePinFunctions(DALHAL_GPIO_MGR_PINFUNC_TYPE pinFuncMask) {
            std::string result;
            for (int i=0; i < (int)PinModeStrings_size; ++i) {
                const PinFuncDef& pinModeDef = PinModeStrings[i];
                if (pinModeDef.Name == nullptr) continue; // failsafe

                if (pinFuncMask & pinModeDef.func) {
                    if (!result.empty()) result += "|";
                    result += pinModeDef.Name;
                }
            }
            return result.empty() ? "None" : result;
        }

        const gpio_pin& GetPinInfo(uint8_t pin, int& index) {
           for (int i = 0; i < (int)available_gpio_list_size; i++) {
                const gpio_pin& pinDef = available_gpio_list[i];
                if (pinDef.pin == pin) {
                    index = i;
                    return pinDef;
                }
           }
           index = -1;
           return NA_PIN;
        }

        void ClearAllReservations() {
            for (int i=0;i<(int)available_gpio_list_size;i++) {
                if (reservedPins[i] == PinReservationType::DYNAMIC) {
                    reservedPins[i] = PinReservationType::FREE;
                }
            }
        }

        CheckPinResult TryReservePin(uint8_t pin, DALHAL_GPIO_MGR_PINFUNC_TYPE pinFuncMask) {
            int index = 0;
            const gpio_pin& pinInfo = GetPinInfo(pin, index);

            if (index == -1) {
                return CheckPinResult::NotFound;
            }

            if (reservedPins[index] != PinReservationType::FREE) {
                return CheckPinResult::InUse;
            }

            if ((pinFuncMask & pinInfo.func) != pinFuncMask) {
                return CheckPinResult::ModeMismatch;
            }

            // state change happens ONLY here
            reservedPins[index] = PinReservationType::DYNAMIC;

            return CheckPinResult::Success;
        }

        CheckPinResultError GetCheckPinResultError(CheckPinResult res, uint8_t pin, DALHAL_GPIO_MGR_PINFUNC_TYPE pinFuncMask) {
            if (res == CheckPinResult::InUse) {
                return {BASE_MSG_TYPE_FUNC("CheckIfPinAvailable error - pin allready reserved: "), std::to_string(pin)};
            } else if (res == CheckPinResult::ModeMismatch) {
                int index = 0;
                const gpio_pin& pinInfo = GetPinInfo(pin, index); // index is passed by ref
                std::string errStr = Convert::toBin(pinFuncMask) + " & " + Convert::toBin(pinInfo.func);
                return {BASE_MSG_TYPE_FUNC("CheckIfPinAvailable error - pinmode mismatch: "), errStr};
            } else if (res == CheckPinResult::NotFound) {
                return {BASE_MSG_TYPE_FUNC("Pin to reserve - not found: "), std::to_string(pin)};
            }
            return {BASE_MSG_TYPE_FUNC("unknown"),nullptr};
        }

        void triStateAvailablePins() {
#if defined(ESP32)

            gpio_config_t io_conf{};
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io_conf.intr_type = GPIO_INTR_DISABLE;

            uint64_t mask = 0;

            for (int i = 0; i < (int)available_gpio_list_size; ++i) {
                const auto& g = available_gpio_list[i];

                if (g.func & (
                    PinFunc::Reserved |
                    PinFunc::SpecialAtBoot |
                    PinFunc::UARTFLASH
                )) {
                    continue;
                }

                mask |= (1ULL << g.pin);
            }

            io_conf.pin_bit_mask = mask;

            esp_err_t res = gpio_config(&io_conf);
            if (res != ESP_OK) {
                printf("Failed to tri-state GPIO manager pins: %d\n", res);
            }

#elif defined(ESP8266)

            for (int i = 0; i < (int)available_gpio_list_size; ++i) {
                const auto& g = available_gpio_list[i];

                if (g.func & (
                    PinFunc::Reserved |
                    PinFunc::SpecialAtBoot |
                    PinFunc::UARTFLASH
                )) {
                    continue;
                }

                pinMode(g.pin, INPUT);   // closest equivalent to "tri-state"
            }
#endif
        }

        std::string GetList(ZeroCopyString& zcMode)
        {
            GPIO_manager::PrintListMode listMode = GPIO_manager::PrintListMode::Hex; // set the default here

            ZeroCopyString zcPrintMode = zcMode.SplitOffHead('/');
            if (zcPrintMode.NotEmpty()) {
                if (zcPrintMode == DALHAL_CMD_EXEC_GPIO_LIST_MODE_STRING)
                    listMode = GPIO_manager::PrintListMode::String;
                else if (zcPrintMode == DALHAL_CMD_EXEC_GPIO_LIST_MODE_BINARY)
                    listMode = GPIO_manager::PrintListMode::Binary;
                else if (zcPrintMode == DALHAL_CMD_EXEC_GPIO_LIST_MODE_HEX)
                    listMode = GPIO_manager::PrintListMode::Hex;
                //else // default set above
                //    listMode = DALHAL_CMD_EXEC_GPIO_LIST_MODE_DEFAULT; // default
            }
            std::string strList;// = "{";
            
    #if defined(ESP8266)
            strList.append("\"MCU\":\"ESP8266\",");
    #elif defined(ESP32)
            strList.append("\"MCU\":\"ESP32\",");
    #elif defined(_WIN32) || defined(__linux__)
            strList.append("\"MCU\":\"PC_SIM\",");
    #endif
            strList.append("\"variant\":\""); strList.append(Info::getESPVariant()); strList.append("\",");
            if (listMode != PrintListMode::String) {
                strList.append("\"PinModes\":{");
                
                for (int i=0;i<(int)PinModeStrings_size;++i)
                {
                    if (i>0) { strList += ',';}
                    strList += '"';
                    uint8_t modeMask = PinModeStrings[i].func;
                    if (listMode == PrintListMode::Binary) {
                        strList.append(Convert::toBin(modeMask));
                    }
                    else {
                        strList.append(Convert::toHex(modeMask));
                    }
                    strList.append("\":\"");
                    strList.append(PinModeStrings[i].Name);
                    strList += '"';
                }
                strList.append("},");
            }
            strList.append("\"list\":{");
            bool first = true;
           // if (available_gpio_list_lenght == -1) set_available_gpio_list_length();
            for (int i=0;i<(int)available_gpio_list_size;i++)
            {
                if (first == false)
                    strList.append(",");
                else
                    first = false;

                strList.append("\"");
                strList.append(std::to_string(available_gpio_list[i].pin));
                strList.append("\":\"");
                uint8_t modeMask = available_gpio_list[i].func;
                if (listMode == PrintListMode::String)
                    strList.append(describePinFunctions(modeMask).c_str());
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