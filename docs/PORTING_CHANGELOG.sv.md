# Ändringslogg för portningen: loss från Arduino-ramverket

[English](PORTING_CHANGELOG.md)

DALHAL flyttas bort från det hårda beroendet av Arduino-ramverket och av PlatformIO-forkar (pioarduino).
Målet är att DALHAL kör på ett tunt Arduino-liknande portlager, på samma sätt som PC-simuleringen redan gör,
ovanpå tillverkarnas SDK:er (ESP-IDF, senare Teensy 4.x, RP2040/RP2350, STM32).
Arduino-byggena (ESP32, ESP8266) fortsätter att fungera hela tiden.

**Läget:** pågående arbete. Alla riktiga enhetsbyggen i `platformio.ini` använder fortfarande `framework = arduino`.
Det enda bygget utan Arduino-ramverket är `targets/esp-idf`. Det har WiFi, webbgränssnittet, OTA, failsafe och HomeAssistant,
men ännu inga externa drivrutiner, ThingSpeak/REST, mDNS eller fillistning, och på hårdvara är hittills uppstart, WiFi, LittleFS, NTP, WebSocket-API, OTA och failsafe verifierade (se dess [README](../targets/esp-idf/README.md)). DALHAL-kärnan använder fortfarande Arduinos
*API* (`millis`, `digitalWrite`, `Serial`, `F()` …) med avsikt: på backends utan Arduino kommer det från de tunna portarna i `ports/`.
ESP8266 blir troligen kvar på Arduino-kärnan, backend-modellen klarar båda.

Lagren och reglerna beskrivs i [PLATFORM_CONTRACT.md](PLATFORM_CONTRACT.md) (på engelska).
Den här filen listar vad som gjorts, ett kapitel per dag, senaste först.

## Plan

| Steg | Vad | Status |
|---|---|---|
| 1 | PC-simuleringen bygger igen | klart |
| 2 | Portar i `ports/common` + `ports/pc` | klart |
| 3 | ESP-IDF-target utan Arduino (`targets/esp-idf`) | körs på esp32dev |
| 4 | Nätverk, webbserver/WebSocket, OTA, failsafe, HomeAssistant på ESP-IDF | WiFi, WebSocket, OTA, failsafe verifierade på esp32dev, HomeAssistant inte än; mDNS, SD-kort, W5500, HTTP-klient kvar |
| 5 | Drivrutiner en i taget (I2C, OneWire, DHT, WS2812, REGO600 …) | |

## Byggen

| Bygge | Kommando |
|---|---|
| ESP32 (Arduino) | `pio run -e esp32dev` |
| ESP8266 (Arduino) | `pio run -e esp12e` |
| PC-simulering | `pio run -e pc_sim`, för gdb `pio run -e pc_sim_debug`, eller CMake i `pc_simulation/` |
| ESP32 på ren ESP-IDF | `pio run -d targets/esp-idf` |

---

## 2026-11-28

### Tillagt
- Scripts-flik (`scripts.js`, `css/scripts.css`): en skripteditor.
  - Skriptfilerna i `/scripts` med en kryssruta "aktiv" som ändrar `list.txt` varsamt: dess kommentarer och ordning
    behålls, ett avstängt skript kommenteras ut (`//script2.txt`), att slå på det igen tar bort kommentaren.
  - ACE med färgmarkering för skriptspråket (on/then/do/endon, if/elseif/else/endif, and/or, eachloop, tal, hex,
    kommentarer, uid-sökvägar, `#namn`, `@Event`) och indrag efter then/do.
  - Autokomplettering från den körande enheten (cfg.json + `hal/meta/reg/functions` och `events`): uid-sökvägar
    (`temps:a:b`), `uid#` ger typens namngivna värden och funktioner (`neo#color`, `birel#set()`), `uid@` dess events,
    efter `on` triggers (`eachloop`, `i2c:pcf2@Interrupt` ...), mallar för on/endon och if/endif.
  - Kontroller medan man skriver (varningar): uid som inte finns i konfigurationen, okända `#namn` och `@events` (bara där
    registret beskriver typen; t.ex. enheterna i en 1-wire-grupp beskrivs inte, så de kontrolleras inte).
  - Spara (Ctrl+S) låter enheten kontrollera filen (`hal/scripts/validate`), dess fel med radnummer markeras i editorn
    och listas under den. "reload scripts" laddar de aktiva skripten (`hal/scripts/reload`).
