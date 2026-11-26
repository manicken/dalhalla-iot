#include "WiFiClient.h"
#include <chrono>

WiFiClient::WiFiClient() : sock(INVALID_SOCKET), isConnected(false) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        std::cerr << "WSAStartup failed.\n";
    }
}

WiFiClient::~WiFiClient() {
    stop();
    WSACleanup();
}

int WiFiClient::connect(const char* host, uint16_t port) {
    if (sock != INVALID_SOCKET) stop();

    struct addrinfo hints = {}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char portStr[6];
    sprintf(portStr, "%u", port);

    if (getaddrinfo(host, portStr, &hints, &res) != 0) return 0;

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == INVALID_SOCKET) {
        freeaddrinfo(res);
        return 0;
    }

    if (::connect(sock, res->ai_addr, (int)res->ai_addrlen) != 0) {
        closesocket(sock);
        sock = INVALID_SOCKET;
        freeaddrinfo(res);
        return 0;
    }

    freeaddrinfo(res);
    isConnected = true;
    return 1;
}

int WiFiClient::available() {
    if (sock == INVALID_SOCKET) return 0;
    u_long bytes = 0;
    ioctlsocket(sock, FIONREAD, &bytes);
    return static_cast<int>(bytes);
}

int WiFiClient::read() {
    uint8_t b;
    int r = recv(sock, reinterpret_cast<char*>(&b), 1, 0);
    if (r <= 0) return -1;
    return b;
}

int WiFiClient::read(uint8_t* buf, size_t size) {
    if (sock == INVALID_SOCKET) return -1;
    int r = recv(sock, reinterpret_cast<char*>(buf), static_cast<int>(size), 0);
    return (r <= 0) ? -1 : r;
}

size_t WiFiClient::write(uint8_t data) {
    if (sock == INVALID_SOCKET) return 0;
    int r = send(sock, reinterpret_cast<const char*>(&data), 1, 0);
    return (r <= 0) ? 0 : 1;
}

size_t WiFiClient::write(const uint8_t* buf, size_t size) {
    if (sock == INVALID_SOCKET) return 0;
    int r = send(sock, reinterpret_cast<const char*>(buf), static_cast<int>(size), 0);
    return (r <= 0) ? 0 : static_cast<size_t>(r);
}

void WiFiClient::stop() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    isConnected = false;
}

uint8_t WiFiClient::connected() {
    return isConnected && sock != INVALID_SOCKET;
}

WiFiClient::operator bool() {
    return connected();
}
