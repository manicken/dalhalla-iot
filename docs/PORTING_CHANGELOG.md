# Porting changelog: breaking free from the Arduino framework

[Svenska](PORTING_CHANGELOG.sv.md)

DALHAL is being moved off its hard dependency on the Arduino framework and on PlatformIO forks (pioarduino).
The goal is that DALHAL runs on a thin Arduino-like port layer, the same way the PC simulation already does,
on top of the vendor SDKs (ESP-IDF, later Teensy 4.x, RP2040/RP2350, STM32).
The Arduino builds (ESP32, ESP8266) keep working the whole time.

**Where it stands:** work in progress. All real device builds in `platformio.ini` still use `framework = arduino`.
The only build without the Arduino framework is `targets/esp-idf`. It has WiFi, the web UI, OTA, failsafe and HomeAssistant,
but no external drivers, ThingSpeak/REST, mDNS or file listing yet, and on hardware so far boot, WiFi, LittleFS, NTP, WebSocket API, OTA and failsafe are verified (see its [README](../targets/esp-idf/README.md)). The DALHAL core still uses the Arduino
*API* (`millis`, `digitalWrite`, `Serial`, `F()` ...) on purpose: on non Arduino backends it comes from the thin ports in `ports/`.
ESP8266 will most likely stay on the Arduino core, the backend model supports both.

The layers and rules are described in [PLATFORM_CONTRACT.md](PLATFORM_CONTRACT.md).
This file lists what has been done, one chapter per day, newest first.

## Plan

| Step | What | Status |
|---|---|---|
| 1 | PC simulation builds again | done |
| 2 | Ports in `ports/common` + `ports/pc` | done |
| 3 | ESP-IDF target without Arduino (`targets/esp-idf`) | runs on esp32dev |
| 4 | Network, web server/WebSocket, OTA, failsafe, HomeAssistant on ESP-IDF | WiFi, WebSocket, OTA, failsafe verified on esp32dev, HomeAssistant not yet; mDNS, SD card, W5500, HTTP client left |
| 5 | Drivers one at a time (I2C, OneWire, DHT, WS2812, REGO600 ...) | |

## Builds

| Build | Command |
|---|---|
| ESP32 (Arduino) | `pio run -e esp32dev` |
| ESP8266 (Arduino) | `pio run -e esp12e` |
| PC simulation | `pio run -e pc_sim`, for gdb `pio run -e pc_sim_debug`, or CMake in `pc_simulation/` |
| ESP32 on plain ESP-IDF | `pio run -d targets/esp-idf` |

---

## 2026-11-28

### Changed
- Config tab: the device tree has collapsible nodes (▸/▾, double click also toggles), collapsed at start so only the
  root devices show; child count and problem dots stay visible on collapsed nodes. Selecting an item from elsewhere
  (form, link, problem list, a new item) expands the way to it. Expand all / collapse all buttons. The expanded nodes
  are remembered in the browser (by uid path), so a reload keeps the tree as it was.
- Frontend: all elements are built with `newEl()` from `domHelpers.js` (loaded first) instead of
  createElement/className/textContent/appendChild chains: `newEl('div', { cls, text, title, onclick, styles }, ...children)`,
  plus `downloadUrl()`. config.js lost its own copy (`h`), the command suggestions are built as elements
  (texts from the device are shown as text, no longer as HTML). Size, as embedded (gzip -9 per file): 52016 -> 51924 bytes
  (the code files shrink by about 660 bytes, domHelpers.js adds 181); mainly it is shorter and easier to read.
  Checked in a real browser on esp32dev: console buttons/suggestions/log, WiFi dialog, files tree/menu/editor, config tab.
