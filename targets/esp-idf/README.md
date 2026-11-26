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
| Devices: script variables, DIN/DOUT/DPOUT, ADC, button, container, actuators, latching relay, PWM servo, TX433 | builds and links |
| `analogRead` | returns 0, ADC oneshot driver todo |
| WebSocket API | placeholder without a server (`ports/esp-idf/DALHAL_WebSocketAPI`), esp_http_server is step 4 |
| Command executor, Serial API, network, OTA, failsafe | not built yet |
| I2C, OneWire, DHT, WS2812, REGO600, ThingSpeak, HomeAssistant, REST | not built yet (`DALHAL_REGISTRY_ARDUINO_ONLY_DEVICES` in the device registry) |
| Running on hardware | not tested yet |