- Firmware: `hal/scripts/validate/<fil>` (`ScriptEngine::ValidateScriptFile`): parsar bara en skriptfil i `/scripts`,
  felen hamnar i loggen; de laddade skripten och deras körläge påverkas inte.
- Firmware: `hal/scripts/check/<namn>/<skripttext>` (`ScriptEngine::ValidateScriptText`): kontrollerar ett skript som
  skickas med kommandot, i RAM, inget skrivs till flash (inget slitage, snabbare); felen har rad/kolumn i texten.
  Parsern är delad för det: `Parser::ParseScriptText(text, namn, callback)` gör jobbet, `ReadAndParseScriptFile`
  läser bara filen och anropar den (laddning av skript fungerar som förut).
  Scripts-fliken använder det vid sparande (aktiva skript) och för check-knappen, så editorns text kontrolleras som den är.
  Båda kontrollkommandona svarar success även när skriptet inte är giltigt (svaret säger det), eftersom ett misslyckat
  kommando skulle eka hela kommandot, här skriptet, som ett extra fel.
- Arduino WebSocket-API: ett meddelande som kommer i delar (större än ett TCP-paket, t.ex. ett skript till
  hal/scripts/check) sätts ihop per klient (max 16 kB, som ESP-IDF-versionen); förut kastades det med
  "WS EVT_DATA rx error". Testat med ett skript på 12 kB: kontrollerat, felrad 172 rapporterad rätt.
- `codeEditor.js`: ACE-/textrute-editorn delas av Files- och Scripts-flikarna (låg i files.js); `loadScript()` flyttad till
  `domHelpers.js`.

- Skript och konfiguration hålls i takt (firmwaren laddar om skripten efter varje konfigurationsladdning, och ett
  skript som använder en saknad enhet stoppar alla skript):
  - Scripts-fliken: ett aktivt skript är alltid giltigt. Att spara det vägras när det använder en enhet som saknas
    eller är avstängd i konfigurationen (kontrolleras direkt), eller när enheten inte godkänner det (texten
    kontrolleras först i RAM, se hal/scripts/check, filen skrivs bara när den är giltig). Ett skript aktiveras bara
    när det är giltigt. Inaktiva skript är utkast: sparas, med felen visade. Saknade/avstängda enheter är fel i editorn.
  - Config-fliken: läser skripten och kontrollerar dem mot konfigurationen medan man redigerar (`scriptTools.js`, delad
    med Scripts-fliken): "active script script1.txt line 3 uses "var1", which is disabled"; klick öppnar skriptet.
    Save och apply vägras så länge ett aktivt skript använder en enhet som konfigurationen inte skapar.
    Inaktiva skript ger varningar. Byta namn på en enhet som ett skript använder: gör skriptet inaktivt, ändra
    konfigurationen, rätta skriptet, aktivera det igen.

### Ändrat
- Config-fliken: enhetsträdet har hopfällbara noder (▸/▾, dubbelklick fäller också), hopfällda från start så bara
  rotenheterna syns; antal och problemprickar syns även på hopfällda noder. När något väljs någon annanstans ifrån
  (formulär, länk, problemlista, ny enhet) fälls vägen dit ut. Knappar för fäll ut allt / fäll ihop allt. Utfällda noder
  sparas i webbläsaren (per uid-sökväg), så en omladdning behåller trädet som det var.