- Config tab: the problem list shows errors before warnings.
- `tools/gen_frontend.py` minifies JS, CSS and HTML before gzip: comments, indentation and trailing spaces out.
  Every line keeps its line number (a comment or empty line becomes an empty line), so an error at line N in the
  browser dev tools is at line N in the source file; that costs about 300 bytes after gzip.
  A small tokenizer copies strings, template literals (nested `${}`) and regex literals exactly, so `//` in a URL
  is not a comment. No new dependency. Embedded frontend: 51960 -> 41870 bytes (-19 %). Checked: every JS file has
  an identical syntax tree before/after (TypeScript parser) with every node on the same line, and the whole UI works on esp32dev.
  `DALHAL_FRONTEND_MINIFY=0` keeps the files as they are.

## 2026-11-27

### Added
- ESP-IDF: one HTTP server for everything, `ports/esp-idf/HttpServer` (esp_http_server on port 80).
  It only owns the server, the modules add their routes with `HttpServer::On()`; the GET fallback is kept last
  (esp_http_server matches in registration order). `/reset` is built in.
- The filesystems over HTTP, the same routes and answers on both backends (`ports/esp-idf/FS_Http`,
  Arduino `src/System/FS_Http`): `GET /fs/mounts`, `GET /fs/list`, `GET`/`PUT`/`DELETE /fs/file`,
  `POST /fs/mkdir`, `POST /fs/rename`, each with `fs=` (filesystem, `littlefs` when missing) and `path=`.
  An upload goes to `<path>.tmp` and replaces the file only when complete. Paths must be absolute, no `..`.
  HTTP and not WebSocket commands on purpose: a request gets its answer directly, simpler for the frontend.
  The filesystem selector, listing and the mounts come from the new portable `src/Support/FS_Json`
  (ESP32 Arduino, ESP8266, ESP-IDF), so every firmware gives the same JSON:
  `/fs/mounts` = `[{"fs":"littlefs","total":983040,"used":389120},{"fs":"sd",...}]` (only mounted ones),
  `/fs/list` = `[{"name":"cfg.json","dir":false,"size":2482}]`.
  `fs=sd` is the SD card (ESP32 with SDMMC on the Arduino core, when a card is mounted); SD on ESP-IDF is still to do.
- Arduino: `src/System/HttpServer` (`HttpServer::Get()`), the one `AsyncWebServer` on port 80, same idea as on ESP-IDF.
- Frontend with tabs (`tabs.js`, selected by the url hash `#console` / `#files`, the WebSocket is shared):
  - Console: the existing terminal, buttons and log.
  - Files (`files.js`, loaded when the tab is first shown, `css/files.css`), laid out as the old FSBrowser edit page:
    a fixed folder tree on the left with one root per mounted filesystem (flash, SD card), (expand/collapse, right click menu: upload here, new file/folder,
    rename/move, delete, download; drag and drop upload onto a folder), the ACE editor on the right
    (mode from the extension, Ctrl+S saves, JSON is checked before saving, `{ } format`), images are shown
    (small ones scaled up), other binary files as a hex dump. Filesystem usage in the header.
    ACE 1.36.2 is not in the firmware: it is loaded from cdnjs, else from LittleFS `/edit/ace.js` (as FSBrowser),
    else a plain textarea is used (failsafe AP without internet). `#files=/cfg.json` (or `#files=sd:/a.txt`)
    opens a file directly.
    Uses the `/fs/...` routes, so it is the same on the Arduino and the ESP-IDF firmware.
  - Unzip (`zip.js`, no library, ~2 kB gzip): a .zip from the computer (`⇪ unzip`, folder right click menu) or one
    on the device (click it: content preview, or right click `unzip into`) is unpacked in the browser into the
    current folder, folders are created, then the files are sent one by one with `PUT /fs/file`. One question when
    files already exist: overwrite them or keep them. `..`, `__MACOSX/` and `.DS_Store` are skipped.
    The zip table of content is parsed in JS, deflate is done by the browser (`DecompressionStream`), so it works
    offline too; stored and deflate entries, no ZIP64, no encryption. Tested against Python zipfile and Windows
    `Compress-Archive` zips, and on esp32dev (Arduino) through a real browser: byte identical files.
  - Download as zip (backup), the other way: `⬇ zip` in the toolbar (current folder) or `download as .zip` in the
    folder right click menu. The folder is read file by file and packed in the browser (`zip.js`: CRC32 in JS,
    deflate by `CompressionStream` when it makes a file smaller, UTF-8 names, empty folders kept); the root gives
    `dalhal-<fs>-<date>.zip`. Checked with Python `zipfile.testzip` and Windows `Expand-Archive`, and on esp32dev:
    a zip of the whole LittleFS made in the browser matched every file on the device.

