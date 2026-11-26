# Porting changelog: breaking free from the Arduino framework

[Svenska](PORTING_CHANGELOG.sv.md)

DALHAL is being moved off its hard dependency on the Arduino framework and on PlatformIO forks (pioarduino).
The goal is that DALHAL runs on a thin Arduino-like port layer, the same way the PC simulation already does,
on top of the vendor SDKs (ESP-IDF, later Teensy 4.x, RP2040/RP2350, STM32).
The Arduino builds (ESP32, ESP8266) keep working the whole time.

**Where it stands:** work in progress. All real device builds in `platformio.ini` still use `framework = arduino`.
The only build without the Arduino framework is `targets/esp-idf`. It has WiFi, the web UI, OTA, failsafe and HomeAssistant,
but no external drivers, ThingSpeak/REST, mDNS or FSBrowser yet, and it is not tested on hardware (see its [README](../targets/esp-idf/README.md)). The DALHAL core still uses the Arduino
*API* (`millis`, `digitalWrite`, `Serial`, `F()` ...) on purpose: on non Arduino backends it comes from the thin ports in `ports/`.
ESP8266 will most likely stay on the Arduino core, the backend model supports both.

The layers and rules are described in [PLATFORM_CONTRACT.md](PLATFORM_CONTRACT.md).
This file lists what has been done, one chapter per day, newest first.

## Plan

| Step | What | Status |
|---|---|---|
| 1 | PC simulation builds again | done |
| 2 | Ports in `ports/common` + `ports/pc` | done |
| 3 | ESP-IDF target without Arduino (`targets/esp-idf`) | builds and links, not tested on hardware |
| 4 | Network, web server/WebSocket, OTA, failsafe, HomeAssistant on ESP-IDF | builds and links, not tested on hardware; mDNS, FSBrowser, W5500, HTTP client left |
| 5 | Drivers one at a time (I2C, OneWire, DHT, WS2812, REGO600 ...) | |

## Builds

| Build | Command |
|---|---|
| ESP32 (Arduino) | `pio run -e esp32dev` |
| ESP8266 (Arduino) | `pio run -e esp12e` |
| PC simulation | `pio run -e pc_sim`, for gdb `pio run -e pc_sim_debug`, or CMake in `pc_simulation/` |
| ESP32 on plain ESP-IDF | `pio run -d targets/esp-idf` |

---

## 2026-11-26

### Added
- **Network on ESP-IDF** (step 4a): `ports/esp-idf/WiFi`, the part of the Arduino WiFi API that DALHAL uses, on esp_wifi/esp_netif,
  so `WiFiConnectionManager`, `WiFiLink`, the config portal AP, the failsafe AP and the `wifi/...` commands are built unchanged.
  Credentials live in esp_wifi's NVS storage, the same as arduino-esp32, so the Arduino and the ESP-IDF firmware share them.
  `ports/esp-idf/WiFiClient`: TCP client on lwIP sockets (`Network::TcpClient`).
- **Web server and WebSocket on `esp_http_server`** (step 4b): port 82, `/ws`, LittleFS `/api/...` before the built in frontend (gzip),
  commands through the `CommandExecutor` queue, `Broadcast` to all clients, safe from any task.
- **OTA on ESP-IDF** (step 4c): `ports/esp-idf/ArduinoOTA` speaks the espota protocol of arduino-esp32 (port 3232, optional password),
  firmware and filesystem images, the MD5 is checked before anything is activated. `pio run -t upload` with an IP works as before.
- **Failsafe on ESP-IDF** (step 4d): after a crash only the network failsafe, OTA, an HTTP page on port 80 (`/`, `/reset`) and the
  WebSocket API in failsafe mode are started, restart after 10 minutes, as on the Arduino target. `system/EnterRecoveryMode` over WebSocket.
- **HomeAssistant on ESP-IDF**: MQTT via the PubSubClient fork on `Network::TcpClient`.
- `Platform::ResetWasCrash()`, `ResetReasonStr()`, `FlashSize()`, `BackendName()`, `DALHAL_HAS_WIFI` capability macro.
- **Command API over the serial console on ESP-IDF** (step 3c): `CommandExecutor` and `SerialAPI` are built on ESP-IDF.
  `app_main` uses the same `DALHAL::begin()` / `DALHAL::loop()` (`src/DALHAL/DALHAL.cpp`) as the Arduino target,
  plus HeartbeatLed and Scheduler. Firmware: 507 KB flash.
