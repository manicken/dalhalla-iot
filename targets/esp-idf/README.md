# DALHAL on plain ESP-IDF

DALHAL without the Arduino core: the Arduino subset that DALHAL uses comes from
`ports/common` (platform independent) and `ports/esp-idf` (time, GPIO, Serial, LittleFS, Ticker on ESP-IDF APIs).

## Build

PlatformIO is only used to get ESP-IDF and the toolchain, the build is the normal ESP-IDF CMake build.

```
pio run -d targets/esp-idf              # build
pio run -d targets/esp-idf -t upload    # flash
pio device monitor -b 115200
```

With a standalone ESP-IDF (>= 5.1) the same folder builds with `idf.py build` from `targets/esp-idf`.

ArduinoJson is taken from `.pio/libdeps/esp32dev/ArduinoJson` (same as the PC build), so build the
`esp32dev` env of the root project once first. esp_littlefs comes from the ESP-IDF component manager
(`components/dalhal/idf_component.yml`).

## Layout

| Path | What |
|---|---|
| `main/app_main.cpp` | mounts LittleFS, loads the config and scripts, runs the DALHAL loop |
| `components/dalhal/` | DALHAL core as an ESP-IDF component, the source list is in its `CMakeLists.txt` |
| `sdkconfig.defaults` | 1000 Hz tick, 8 KB main task, 4 MB flash, `partitions_32mbit.csv` |

The filesystem is mounted at `/littlefs` on the partition `spiffs`, the same as arduino-esp32,
so the partition table and the LittleFS image of the Arduino builds work unchanged.

## Status

| Part | State |
|---|---|
| Core, JSON config, script engine | builds and links |
| Command API over the serial console (`SerialAPI`, `CommandExecutor`) | builds and links |
| `DALHAL::begin()` / `DALHAL::loop()`, HeartbeatLed, Scheduler | builds and links, shared with the Arduino target |
| WiFi (`ports/esp-idf/WiFi`, `WiFiConnectionManager`, `WiFiLink`, config portal AP, `wifi/...` commands) | builds and links, credentials shared with the Arduino firmware (same NVS) |
| TCP client (`ports/esp-idf/WiFiClient`, lwIP sockets) | builds and links, used by `Network::TcpClient` |
| Web server and WebSocket on `esp_http_server` (port 82, `/ws`, frontend with gzip) | builds and links |
| OTA (`ports/esp-idf/ArduinoOTA`, espota protocol, firmware and filesystem, MD5 checked) | builds and links |
| Failsafe after a crash (network failsafe, OTA, HTTP page on port 80, WebSocket in failsafe mode, 10 min timeout), `system/EnterRecoveryMode` | builds and links |
| NTP (esp_sntp) | builds and links |
| HomeAssistant (MQTT via the own PubSubClient fork on `Network::TcpClient`) | builds and links |
| Devices: script variables, DIN/DOUT/DPOUT, ADC, button, container, actuators, latching relay, PWM servo, TX433 | builds and links |
| `analogRead` | returns 0, ADC oneshot driver todo |
| mDNS, FSBrowser, W5500 Ethernet, Scheduler functions from main.cpp | not built yet |
| ThingSpeak, REST | not built yet (need an HTTP client port) |
| I2C, OneWire, DHT, WS2812, REGO600 | not built yet (step 5, `DALHAL_REGISTRY_ARDUINO_ONLY_DEVICES` in the device registry) |
| Running on hardware | not tested yet |

## First test on hardware

```
pio run -e esp32dev -t uploadfs          # from the repo root: the same LittleFS image as the Arduino build
pio run -d targets/esp-idf -t upload     # the ESP-IDF firmware
pio device monitor -b 115200
```

Then type a command and press Enter, e.g. `system/info`, `system/heap` or `help`.
With WiFi up: the web UI on `http://<ip>:82/`, OTA with `pio run -d targets/esp-idf -t upload --upload-port <ip>`.
The serial API executes a command when the line ends (`\n`), the PlatformIO monitor sends CR LF on Enter.