- Config tab (`config.js`, `css/config.css`): a cfg.json editor in the "cfg::editor" layout of `html/cfgeditor/from claude ver3`
  (device tree left, form right, JSON panel, status bar), rewritten to follow the firmware exactly:
  - The schema comes from the running firmware (`hal/meta/reg/cfgschema`), the GPIO list from `hal/meta/gpio`,
    so the editor always matches the firmware it talks to; cfg is `/hal/cfg.json` via `/fs/...`.
  - Modes as in `ModeSelector::evaluate`: a conjunction with required must be there, one without must not,
    exactly one mode must match, an AllOfFieldsGroup counts only when complete. Choosing a mode adds/removes the fields.
  - Form per field type: GPIO select (only pins that can do the mode, boot strapping pins marked, users of a pin shown,
    a negative pin = disabled pin use as in the firmware), allowed values as select, one-of groups (e.g. refresh time
    unit + value), all-or-none groups, nested objects, child items (registry types and object arrays, e.g. 1-wire,
    TX433 units, HomeAssistant entities), REGO600 registers as toggle chips, uid path autocomplete, free JSON fields,
    unknown fields, comment items (strings) kept and editable. Optional fields can be removed (default used).
    Password/key fields are masked.
  - Live validation with the firmware rules (required, ranges with maxValue 0 = no limit, string lengths, hex bytes,
    uid max 8 and unique, GPIO capability and conflicts, constraints like minPulseLength < maxPulseLength, modes);
    errors under the fields, dots in the tree, a clickable problem list. Disabled devices give warnings only.
  - Add/duplicate/move/delete, import/export, `#config=uid:uid` links (the address follows the selection).
  - Save writes `/hal/cfg.json`. Apply writes `/hal/cfg.new.json`, lets the firmware validate and load it
    (`hal/config/reload/cfg.new.json`) and only then writes `/hal/cfg.json`; when the firmware refuses it, the
    running config stays and the new firmware log entries are shown. Files are written one root device per line
    (the firmware reserves about twice the file size in RAM to parse it).
  - Tested on esp32dev (Arduino) with the real 32 device config through a real browser: loads, validates (results
    match what the firmware reports), edits, mode changes, and a refused apply (device kept its config, showed why).
- `main.js`: `wsRequest(cmd, tag)` returns a Promise with the answer of a command (by its BlockStreamer tag);
  answers requested this way are not printed in the console log.
### Changed
- ESP-IDF: the WebSocket API (`/ws`) and the built in frontend moved from their own server on port 82 to the
  shared server on port 80. Files are looked up as: LittleFS `/api/...` override, built in frontend, any other
  LittleFS file (a folder gives its `index.htm`), else 404.
- ESP-IDF failsafe: no own server any more, it uses the same server (frontend, `/ws` in failsafe mode, `/fs/...`
  to fix a broken `cfg.json`, `/reset`) and mounts LittleFS for that.
- Arduino: everything on port 80 as well, no port to type any more. The WebSocket API (`/ws`, frontend fallback)
  is on the shared server instead of its own server on port 82; `main.h` has no own `webserver` any more.
  The file fallback is the same as on ESP-IDF (LittleFS `/api/...`, built in frontend, other LittleFS files, 404).
  Failsafe uses the same server.
