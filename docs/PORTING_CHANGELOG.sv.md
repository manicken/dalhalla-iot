# Ändringslogg för portningen: loss från Arduino-ramverket

[English](PORTING_CHANGELOG.md)

DALHAL flyttas bort från det hårda beroendet av Arduino-ramverket och av PlatformIO-forkar (pioarduino).
Målet är att DALHAL kör på ett tunt Arduino-liknande portlager, på samma sätt som PC-simuleringen redan gör,
ovanpå tillverkarnas SDK:er (ESP-IDF, senare Teensy 4.x, RP2040/RP2350, STM32).
Arduino-byggena (ESP32, ESP8266) fortsätter att fungera hela tiden.

**Läget:** pågående arbete. Alla riktiga enhetsbyggen i `platformio.ini` använder fortfarande `framework = arduino`.
Det enda bygget utan Arduino-ramverket är `targets/esp-idf`. Det har WiFi, webbgränssnittet, OTA, failsafe och HomeAssistant,
men ännu inga externa drivrutiner, ThingSpeak/REST, mDNS eller FSBrowser, och det är inte testat på hårdvara (se dess [README](../targets/esp-idf/README.md)). DALHAL-kärnan använder fortfarande Arduinos
*API* (`millis`, `digitalWrite`, `Serial`, `F()` …) med avsikt: på backends utan Arduino kommer det från de tunna portarna i `ports/`.
ESP8266 blir troligen kvar på Arduino-kärnan, backend-modellen klarar båda.

Lagren och reglerna beskrivs i [PLATFORM_CONTRACT.md](PLATFORM_CONTRACT.md) (på engelska).
Den här filen listar vad som gjorts, ett kapitel per dag, senaste först.

## Plan

| Steg | Vad | Status |
|---|---|---|
| 1 | PC-simuleringen bygger igen | klart |
| 2 | Portar i `ports/common` + `ports/pc` | klart |
| 3 | ESP-IDF-target utan Arduino (`targets/esp-idf`) | bygger och länkar, inte testat på hårdvara |
| 4 | Nätverk, webbserver/WebSocket, OTA, failsafe, HomeAssistant på ESP-IDF | bygger och länkar, inte testat på hårdvara; mDNS, FSBrowser, W5500, HTTP-klient kvar |
| 5 | Drivrutiner en i taget (I2C, OneWire, DHT, WS2812, REGO600 …) | |

## Byggen

| Bygge | Kommando |
|---|---|
| ESP32 (Arduino) | `pio run -e esp32dev` |
| ESP8266 (Arduino) | `pio run -e esp12e` |
| PC-simulering | `pio run -e pc_sim`, för gdb `pio run -e pc_sim_debug`, eller CMake i `pc_simulation/` |
| ESP32 på ren ESP-IDF | `pio run -d targets/esp-idf` |

---

## 2026-11-26

### Tillagt
- **Nätverk på ESP-IDF** (steg 4a): `ports/esp-idf/WiFi`, den del av Arduinos WiFi-API som DALHAL använder, på esp_wifi/esp_netif,
  så att `WiFiConnectionManager`, `WiFiLink`, konfigurationsportalens AP, failsafe-AP:n och `wifi/...`-kommandona byggs oförändrade.
  Inloggningsuppgifterna ligger i esp_wifi:s NVS-lagring, samma som arduino-esp32, så Arduino- och ESP-IDF-firmwaren delar dem.
  `ports/esp-idf/WiFiClient`: TCP-klient på lwIP-sockets (`Network::TcpClient`).
- **Webbserver och WebSocket på `esp_http_server`** (steg 4b): port 82, `/ws`, LittleFS `/api/...` före den inbyggda frontenden (gzip),
  kommandon via `CommandExecutor`-kön, `Broadcast` till alla klienter, säkert från vilken task som helst.
