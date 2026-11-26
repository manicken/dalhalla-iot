# DALHAL platform contract

What DALHAL needs from the platform it runs on. Every backend (Arduino core, ESP-IDF, PC simulation) has to provide this, and nothing more.

The goal is that porting DALHAL to a new MCU means writing a backend for this contract, not touching DALHAL code.

Survey date: 2026-11-25 (from `#include`s and API usage under `src/`). Update this file when DALHAL starts using something new.

## Backends

| Backend | Platforms | Provides layer A-D by | Status |
|---|---|---|---|
| Arduino core | ESP32 (arduino-esp32), ESP8266, Teensy 4.x (Teensyduino), RP2040/RP2350 (arduino-pico), STM32 (STM32duino) | the core and its libraries | ESP32 2.x and ESP8266 in use, ESP32 3.x in progress |
| ESP-IDF native | ESP32 family | `ports/common` + `ports/esp-idf`, built from `targets/esp-idf` | in progress: core, script engine and the basic devices build and link, see `targets/esp-idf/README.md` |
| PC simulation | Windows (MinGW), Linux | `ports/common` + `ports/pc` (real) and `ports/pc/stubs` (fake hardware), built from `pc_simulation/CMakeLists.txt` | in use |

Layer E has to go behind DALHAL's own platform API for **every** backend.

## Port layout

Non Arduino backends get layer B (and the parts of C they support) from `ports/`:

| Folder | Contents | Rule |
|---|---|---|
| `ports/common` | `ArduinoCommon.h` (constants, `byte`), `WString`, `Print`/`Printable`, `IPAddress`, `pgmspace.h` | plain C++, no host OS or vendor SDK, shared by every non Arduino backend |
| `ports/pc` | `Arduino.h` (time via `std::chrono`, printing GPIO stubs, `Serial` on `std::cout`), `Stream`, `Ticker`, `LittleFS` (on disk), `WiFi`, `WiFiClient`, `HTTPClient` (winsock), `DALHAL_WebSocketAPI_PC` | host OS implementations |
| `ports/pc/stubs` | DHTesp, OneWire, DallasTemperature, WS2812FX, Adafruit GFX/SSD1306, `Wire` | fake hardware drivers, replaced by real drivers (layer D) on hardware backends |
| `ports/<backend>` | e.g. `ports/esp-idf` | the backend specific part, its own `Arduino.h` includes `ArduinoCommon.h` |

`pc_simulation/` is the PC target: `main_pc.cpp`, the command loop, the CMake build and the runtime data (`hal/`, `scripts/`).
Files that nothing references any more are kept in `attic/` for review.

## A. Pure software, works everywhere

No porting needed.

- C/C++ standard library: `string`, `vector`, `functional`, `chrono`, `mutex`, `queue`, `stack`, `memory`, C headers
- ArduinoJson 6
- PubSubClient (own fork in `lib/PubSubClient`, only needs a `Client`)
- TimeLib, TimeAlarms
- fmt

## B. Arduino core subset

Small API surface, used everywhere. On PC it exists as ports/stubs, on ESP-IDF it is a thin shim.

| API | Usage | Notes |
|---|---|---|
| `pinMode`, `digitalRead`, `digitalWrite` | ~50 calls | |
| `analogRead`, `analogWrite` | 1 file | |
| `millis`, `micros`, `delay`, `yield` | ~95 calls | |
| `delayMicroseconds` | 26 calls, all in `Drivers/RF433.cpp` | bit-banged protocol, timing critical |
| `noInterrupts`, `interrupts` | `API/DALHAL_CommandExecutor.h` | command queue lock |
| `Serial`, `Serial1`, `Print`, `Stream` | ~250 calls | `Serial1` is used as a debug channel |
| `String` | only Arduino platform code: `Serial.readStringUntil`, AsyncWebServer, `HTTPClient::getString`, `IPAddress::toString`, OTA, FSBrowser, failsafe server | the DALHAL core uses `std::string`, flash strings go through `ZeroCopyString(F("..."))` / `ToStdString(F("..."))` |
| `F()`, `PSTR`, `FPSTR`, `*_P()`, `pgm_read_*` | ~1300 | flash strings, macros exist on all targets (no-ops where flash is memory mapped) |
| `IPAddress` | network code | |

## C. Network and system services