- Frontend: alla element byggs med `newEl()` från `domHelpers.js` (laddas först) i stället för
  createElement/className/textContent/appendChild-kedjor: `newEl('div', { cls, text, title, onclick, styles }, ...barn)`,
  plus `downloadUrl()`. config.js tappade sin egen kopia (`h`), kommandoförslagen byggs som element
  (texter från enheten visas som text, inte längre som HTML). Storlek som inbäddat (gzip -9 per fil): 52016 -> 51924 byte
  (kodfilerna krymper med ca 660 byte, domHelpers.js lägger till 181); framför allt blir koden kortare och lättare att läsa.
  Kontrollerat i en riktig webbläsare på esp32dev: konsolens knappar/förslag/logg, WiFi-dialogen, filträd/meny/editor, config-fliken.
- Config-fliken: problemlistan visar fel före varningar.
- Config-fliken: cfg.json skrivs som en handskriven: en enhet är kompakt JSON på en rad, men dess arrayer med
  underenheter (items, units ...) expanderas, en per rad, indenterade efter djup, `]}` på egen rad.
  Övriga värden (objekt, arrayer med värden) förblir inline. Ca 6 % större än en rotenhet per rad.
- Frontend: `el(id)` (getElementById) ligger i `domHelpers.js` och används överallt; config.js använder inte längre `$`
  för det (det såg ut som jQuery, som inte används), files.js tappade sin egen kopia.
- `tools/gen_frontend.py` minifierar JS, CSS och HTML före gzip: kommentarer, indentering och avslutande mellanslag bort.
  Varje rad behåller sitt radnummer (en kommentar eller tom rad blir en tom rad), så ett fel på rad N i webbläsarens
  dev tools finns på rad N i källfilen; det kostar ca 300 byte efter gzip.
  En liten tokenizer kopierar strängar, template literals (nästlade `${}`) och regex-literaler exakt, så `//` i en URL
  tas inte för en kommentar. Inget nytt beroende. Inbäddad frontend: 51960 -> 41870 byte (-19 %). Kontrollerat: varje
  JS-fil har identiskt syntaxträd före/efter (TypeScripts parser) med varje nod på samma rad, och hela gränssnittet fungerar på esp32dev.
  `DALHAL_FRONTEND_MINIFY=0` behåller filerna som de är.

### Rättat
- `hal/scripts/reload` svarade `"info":"ok"` utan `{}` (inte giltig JSON), nu ett JSON-objekt.
- Upptäckt, inte ändrat: en misslyckad `hal/scripts/reload` lämnar skripten stoppade (`ScriptBlocks::running = false`
  före valideringen), Scripts-fliken säger det.

## 2026-11-27

### Tillagt
- ESP-IDF: en HTTP-server för allt, `ports/esp-idf/HttpServer` (esp_http_server på port 80).
  Den äger bara servern, modulerna lägger till sina routes med `HttpServer::On()`; GET-reserven hålls sist
  (esp_http_server matchar i registreringsordning). `/reset` är inbyggd.
- Filsystemen över HTTP, samma routes och svar på båda backends (`ports/esp-idf/FS_Http`,
  Arduino `src/System/FS_Http`): `GET /fs/mounts`, `GET /fs/list`, `GET`/`PUT`/`DELETE /fs/file`,
  `POST /fs/mkdir`, `POST /fs/rename`, alla med `fs=` (filsystem, `littlefs` om den saknas) och `path=`.
  En uppladdning skrivs till `<path>.tmp` och ersätter filen först när den är komplett. Sökvägar måste vara absoluta, utan `..`.
  HTTP och inte WebSocket-kommandon med avsikt: en request får sitt svar direkt, enklare för frontenden.
  Filsystemsväljaren, listningen och monteringarna kommer från nya portabla `src/Support/FS_Json`
  (ESP32 Arduino, ESP8266, ESP-IDF), så alla firmware ger samma JSON:
  `/fs/mounts` = `[{"fs":"littlefs","total":983040,"used":389120},{"fs":"sd",...}]` (bara monterade),
  `/fs/list` = `[{"name":"cfg.json","dir":false,"size":2482}]`.
  `fs=sd` är SD-kortet (ESP32 med SDMMC på Arduino-kärnan, när ett kort är monterat); SD på ESP-IDF återstår.
