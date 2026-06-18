

#include "WiFi.h"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")  // link Winsock library
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

wl_status_t WiFiClass::begin() {
    return wl_status_t::WL_CONNECTED;
}

bool WiFiClass::mode(wifi_mode_t m) {
    return true;
}

wl_status_t WiFiClass::status() {
    return wl_status_t::WL_CONNECTED;
}

int8_t WiFiClass::RSSI() {
    return 0;
}

bool WiFiClass::disconnect(bool) {
    return true;
}

String WiFiClass::SSID() {
    return "WindowsSimulator";
}

IPAddress WiFiClass::localIP() {
    return IPAddress();
}

IPAddress WiFiClass::softAPIP() {
    return IPAddress();
}

bool WiFiClass::softAP(const char* ssid, const char* passphrase/* = NULL*/, int channel/* = 1*/, int ssid_hidden/* = 0*/, int max_connection/* = 4*/, bool ftm_responder/* = false*/) {
    return true;
}

bool WiFiClass::hostByName(const char* host, IPAddress ip) {
#if defined(_WIN32)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        return false;
    }

    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET; // IPv4
    int err = getaddrinfo(host, nullptr, &hints, &res);
    if (err != 0 || res == nullptr) {
        WSACleanup();
        return false;
    }

    struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;
    uint32_t addr_n = ntohl(addr->sin_addr.s_addr);

    ip[0] = (addr_n >> 24) & 0xFF;
    ip[1] = (addr_n >> 16) & 0xFF;
    ip[2] = (addr_n >> 8) & 0xFF;
    ip[3] = addr_n & 0xFF;

    freeaddrinfo(res);
    WSACleanup();
    return true;
#else
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET; // IPv4 only
    int err = getaddrinfo(host, nullptr, &hints, &res);
    if (err != 0 || res == nullptr) {
        return false;
    }

    struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;
    uint32_t addr_n = ntohl(addr->sin_addr.s_addr);

    ip[0] = (addr_n >> 24) & 0xFF;
    ip[1] = (addr_n >> 16) & 0xFF;
    ip[2] = (addr_n >> 8) & 0xFF;
    ip[3] = addr_n & 0xFF;

    freeaddrinfo(res);
    return true;
#endif
}
WiFiClass WiFi;