| Service | Used by | Abstraction |
|---|---|---|
| WiFi / Ethernet link, `WiFiClient` | HomeAssistant, ThingSpeak, REST, OTA | `System/Network` (`Network::IsUp`, `Network::TcpClient` ...) |
| `HTTPClient` | ThingSpeak, REST_Cmd, REST_Value | none yet |
| ESPAsyncWebServer | WebSocket API, FS browser, failsafe server | none yet, largest single item for a non-Arduino backend |
| `ArduinoOTA`, `HTTPUpdate` | `System/OTA`, failsafe | none yet |
| mDNS | `System/System.cpp` | none yet |
| `LittleFS`, `FS`, `SD_MMC` | config, scripts, FS browser, logs | `Support/LittleFS_ext` (partial) |

## D. Hardware drivers

Arduino libraries. On PC these are stubs with fake data. A non-Arduino backend needs real drivers.

| Driver | Used by |
|---|---|
| OneWire + DallasTemperature | `Devices/Sensors/OneWireTemp` |
| DHTesp | `Devices/Sensors/DHT` |
| WS2812FX (Adafruit NeoPixel) | `Devices/Lights` |
| Adafruit SSD1306 + GFX, `Wire` | `Devices/Display` |
| `SPI` | W5500 Ethernet link |
| RF433 (own driver, bit-banged) | `Devices/RF433` |
| REGO600 (own driver, UART) | `Devices/REGO600` |

The own drivers (RF433, REGO600) only use layer B and are portable as is.

## E. Platform specific, must be behind the DALHAL platform API

This blocks every port, also to other Arduino cores.

The platform API lives in `src/System/Platform/`. Each file has one section per platform, a new port adds its section there.

| Where | What | API | Status |
|---|---|---|---|
| `Devices/Actuators/Actuator`, `Devices/Actuators/LatchingRelay` | fast GPIO with edge interrupts and ISR safe writes (was direct ESP-IDF `gpio_*`, `GPIO.out_w1ts`) | `Platform::FastGpio` (`FastGpio.h`) | done: ESP32. ESP8266/PC are no-ops and the devices refuse to drive (`FastGpio::Available == false`), as before |
| `Devices/Actuators/PWM_Servo` | PWM output (was `ledcSetup`/`ledcAttachPin`, removed in arduino-esp32 3.x) | `Platform::Pwm` (`Pwm.h`) | done: ESP32 2.x and 3.x, ESP8266, PC |
| `API/DALHAL_CommandExecutor`, `System/System.cpp` | restart, abort (recovery mode), free heap | `Platform::Restart/Abort/FreeHeap/LargestFreeBlock` (`Platform.h`) | done: ESP32, ESP8266, PC |
| `System/Info` | chip, flash and heap diagnostics | per platform branches in `Info.cpp` | already split per platform, a port adds a branch |
| `main.cpp` | `esp_reset_reason`, `esp_partition_*` (ESP32 core dump to LittleFS) | ESP32 only feature | stays ESP32 specific |
| `System/DeviceUID` | MAC based unique id (`esp_read_mac`) | per platform branches in `DeviceUID.cpp` | already split per platform, a port adds a branch |
| `Core/Manager/DALHAL_GPIO_Manager` | pin capabilities per chip | per platform pin tables | todo |
| `Devices/HomeAssistant` (MQTT connect diagnostics) | `lwip/*` | lwIP based backends only | ESP32 only today |

## Platform identification

Code should test for what it needs, not for a chip:

- `DALHAL_PLATFORM_ESP32`, `_ESP8266`, `_TEENSY4`, `_RP2040`, `_STM32`, `_PC` for the platform itself
- `DALHAL_BACKEND_ARDUINO`, `_ESPIDF`, `_PC` for what provides the Arduino subset (layer B). `ESP32` alone does not tell
  arduino-esp32 and plain ESP-IDF apart, the ESP-IDF target defines `ESP32` as the chip family, the same as the Arduino core does.
  Code that uses an Arduino library (ESPAsyncWebServer, WiFi.h, HTTPClient ...) tests `DALHAL_BACKEND_ARDUINO`.
- `DALHAL_FLASH_STRING_IS_CHAR` (set by `ports/common/WString.h`): `F()` is a plain `const char*`, skip `const __FlashStringHelper*` overloads
- `DALHAL_HAS_*` for capabilities, e.g. `DALHAL_HAS_WIFI`, `DALHAL_HAS_LWIP_SOCKETS`, `DALHAL_HAS_LITTLEFS`, `DALHAL_HAS_ASYNC_WEBSERVER`, `DALHAL_HAS_ARDUINO_OTA`

Today the code mostly uses `defined(ESP32)` (169) and `defined(ESP8266)` (147). They are replaced gradually by what they actually mean, e.g. "real hardware, not PC", "has WiFi" or "flash strings are in a separate address space" (ESP8266/AVR only).