- Arduino: `src/System/HttpServer` (`HttpServer::Get()`), den enda `AsyncWebServer` på port 80, samma idé som på ESP-IDF.
- Frontend med flikar (`tabs.js`, väljs med url-hash `#console` / `#files`, WebSocket delas):
  - Console: den befintliga terminalen, knapparna och loggen.
  - Files (`files.js`, laddas första gången fliken visas, `css/files.css`), upplagd som gamla FSBrowsers edit-sida:
    ett fast mappträd till vänster med en rot per monterat filsystem (flash, SD-kort), (fäll ut/ihop, högerklicksmeny: ladda upp hit, ny fil/mapp, byt namn/flytta,
    radera, hämta; dra och släpp-uppladdning på en mapp), ACE-editorn till höger (läge efter filändelse,
    Ctrl+S sparar, JSON kontrolleras innan sparning, `{ } format`), bilder visas (små förstoras), andra binärfiler
    som hexdump. Filsystemets användning i huvudet.
    ACE 1.36.2 ligger inte i firmwaren: den laddas från cdnjs, annars från LittleFS `/edit/ace.js` (som FSBrowser),
    annars används en enkel textruta (failsafe-AP utan internet). `#files=/cfg.json` (eller `#files=sd:/a.txt`)
    öppnar en fil direkt.
    Använder `/fs/...`-routerna, så den är likadan på Arduino- och ESP-IDF-firmwaren.
  - Packa upp zip (`zip.js`, inget bibliotek, ~2 kB gzip): en .zip från datorn (`⇪ unzip`, högerklick på mapp) eller en
    på enheten (klicka på den: förhandsvisning av innehållet, eller högerklick `unzip into`) packas upp i webbläsaren till
    aktuell mapp, mappar skapas, sedan skickas filerna en och en med `PUT /fs/file`. En fråga när filer redan finns:
    skriv över eller behåll. `..`, `__MACOSX/` och `.DS_Store` hoppas över.
    Zip-filens innehållsförteckning läses i JS, deflate görs av webbläsaren (`DecompressionStream`), så det fungerar
    även offline; stored och deflate, inte ZIP64, inte kryptering. Testat mot zip från Python zipfile och Windows
    `Compress-Archive`, och på esp32dev (Arduino) i en riktig webbläsare: byte-identiska filer.
  - Hämta som zip (backup), åt andra hållet: `⬇ zip` i verktygsraden (aktuell mapp) eller `download as .zip` i mappens
    högerklicksmeny. Mappen läses fil för fil och packas i webbläsaren (`zip.js`: CRC32 i JS, deflate med
    `CompressionStream` när det gör filen mindre, UTF-8-namn, tomma mappar följer med); roten ger
    `dalhal-<fs>-<datum>.zip`. Kontrollerat med Python `zipfile.testzip` och Windows `Expand-Archive`, och på esp32dev:
    en zip av hela LittleFS gjord i webbläsaren stämde med varje fil på enheten.