- Arduino: the old FSBrowser (`FSBrowser.h`, `FSBrowserAsync.h`, moved to `attic/`) is gone, the Files tab replaces it.
  With it went its routes (`/edit`, `/list`, `/status`, `/upload`, `/mkdir`, `/sdcard_listfiles`) and the path
  prefixes `/LittleFS/...` and `/sdcard/...`: the filesystem is chosen with `fs=` instead.
  LittleFS is now mounted in `System::Setup()` and in failsafe (it was done by `FSBrowser::setup()`).
  `/` now shows the frontend (or LittleFS `/api/index.html`), an old LittleFS `/index.htm` is at `/index.htm`.
- The PC simulation keeps its own port (82 by default, `WebSocketAPI::setup(port)`), as port 80 is often taken on a PC.
- Frontend `main.js`: the WebSocket connects to the server the page came from (was always port 82).
- ESP-IDF build: the frontend is regenerated on every build (`extra_scripts = pre:../../tools/gen_frontend.py`
  in `targets/esp-idf/platformio.ini`, the generator finds the repo root from there), before it was only
  regenerated on a clean build. For plain `idf.py` builds cmake re-runs when a `www/` file changes.

### Tested on hardware (esp32dev, ESP-IDF firmware)
- USB flash, boot, WiFi STA with the credentials stored by the Arduino firmware, LittleFS with the existing files, NTP.
- GPIO list with the new board define: the tri-state and pin reservation errors are gone.
- WebSocket API on port 82: `system/info`, `system/heap`, "normal mode" greeting.
- OTA with espota (no password): image received, MD5 ok, booted from the other app partition.
- Failsafe: `system/EnterRecoveryMode` aborts, the next boot starts failsafe (WiFi, OTA, page on port 80,
  WebSocket with "failsafe/recovery mode is active", commands work), `/reset` returns to normal mode.
- Shared server on port 80 (OTA update): frontend, `/ws` commands, upload/download/delete with `/fs/file`,
  LittleFS files by path, 404, failsafe on port 80 including `/fs/file`, `/reset` back to normal mode.
- Not verified yet: HomeAssistant/MQTT and devices, because the test config uses device types that are not
  ported yet (1WTG, DHT, I2C, WS2812) and an invalid config is not loaded at all.

- Noticed, not changed: `DeviceManager::ParseJSON` clears all GPIO reservations before it validates, so a refused
  `hal/config/reload` leaves the running devices without their reservations (only the duplicate check of the next load is affected).
### Fixed
- **LittleFS lost on esp32dev** when going from the ESP-IDF firmware back to the Arduino firmware: esp_littlefs 1.23
  (ESP-IDF build) writes on-disk version 2.1 and upgrades the filesystem on the first write, the Arduino core 2.0.18
  is built with `CONFIG_LITTLEFS_DISK_VERSION_2_0` and can not mount 2.1, and `LittleFS.begin(formatOnFail = true)`
  then formatted it. The ESP-IDF build now uses the same on-disk version (`CONFIG_LITTLEFS_MULTIVERSION=y`,
  `CONFIG_LITTLEFS_DISK_VERSION_2_0=y`), so both firmwares can be flashed in turn without losing files.
- Arduino: `/` gave 404, the file fallback did not map `/` to the built in `index.html` (ESP-IDF did).
- ESP-IDF: the WebSocket of the frontend could not connect after the Files tab was used: lwIP had only
  10 sockets, httpd (7 connections + 2 internal) and OTA took them all, so accept() failed (ENFILE) before httpd
  could drop its oldest connection. Now `CONFIG_LWIP_MAX_SOCKETS=20` and httpd `max_open_sockets = 12`;
  tested with 14 keep-alive connections open: WebSocket, HTTP and OTA still work.
  (OTA needs a free socket too, so it also failed in that state.)
- Log entries on the serial port now end with a newline (they were printed on one long line, also on Arduino).

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