- `system/info` on ESP-IDF returns chip, framework and heap.
- NTP on ESP-IDF via `esp_sntp`, same time zone and servers as on Arduino (needs the network, step 4).
- `[env:pc_sim_debug]`: PC simulation with `-O0 -ggdb` for gdb, its own build folder so switching does not rebuild everything.

### Changed
- `IPAddress` in `ports/common` behaves like Arduino's: `uint32_t` conversion, `fromString`, `==`, printable with `Serial.print`.
- WiFi commands are guarded by `DALHAL_HAS_WIFI` (Arduino core and ESP-IDF), not by the backend.
- `PubSubClient_ErrorStrings.h` (lib fork) includes `<WString.h>` instead of guessing what `__FlashStringHelper` is.
- **`SerialAPI` reads lines without blocking**: characters are collected until `\n`, then the command runs.
  Before, `Serial.readStringUntil` could block the main loop for up to one second.
  A command now needs a line ending (the PlatformIO monitor sends CR LF on Enter). Lines over 1024 chars are discarded.
- WiFi commands (`wifi/scan`, `wifi/set_b64`, `wifi/set_json`) only exist on the Arduino backend, as on PC.
- `DALHAL_WebSocketAPI.cpp` (ESPAsyncWebServer) is only built on the Arduino backend.

### Fixed
- First hardware run of the ESP-IDF firmware (esp32dev, USB flash): boots, WiFi STA, LittleFS with the existing
  files, NTP, WebSocket API and `system/info` work. Problems found and fixed:
  - OTA stopped with "Receive Failed": `WiFiClient::available()` uses `ioctl(FIONREAD)`, which lwIP only
    supports with `CONFIG_LWIP_SO_RCVBUF=y` (now in `sdkconfig.defaults`). This also affected MQTT.
  - OTA erased the whole image area in `esp_ota_begin` before receiving, now `OTA_WITH_SEQUENTIAL_WRITES`
    (erase per sector while writing, as Arduino `Update`).
  - No board define on ESP-IDF, so the GPIO list was wrong ("GPIO_PIN mask error", "Pin to reserve - not found").
    The component now defines `DALHAL_BOARD` from `IDF_TARGET` (esp32 = `ESP32DEV`), overridable.
- `system/info` answers again (it returned an empty string since the old `Info::getESP_info()` was commented out):
  now `Info::getInfo(sbs)` (was `getESP_info()`), streamed with `BlockStreamer` like the other commands, the same on every platform via `Platform::`
  (chip, framework, build version, uptime, reset reason, heap, flash size, LittleFS total/used).
- `DALHAL_HA_DeviceDiscovery`: `%.*s` got a `uint32_t` precision, it must be `int` (wrong on 32 bit ARM/RISC-V).
- `DeviceUID`: `%X` for `uint32_t`, now `PRIX32`. `esp_read_mac` needs `<esp_mac.h>` since ESP-IDF 5.
- Two more `JsonVariant` temporaries bound to references (HomeAssistant schema).

### Build and tooling
- **pioarduino removed from the PlatformIO installation.** The penv had both the `platformio` and the `pioarduino`
  package installed into the same module folder. pioarduino's IDE generator together with the original's
  `.gitignore` template overwrote the project `.gitignore` every time VS Code regenerated IntelliSense
  (this staged `secrets/` once, nothing was committed). Now only the original `platformio` 6.2.0 is installed,
  it never overwrites an existing `.gitignore`. Do not install pioarduino again, ESP-IDF comes from the official
  `platformio/platform-espressif32`.

---

## 2026-11-25

### Added
- **Platform contract** ([PLATFORM_CONTRACT.md](PLATFORM_CONTRACT.md)): layers A (pure software) to E (platform specific),
  what each backend must provide, port layout and platform macros.
- **Platform layer (layer E)** in `src/System/Platform/`:
  - `Platform.h`: `DALHAL_PLATFORM_*` (chip) and `DALHAL_BACKEND_ARDUINO / _ESPIDF / _PC` (what provides the Arduino subset),
    `Restart()`, `Abort()`, `FreeHeap()`, `LargestFreeBlock()`, `ChipName()`.
  - `Pwm`: PWM output for arduino-esp32 2.x and 3.x, ESP-IDF (LEDC driver), ESP8266 and PC.
  - `FastGpio`: fast GPIO with edge interrupts and ISR safe writes (was direct ESP-IDF `gpio_*` calls in the devices).
