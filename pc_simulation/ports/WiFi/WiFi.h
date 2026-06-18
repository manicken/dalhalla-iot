#pragma once

#include <string>
#include <cstdint>
#include <cstring>
#include "../IPAddress/IPAddress.h"

#include <WString.h>

typedef enum {
    WIFI_MODE_NULL = 0,  /**< null mode */
    WIFI_MODE_STA,       /**< WiFi station mode */
    WIFI_MODE_AP,        /**< WiFi soft-AP mode */
    WIFI_MODE_APSTA,     /**< WiFi station + soft-AP mode */
    WIFI_MODE_MAX
} wifi_mode_t;

typedef enum {
    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
} wl_status_t;

#define WIFI_OFF     WIFI_MODE_NULL
#define WIFI_STA     WIFI_MODE_STA
#define WIFI_AP      WIFI_MODE_AP
#define WIFI_AP_STA  WIFI_MODE_APSTA

class WiFiClass {
public:
    // Resolves hostname to uint8_t[4] IPv4 address
    bool hostByName(const char* host, IPAddress ip);
    wl_status_t begin();
    bool disconnect(bool);
    bool mode(wifi_mode_t m);
    wl_status_t status();
    String SSID();
    IPAddress localIP();
    IPAddress softAPIP();
    int8_t RSSI();
    bool softAP(const char* ssid, const char* passphrase = NULL, int channel = 1, int ssid_hidden = 0, int max_connection = 4, bool ftm_responder = false);
};

// Provide a global instance like ESP32/ESP8266 does
extern WiFiClass WiFi;
