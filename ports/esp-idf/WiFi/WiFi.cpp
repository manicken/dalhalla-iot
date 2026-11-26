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

#include "WiFi.h"

#include <atomic>
#include <cstring>
#include <cstdio>

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/netdb.h"

WiFiClass WiFi;

static esp_netif_t* staNetif = nullptr;
static esp_netif_t* apNetif = nullptr;

// written by the event handler (event task), read by the DALHAL loop
static std::atomic<int> staStatus{WL_DISCONNECTED};
static std::atomic<bool> staWanted{false};       // begin() was called and not disconnect()
static std::atomic<bool> staAutoReconnect{true};

static void OnWiFiEvent(void* arg, esp_event_base_t base, int32_t id, void* data) {
    (void)arg;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        const wifi_event_sta_disconnected_t* ev = static_cast<const wifi_event_sta_disconnected_t*>(data);
        int prev = staStatus.load();
        if (ev->reason == WIFI_REASON_NO_AP_FOUND) staStatus = WL_NO_SSID_AVAIL;
        else if (ev->reason == WIFI_REASON_AUTH_FAIL || ev->reason == WIFI_REASON_HANDSHAKE_TIMEOUT ||
                 ev->reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT) staStatus = WL_CONNECT_FAILED;
        else staStatus = (prev == WL_CONNECTED) ? WL_CONNECTION_LOST : WL_DISCONNECTED;
        // as arduino-esp32: reconnect when the connection was wanted and auto reconnect is on
        if (staWanted && staAutoReconnect) esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        staStatus = WL_CONNECTED; // as on Arduino, connected means "has an IP"
    } else if (base == IP_EVENT && id == IP_EVENT_STA_LOST_IP) {
        if (staStatus == WL_CONNECTED) staStatus = WL_CONNECTION_LOST;
    }
}

bool WiFiClass::init() {
    if (initialized) return true;

    // NVS holds the WiFi credentials, never erased here (that would drop the stored credentials)
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        printf("WiFi: nvs_flash_init failed: %s\r\n", esp_err_to_name(err));
        return false;
    }
    err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return false;
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return false;

    staNetif = esp_netif_create_default_wifi_sta();
    apNetif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK) return false;
    esp_wifi_set_storage(persistentStorage ? WIFI_STORAGE_FLASH : WIFI_STORAGE_RAM);

    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &OnWiFiEvent, nullptr);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &OnWiFiEvent, nullptr);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, &OnWiFiEvent, nullptr);

    if (!hostname.empty()) esp_netif_set_hostname(staNetif, hostname.c_str());

    initialized = true;
    return true;
}

bool WiFiClass::ensureStarted() {
    if (!init()) return false;
    if (started) return true;
    wifi_mode_t m = WIFI_MODE_NULL;
    esp_wifi_get_mode(&m);
    if (m == WIFI_MODE_NULL) return false;
    started = esp_wifi_start() == ESP_OK;
    return started;
}

bool WiFiClass::mode(wifi_mode_t m) {
    if (!init()) return false;
    if (m == WIFI_MODE_NULL) {
        if (started) { esp_wifi_stop(); started = false; }
        return esp_wifi_set_mode(WIFI_MODE_NULL) == ESP_OK;
    }
    if (esp_wifi_set_mode(m) != ESP_OK) return false;
    return ensureStarted();
}

wifi_mode_t WiFiClass::getMode() {
    wifi_mode_t m = WIFI_MODE_NULL;
    if (initialized) esp_wifi_get_mode(&m);
    return m;
}

bool WiFiClass::addMode(wifi_mode_t add) {
    wifi_mode_t m = getMode();
    if (m == add || m == WIFI_MODE_APSTA) return ensureStarted();
    if (m == WIFI_MODE_NULL) return mode(add);
    return mode(WIFI_MODE_APSTA);
}

void WiFiClass::persistent(bool enable) {
    persistentStorage = enable;
    if (initialized) esp_wifi_set_storage(enable ? WIFI_STORAGE_FLASH : WIFI_STORAGE_RAM);
}

wl_status_t WiFiClass::begin() {
    if (!addMode(WIFI_MODE_STA)) return WL_CONNECT_FAILED;
    staWanted = true;
    staAutoReconnect = autoReconnect;
    staStatus = WL_DISCONNECTED;
    esp_wifi_connect(); // uses the stored config
    return status();
}

wl_status_t WiFiClass::begin(const char* ssid, const char* passphrase) {
    if (ssid == nullptr || !addMode(WIFI_MODE_STA)) return WL_CONNECT_FAILED;
    wifi_config_t conf = {};
    strncpy(reinterpret_cast<char*>(conf.sta.ssid), ssid, sizeof(conf.sta.ssid));
    if (passphrase) strncpy(reinterpret_cast<char*>(conf.sta.password), passphrase, sizeof(conf.sta.password));
    conf.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    conf.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    esp_wifi_disconnect();
    if (esp_wifi_set_config(WIFI_IF_STA, &conf) != ESP_OK) return WL_CONNECT_FAILED;
    return begin();
}

bool WiFiClass::reconnect() {
    if (!initialized) return false;
    staWanted = true;
    esp_wifi_disconnect();
    return esp_wifi_connect() == ESP_OK;
}