- Config-flik (`config.js`, `css/config.css`): en cfg.json-editor i "cfg::editor"-utseendet från `html/cfgeditor/from claude ver3`
  (enhetsträd till vänster, formulär till höger, JSON-panel, statusrad), omskriven för att följa firmwaren exakt:
  - Schemat kommer från den körande firmwaren (`hal/meta/reg/cfgschema`), GPIO-listan från `hal/meta/gpio`,
    så editorn stämmer alltid med firmwaren den pratar med; cfg är `/hal/cfg.json` via `/fs/...`.
  - Lägen som i `ModeSelector::evaluate`: en conjunction med required måste finnas, en utan får inte finnas,
    exakt ett läge ska matcha, en AllOfFieldsGroup räknas bara när den är komplett. Val av läge lägger till/tar bort fälten.
  - Formulär per fälttyp: GPIO-lista (bara pinnar som klarar läget, strapping-pinnar markerade, vem som använder en pinne,
    negativ pinne = avstängd pinne som i firmwaren), tillåtna värden som lista, one-of-grupper (t.ex. enhet + värde för
    uppdateringstid), allt-eller-inget-grupper, nästlade objekt, underenheter (registertyper och objekt-arrayer, t.ex. 1-wire,
    TX433-enheter, HomeAssistant-entiteter), REGO600-register som chips, autokomplettering av uid-sökvägar, fria JSON-fält,
    okända fält, kommentarer (strängar) behålls och kan redigeras. Valfria fält kan tas bort (standardvärdet används).
    Lösenords-/nyckelfält maskeras.
  - Validering direkt med firmwarens regler (obligatoriskt, intervall där maxValue 0 = ingen gräns, stränglängder, hex-bytes,
    uid max 8 och unikt, GPIO-förmåga och konflikter, constraints som minPulseLength < maxPulseLength, lägen);
    fel under fälten, prickar i trädet, klickbar problemlista. Avstängda enheter ger bara varningar.
  - Lägg till/duplicera/flytta/radera, import/export, `#config=uid:uid`-länkar (adressen följer valet).
  - Save skriver `/hal/cfg.json`. Apply skriver `/hal/cfg.new.json`, låter firmwaren validera och ladda den
    (`hal/config/reload/cfg.new.json`) och skriver först då `/hal/cfg.json`; när firmwaren vägrar fortsätter den
    körande konfigurationen och firmwarens nya loggrader visas. Filer skrivs en rotenhet per rad
    (firmwaren reserverar ungefär dubbla filstorleken i RAM för att läsa den).
  - Testat på esp32dev (Arduino) med den riktiga konfigurationen på 32 enheter i en riktig webbläsare: laddar, validerar
    (stämmer med vad firmwaren rapporterar), redigering, lägesbyten och en underkänd apply (enheten behöll sin konfiguration
    och visade varför).
- `main.js`: `wsRequest(cmd, tag)` ger ett Promise med svaret på ett kommando (efter dess BlockStreamer-tagg);
  svar som hämtas så skrivs inte ut i konsolloggen.
### Ändrat
- ESP-IDF: WebSocket-API:t (`/ws`) och den inbyggda frontenden flyttade från en egen server på port 82 till den
  gemensamma servern på port 80. Filer letas upp så här: LittleFS `/api/...` (åsidosätter), inbyggd frontend, annan
  LittleFS-fil (en mapp ger sin `index.htm`), annars 404.
- ESP-IDF failsafe: ingen egen server längre, den använder samma server (frontend, `/ws` i failsafe-läge, `/fs/...`
  för att rätta en trasig `cfg.json`, `/reset`) och monterar LittleFS för det.
- Arduino: allt på port 80 även här, inget portnummer att skriva längre. WebSocket-API:t (`/ws`, reserv för frontenden)
  ligger på den gemensamma servern i stället för en egen server på port 82; `main.h` har ingen egen `webserver` längre.
  Filreserven är densamma som på ESP-IDF (LittleFS `/api/...`, inbyggd frontend, andra LittleFS-filer, 404).
  Failsafe använder samma server.
- Arduino: gamla FSBrowser (`FSBrowser.h`, `FSBrowserAsync.h`, flyttade till `attic/`) är borta, Files-fliken ersätter den.
  Med den försvann dess routes (`/edit`, `/list`, `/status`, `/upload`, `/mkdir`, `/sdcard_listfiles`) och
  sökvägsprefixen `/LittleFS/...` och `/sdcard/...`: filsystemet väljs med `fs=` i stället.
  LittleFS monteras nu i `System::Setup()` och i failsafe (det gjordes av `FSBrowser::setup()`).
  `/` visar nu frontenden (eller LittleFS `/api/index.html`), en gammal LittleFS `/index.htm` nås på `/index.htm`.