- **OTA på ESP-IDF** (steg 4c): `ports/esp-idf/ArduinoOTA` talar arduino-esp32:s espota-protokoll (port 3232, valfritt lösenord),
  firmware- och filsystemsavbilder, MD5 kontrolleras innan något aktiveras. `pio run -t upload` med en IP fungerar som förut.
- **Failsafe på ESP-IDF** (steg 4d): efter en krasch startas bara nätverkets failsafe, OTA, en HTTP-sida på port 80 (`/`, `/reset`) och
  WebSocket-API:t i failsafe-läge, omstart efter 10 minuter, som på Arduino-targetet. `system/EnterRecoveryMode` via WebSocket.
- **HomeAssistant på ESP-IDF**: MQTT via PubSubClient-forken på `Network::TcpClient`.
- `Platform::ResetWasCrash()`, `ResetReasonStr()`, `FlashSize()`, `BackendName()`, förmågemakrot `DALHAL_HAS_WIFI`.
- **Kommando-API över seriekonsolen på ESP-IDF** (steg 3c): `CommandExecutor` och `SerialAPI` byggs på ESP-IDF.
  `app_main` använder samma `DALHAL::begin()` / `DALHAL::loop()` (`src/DALHAL/DALHAL.cpp`) som Arduino-targetet,
  plus HeartbeatLed och Scheduler. Firmware: 507 KB flash.
- `system/info` på ESP-IDF returnerar chip, ramverk och heap.
- NTP på ESP-IDF via `esp_sntp`, samma tidszon och servrar som på Arduino (kräver nätverket, steg 4).
- `[env:pc_sim_debug]`: PC-simuleringen med `-O0 -ggdb` för gdb, med egen byggkatalog så att ett byte inte bygger om allt.

### Ändrat
- `IPAddress` i `ports/common` beter sig som Arduinos: konvertering till `uint32_t`, `fromString`, `==`, kan skrivas ut med `Serial.print`.
- WiFi-kommandona styrs av `DALHAL_HAS_WIFI` (Arduino-kärnan och ESP-IDF), inte av backend:en.
- `PubSubClient_ErrorStrings.h` (biblioteksforken) inkluderar `<WString.h>` istället för att gissa vad `__FlashStringHelper` är.
- **`SerialAPI` läser rader utan att blockera**: tecken samlas tills `\n` kommer, sedan körs kommandot.
  Tidigare kunde `Serial.readStringUntil` blockera huvudloopen i upp till en sekund.
  Ett kommando kräver nu radslut (PlatformIO-monitorn skickar CR LF vid Enter). Rader över 1024 tecken kastas.
- WiFi-kommandona (`wifi/scan`, `wifi/set_b64`, `wifi/set_json`) finns bara på Arduino-backend:en, som på PC.
- `DALHAL_WebSocketAPI.cpp` (ESPAsyncWebServer) byggs bara på Arduino-backend:en.

### Rättat
- Första körningen av ESP-IDF-firmwaren på hårdvara (esp32dev, flashad via USB): startar, WiFi STA, LittleFS med
  befintliga filer, NTP, WebSocket-API och `system/info` fungerar. Problem som hittades och rättades:
  - OTA avbröts med "Receive Failed": `WiFiClient::available()` använder `ioctl(FIONREAD)`, som lwIP bara
    stödjer med `CONFIG_LWIP_SO_RCVBUF=y` (nu i `sdkconfig.defaults`). Det påverkade även MQTT.
  - OTA raderade hela image-ytan i `esp_ota_begin` innan mottagning, nu `OTA_WITH_SEQUENTIAL_WRITES`
    (raderar per sektor under skrivning, som Arduino `Update`).
  - Inget kortmakro på ESP-IDF, så GPIO-listan blev fel ("GPIO_PIN mask error", "Pin to reserve - not found").
    Komponenten definierar nu `DALHAL_BOARD` utifrån `IDF_TARGET` (esp32 = `ESP32DEV`), går att ändra.