bool WiFiClass::disconnect(bool wifioff, bool eraseap) {
    if (!initialized) return false;
    staWanted = false;
    esp_wifi_disconnect();
    if (eraseap) {
        wifi_config_t conf = {};
        esp_wifi_set_config(WIFI_IF_STA, &conf);
    }
    if (wifioff) mode(WIFI_MODE_NULL);
    staStatus = WL_DISCONNECTED;
    return true;
}

wl_status_t WiFiClass::status() {
    if (!initialized) return WL_DISCONNECTED;
    return static_cast<wl_status_t>(staStatus.load());
}

bool WiFiClass::setSleep(bool enable) {
    if (!init()) return false;
    return esp_wifi_set_ps(enable ? WIFI_PS_MIN_MODEM : WIFI_PS_NONE) == ESP_OK;
}

bool WiFiClass::setHostname(const char* name) {
    hostname = name ? name : "";
    if (staNetif) return esp_netif_set_hostname(staNetif, hostname.c_str()) == ESP_OK;
    return true; // applied in init()
}

static IPAddress NetifIP(esp_netif_t* netif) {
    esp_netif_ip_info_t info = {};
    if (netif == nullptr || esp_netif_get_ip_info(netif, &info) != ESP_OK) return IPAddress();
    return IPAddress(info.ip.addr);
}

IPAddress WiFiClass::localIP() {
    return NetifIP(staNetif);
}

IPAddress WiFiClass::softAPIP() {
    return NetifIP(apNetif);
}

String WiFiClass::SSID() {
    if (!initialized) return String();
    wifi_ap_record_t ap = {};
    if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) return String(reinterpret_cast<const char*>(ap.ssid));
    wifi_config_t conf = {};
    if (esp_wifi_get_config(WIFI_IF_STA, &conf) == ESP_OK) return String(reinterpret_cast<const char*>(conf.sta.ssid));
    return String();
}

int8_t WiFiClass::RSSI() {
    if (!initialized) return 0;
    wifi_ap_record_t ap = {};
    if (esp_wifi_sta_get_ap_info(&ap) != ESP_OK) return 0;
    return ap.rssi;
}

bool WiFiClass::softAP(const char* ssid, const char* passphrase, int channel, int ssidHidden, int maxConnection) {
    if (ssid == nullptr || !addMode(WIFI_MODE_AP)) return false;
    wifi_config_t conf = {};
    strncpy(reinterpret_cast<char*>(conf.ap.ssid), ssid, sizeof(conf.ap.ssid));
    conf.ap.ssid_len = (uint8_t)strnlen(ssid, sizeof(conf.ap.ssid));
    conf.ap.channel = (uint8_t)channel;
    conf.ap.ssid_hidden = (uint8_t)ssidHidden;
    conf.ap.max_connection = (uint8_t)maxConnection;
    if (passphrase != nullptr && strlen(passphrase) >= 8) {
        strncpy(reinterpret_cast<char*>(conf.ap.password), passphrase, sizeof(conf.ap.password));
        conf.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        conf.ap.authmode = WIFI_AUTH_OPEN; // as arduino-esp32, a too short passphrase gives an open AP
    }
    return esp_wifi_set_config(WIFI_IF_AP, &conf) == ESP_OK;
}

int WiFiClass::hostByName(const char* host, IPAddress& result) {
    if (host == nullptr) return 0;
    if (result.fromString(host)) return 1;
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    struct addrinfo* res = nullptr;
    if (getaddrinfo(host, nullptr, &hints, &res) != 0 || res == nullptr) return 0;
    const struct sockaddr_in* addr = reinterpret_cast<const struct sockaddr_in*>(res->ai_addr);
    result = IPAddress((uint32_t)addr->sin_addr.s_addr);
    freeaddrinfo(res);
    return 1;
}

int16_t WiFiClass::scanNetworks(bool async, bool showHidden) {
    (void)async; // always blocking here
    scanResults.clear();
    if (!addMode(WIFI_MODE_STA)) return WIFI_SCAN_FAILED;
    wifi_scan_config_t cfg = {};
    cfg.show_hidden = showHidden;
    if (esp_wifi_scan_start(&cfg, true) != ESP_OK) return WIFI_SCAN_FAILED;
    uint16_t count = 0;
    esp_wifi_scan_get_ap_num(&count);
    scanResults.resize(count);
    if (count > 0 && esp_wifi_scan_get_ap_records(&count, scanResults.data()) != ESP_OK) {
        scanResults.clear();
        return WIFI_SCAN_FAILED;
    }
    scanResults.resize(count);
    return (int16_t)count;
}

String WiFiClass::SSID(uint8_t index) {
    if (index >= scanResults.size()) return String();
    return String(reinterpret_cast<const char*>(scanResults[index].ssid));
}

int32_t WiFiClass::RSSI(uint8_t index) {
    return index < scanResults.size() ? scanResults[index].rssi : 0;
}

int32_t WiFiClass::channel(uint8_t index) {
    return index < scanResults.size() ? scanResults[index].primary : 0;
}

wifi_auth_mode_t WiFiClass::encryptionType(uint8_t index) {
    return index < scanResults.size() ? scanResults[index].authmode : WIFI_AUTH_OPEN;
}