- PC-simuleringen behåller en egen port (82 som standard, `WebSocketAPI::setup(port)`), eftersom port 80 ofta är upptagen på en PC.
- Frontend `main.js`: WebSocket ansluter till servern som sidan kom ifrån (var alltid port 82).
- ESP-IDF-bygget: frontenden genereras vid varje bygge (`extra_scripts = pre:../../tools/gen_frontend.py`
  i `targets/esp-idf/platformio.ini`, generatorn hittar repo-roten därifrån), tidigare bara vid ett rent bygge.
  För vanliga `idf.py`-byggen körs cmake om när en `www/`-fil ändras.

### Testat på hårdvara (esp32dev, ESP-IDF-firmware)
- USB-flashning, uppstart, WiFi STA med uppgifterna som Arduino-firmwaren sparat, LittleFS med befintliga filer, NTP.
- GPIO-listan med det nya kortmakrot: felen vid tri-state och pinreservation är borta.
- WebSocket-API på port 82: `system/info`, `system/heap`, hälsningen "normal mode".
- OTA med espota (utan lösenord): image mottagen, MD5 ok, startade från den andra app-partitionen.
- Failsafe: `system/EnterRecoveryMode` gör abort, nästa uppstart går till failsafe (WiFi, OTA, sida på port 80,
  WebSocket med "failsafe/recovery mode is active", kommandon fungerar), `/reset` går tillbaka till normalt läge.
- Gemensam server på port 80 (uppdaterad via OTA): frontend, kommandon via `/ws`, uppladdning/hämtning/radering med
  `/fs/file`, LittleFS-filer via sökväg, 404, failsafe på port 80 inklusive `/fs/file`, `/reset` tillbaka till normalt läge.
- Inte verifierat än: HomeAssistant/MQTT och enheter, eftersom testkonfigurationen använder enhetstyper som inte
  är portade än (1WTG, DHT, I2C, WS2812) och en ogiltig konfiguration laddas inte alls.

- Upptäckt, inte ändrat: `DeviceManager::ParseJSON` rensar alla GPIO-reservationer innan den validerar, så en underkänd
  `hal/config/reload` lämnar de körande enheterna utan sina reservationer (påverkar bara dubblettkontrollen vid nästa laddning).
### Rättat
- **LittleFS förlorat på esp32dev** vid byte från ESP-IDF-firmwaren tillbaka till Arduino-firmwaren: esp_littlefs 1.23
  (ESP-IDF-bygget) skriver diskformat 2.1 och uppgraderar filsystemet vid första skrivningen, Arduino-kärnan 2.0.18
  är byggd med `CONFIG_LITTLEFS_DISK_VERSION_2_0` och kan inte montera 2.1, och `LittleFS.begin(formatOnFail = true)`
  formaterade det då. ESP-IDF-bygget använder nu samma diskformat (`CONFIG_LITTLEFS_MULTIVERSION=y`,
  `CONFIG_LITTLEFS_DISK_VERSION_2_0=y`), så båda firmware kan flashas om vartannat utan att filer försvinner.
- Arduino: `/` gav 404, filreserven mappade inte `/` till den inbyggda `index.html` (ESP-IDF gjorde det).
- ESP-IDF: frontendens WebSocket kunde inte ansluta efter att Files-fliken använts: lwIP hade bara
  10 sockets, httpd (7 anslutningar + 2 interna) och OTA tog alla, så accept() misslyckades (ENFILE) innan httpd
  hann stänga sin äldsta anslutning. Nu `CONFIG_LWIP_MAX_SOCKETS=20` och httpd `max_open_sockets = 12`;
  testat med 14 öppna keep-alive-anslutningar: WebSocket, HTTP och OTA fungerar fortfarande.
  (OTA behöver också en ledig socket, så även den misslyckades i det läget.)
- Loggposter på serieporten avslutas nu med radbrytning (de skrevs på en lång rad, även på Arduino).

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