- `system/info` svarar igen (den returnerade en tom sträng sedan gamla `Info::getESP_info()` kommenterades bort):
  nu `Info::getInfo(sbs)` (var `getESP_info()`), strömmas med `BlockStreamer` som de andra kommandona, likadant på alla plattformar via `Platform::`
  (chip, ramverk, byggversion, drifttid, omstartsorsak, heap, flashstorlek, LittleFS totalt/använt).
- `DALHAL_HA_DeviceDiscovery`: `%.*s` fick en `uint32_t` som precision, den måste vara `int` (fel på 32-bitars ARM/RISC-V).
- `DeviceUID`: `%X` för `uint32_t`, nu `PRIX32`. `esp_read_mac` kräver `<esp_mac.h>` sedan ESP-IDF 5.
- Ytterligare två temporära `JsonVariant` bundna till referenser (HomeAssistant-schemat).

### Bygg och verktyg
- **pioarduino borttaget ur PlatformIO-installationen.** I penv låg både paketet `platformio` och `pioarduino`
  installerade i samma modulmapp. pioarduinos IDE-generator tillsammans med originalets `.gitignore`-mall
  skrev över projektets `.gitignore` varje gång VS Code genererade om IntelliSense
  (det gjorde en gång att `secrets/` hamnade i staging, inget committades). Nu finns bara originalets `platformio` 6.2.0,
  som aldrig skriver över en befintlig `.gitignore`. Installera inte pioarduino igen, ESP-IDF kommer från den officiella
  `platformio/platform-espressif32`.

---

## 2026-11-25

### Tillagt
- **Plattformskontrakt** ([PLATFORM_CONTRACT.md](PLATFORM_CONTRACT.md)): lager A (ren mjukvara) till E (plattformsspecifikt),
  vad varje backend måste erbjuda, mappstruktur för portar och plattformsmakron.
- **Plattformslager (lager E)** i `src/System/Platform/`:
  - `Platform.h`: `DALHAL_PLATFORM_*` (chip) och `DALHAL_BACKEND_ARDUINO / _ESPIDF / _PC` (vad som levererar Arduino-delmängden),
    `Restart()`, `Abort()`, `FreeHeap()`, `LargestFreeBlock()`, `ChipName()`.
  - `Pwm`: PWM-utgång för arduino-esp32 2.x och 3.x, ESP-IDF (LEDC-drivern), ESP8266 och PC.
  - `FastGpio`: snabb GPIO med flankavbrott och ISR-säkra skrivningar (var direkta ESP-IDF-anrop `gpio_*` i enheterna).
- **Mappstruktur för portar** (steg 2):
  - `ports/common`: plattformsoberoende Arduino-delmängd (`String`, `Print`, `IPAddress`, `pgmspace.h`, `ArduinoCommon.h`, `WProgram.h`).
  - `ports/pc`: PC-implementationer (tid, sockets, filer, `Serial`, `DALHAL_WebSocketAPI_PC`).
  - `ports/pc/stubs`: låtsasdrivrutiner för simuleringen.
  - `ports/esp-idf`: `Arduino.h` (esp_timer, FreeRTOS, gpio-drivern, `Serial` på konsolen), `LittleFS` (på esp_littlefs,
    samma partition och monteringspunkt som arduino-esp32), `Ticker` (esp_timer), platshållare för WebSocket-API:t.
  - `attic/`: filer som ingenting längre refererar till, sparade för genomgång.
- **ESP-IDF-target** `targets/esp-idf` (steg 3a/3b): eget litet PlatformIO-projekt med `framework = espidf`
  (officiella espressif32-plattformen, ESP-IDF 5.5), går även att bygga med `idf.py`. Kärnan, JSON-konfigurationen,
  skriptmotorn och de enkla enheterna bygger och länkar. Enheter utan portade drivrutiner eller nätverk är bortvalda
  ur registret på ESP-IDF (`DALHAL_REGISTRY_ARDUINO_ONLY_DEVICES`).