- **Port layout** (step 2):
  - `ports/common`: platform independent Arduino subset (`String`, `Print`, `IPAddress`, `pgmspace.h`, `ArduinoCommon.h`, `WProgram.h`).
  - `ports/pc`: PC implementations (time, sockets, files, `Serial`, `DALHAL_WebSocketAPI_PC`).
  - `ports/pc/stubs`: fake hardware drivers for the simulation.
  - `ports/esp-idf`: `Arduino.h` (esp_timer, FreeRTOS, gpio driver, console `Serial`), `LittleFS` (on esp_littlefs,
    same partition and mount point as arduino-esp32), `Ticker` (esp_timer), placeholder WebSocket API.
  - `attic/`: files nothing referenced any more, kept for review.
- **ESP-IDF target** `targets/esp-idf` (steps 3a/3b): own small PlatformIO project with `framework = espidf`
  (official espressif32 platform, ESP-IDF 5.5), also buildable with `idf.py`. Core, JSON config, script engine and the basic
  devices build and link. Devices without ported drivers or network are left out of the registry on ESP-IDF
  (`DALHAL_REGISTRY_ARDUINO_ONLY_DEVICES`).
- `[env:pc_sim]` in `platformio.ini` works (was broken), gives IntelliSense for the PC code. `tools/pio_native.py` adds the
  sources outside `src/`.
- `DALHAL_WebSocketAPI_Backend.h`: one header that picks the WebSocket API of the current backend.

### Changed
- **Arduino `String` removed from the DALHAL core**:
  - `ZeroCopyString` can point straight at flash strings (`F("...")`). On ESP8266 the sinks copy straight from flash,
    on all other targets this costs nothing.
  - ~190 `String(F(...)).c_str()` replaced by `F(...)` or `ToStdString(F(...))`.
  - Own helpers (`b64urlEncode`, `convertISO88591toUTF8`, `getSSID` ...) use `std::string`.
  - 40 headers had `#include <Arduino.h> // Needed for String class`, removed or moved to the .cpp with what is really used.
  - `String` is only left in Arduino platform code (web server, WiFi, HTTP client, OTA, FSBrowser).
- Guards that meant "there is a separate flash string type" test `DALHAL_FLASH_STRING_IS_CHAR` instead of "not PC".
- PC port renamed `DALHAL_WebSocketAPI_Windows` → `DALHAL_WebSocketAPI_PC`.
- `PWM_Servo`, `Actuator`, `LatchingRelay` use `Platform::Pwm` / `Platform::FastGpio` instead of ESP-IDF and ledc calls.
- `Info::getESPVariant()` (always returned "") replaced by `Platform::ChipName()`.
- The PC simulation serves the real frontend from `FrontendFiles/www` (gzip), so the web UI can be developed without flashing.

### Fixed
- Actuator: drive to max set the wrong direction. LatchingRelay: drive to set had the wrong data level.
- Actuator / LatchingRelay: the end stop ISR could disable the wrong interrupt.
- `GPIO_Manager`: a `std::string` was constructed from `nullptr` (undefined behaviour on every platform).
- `DALHAL_Value`: `%d` / `%u` for `int32_t` / `uint32_t`, which are `long` on Xtensa, RISC-V and ARM, now `PRId32` / `PRIu32`.
- REGO600: an error message printed 16 copies of a char on PC instead of a hex number.
- Six places bound a `JsonVariant` temporary to a reference, now by value.
- PC simulation build fixed (includes, `String` class, renamed script engine functions).

### Build and tooling
- `output.map` (linker map) is in `.gitignore`.
- The PC builds need the local patches of `Time` in `.pio/libdeps/esp32dev` for CMake; `pc_sim` in PlatformIO uses the
  original library with `-D __time_t_defined` and `WProgram.h` instead.

---

## 2026-11-24

### Added
- **Network abstraction** `src/System/Network`: `Network::IsUp()`, `LocalIP()`, `HostByName()`, `Network::TcpClient`,
  up/down events, failsafe start. Links: `WiFiLink` (wraps `WiFiConnectionManager`, config portal only with WiFi)
  and `EthW5500Link` (W5500 in MAC raw mode under lwIP, static fallback IP from build flags, not compiled yet).
  HomeAssistant, ThingSpeak, REST and OTA use it instead of WiFi directly.

### Fixed
- HomeAssistant: re-subscribes to the command topics after an MQTT reconnect (switches stopped reacting).