- `[env:pc_sim]` i `platformio.ini` fungerar (var trasig) och ger IntelliSense för PC-koden. `tools/pio_native.py` lägger till
  källkoden utanför `src/`.
- `DALHAL_WebSocketAPI_Backend.h`: en header som väljer WebSocket-API:t för aktuell backend.

### Ändrat
- **Arduinos `String` borttagen ur DALHAL-kärnan**:
  - `ZeroCopyString` kan peka direkt på flash-strängar (`F("...")`). På ESP8266 kopierar mottagarna direkt från flash,
    på alla andra plattformar kostar det ingenting.
  - ~190 `String(F(...)).c_str()` ersatta med `F(...)` eller `ToStdString(F(...))`.
  - Egna hjälpfunktioner (`b64urlEncode`, `convertISO88591toUTF8`, `getSSID` …) använder `std::string`.
  - 40 headers hade `#include <Arduino.h> // Needed for String class`, borttagen eller flyttad till .cpp-filen med det som faktiskt används.
  - `String` finns bara kvar i Arduino-specifik plattformskod (webbserver, WiFi, HTTP-klient, OTA, FSBrowser).
- Villkor som betydde "det finns en separat flash-strängstyp" testar `DALHAL_FLASH_STRING_IS_CHAR` istället för "inte PC".
- PC-porten bytte namn `DALHAL_WebSocketAPI_Windows` → `DALHAL_WebSocketAPI_PC`.
- `PWM_Servo`, `Actuator`, `LatchingRelay` använder `Platform::Pwm` / `Platform::FastGpio` istället för ESP-IDF- och ledc-anrop.
- `Info::getESPVariant()` (returnerade alltid "") ersatt med `Platform::ChipName()`.
- PC-simuleringen serverar den riktiga frontenden från `FrontendFiles/www` (gzip), så webbgränssnittet kan utvecklas utan att flasha.

### Rättat
- Actuator: körning mot max satte fel riktning. LatchingRelay: körning mot set hade fel datanivå.
- Actuator / LatchingRelay: ändlägesavbrottet kunde stänga av fel avbrott.
- `GPIO_Manager`: en `std::string` skapades från `nullptr` (odefinierat beteende på alla plattformar).
- `DALHAL_Value`: `%d` / `%u` för `int32_t` / `uint32_t`, som är `long` på Xtensa, RISC-V och ARM, nu `PRId32` / `PRIu32`.
- REGO600: ett felmeddelande skrev på PC 16 kopior av ett tecken istället för ett hextal.
- Sex ställen band ett temporärt `JsonVariant` till en referens, nu som värde.
- PC-simuleringens bygge lagat (includes, `String`-klass, omdöpta funktioner i skriptmotorn).

### Bygg och verktyg
- `output.map` (länkarens map-fil) ligger i `.gitignore`.
- PC-bygget med CMake behöver de lokala patcharna av `Time` i `.pio/libdeps/esp32dev`; `pc_sim` i PlatformIO använder
  originalbiblioteket med `-D __time_t_defined` och `WProgram.h` istället.

---

## 2026-11-24

### Tillagt
- **Nätverksabstraktion** `src/System/Network`: `Network::IsUp()`, `LocalIP()`, `HostByName()`, `Network::TcpClient`,
  upp/ned-händelser, failsafe-start. Länkar: `WiFiLink` (omsluter `WiFiConnectionManager`, konfigurationsportal bara med WiFi)
  och `EthW5500Link` (W5500 i MAC raw-läge under lwIP, statisk reserv-IP från build flags, inte kompilerad än).
  HomeAssistant, ThingSpeak, REST och OTA använder den istället för WiFi direkt.

### Rättat
- HomeAssistant: prenumererar på kommandotopics igen efter en MQTT-återanslutning (switchar slutade reagera).
