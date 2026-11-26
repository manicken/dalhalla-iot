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

#include "WiFiClient.h"

#include <cerrno>
#include <fcntl.h>
#include "lwip/sockets.h"
#include "lwip/netdb.h"

WiFiClient& WiFiClient::operator=(WiFiClient&& other) noexcept {
    if (this != &other) {
        stop();
        fd = other.fd;
        timeoutMs = other.timeoutMs;
        other.fd = -1;
    }
    return *this;
}

int WiFiClient::connect(IPAddress ip, uint16_t port) {
    stop();
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) return 0;

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = (uint32_t)ip;

    // non blocking connect with timeout
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    int res = ::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    if (res < 0 && errno != EINPROGRESS) { stop(); return 0; }
    if (res < 0) {
        fd_set wset;
        FD_ZERO(&wset);
        FD_SET(fd, &wset);
        struct timeval tv;
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        if (select(fd + 1, nullptr, &wset, nullptr, &tv) <= 0) { stop(); return 0; }
        int soErr = 0;
        socklen_t len = sizeof(soErr);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &soErr, &len);
        if (soErr != 0) { errno = soErr; stop(); return 0; }
    }
    // blocking again, writes are bounded by the send timeout
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    struct timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    int one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    return 1;
}

int WiFiClient::connect(const char* host, uint16_t port) {
    if (host == nullptr) return 0;
    IPAddress ip;
    if (!ip.fromString(host)) {
        struct addrinfo hints = {};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        struct addrinfo* res = nullptr;
        if (getaddrinfo(host, nullptr, &hints, &res) != 0 || res == nullptr) return 0;
        ip = IPAddress((uint32_t)reinterpret_cast<const struct sockaddr_in*>(res->ai_addr)->sin_addr.s_addr);
        freeaddrinfo(res);
    }
    return connect(ip, port);
}

size_t WiFiClient::write(uint8_t b) {
    return write(&b, 1);
}

size_t WiFiClient::write(const uint8_t* buf, size_t size) {
    if (fd < 0 || buf == nullptr) return 0;
    size_t sent = 0;
    while (sent < size) {
        int n = send(fd, buf + sent, size - sent, 0);
        if (n <= 0) {
            if (n < 0 && (errno == EINTR)) continue;
            if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) stop();
            break; // timeout or error
        }
        sent += (size_t)n;
    }
    return sent;
}

int WiFiClient::available() {
    if (fd < 0) return 0;
    int count = 0;
    if (ioctl(fd, FIONREAD, &count) < 0) return 0;
    return count;
}

int WiFiClient::read() {
    uint8_t b;
    return read(&b, 1) == 1 ? b : -1;
}

int WiFiClient::read(uint8_t* buf, size_t size) {
    if (fd < 0 || buf == nullptr || size == 0) return -1;
    int n = recv(fd, buf, size, MSG_DONTWAIT);
    if (n == 0) { stop(); return -1; } // closed by the peer
    if (n < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) stop();
        return -1;
    }
    return n;
}

int WiFiClient::peek() {
    if (fd < 0) return -1;
    uint8_t b;
    int n = recv(fd, &b, 1, MSG_PEEK | MSG_DONTWAIT);
    return n == 1 ? b : -1;
}

void WiFiClient::stop() {
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

uint8_t WiFiClient::connected() {
    if (fd < 0) return 0;
    uint8_t b;
    int n = recv(fd, &b, 1, MSG_PEEK | MSG_DONTWAIT);
    if (n == 0) { stop(); return 0; } // closed by the peer
    if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) { stop(); return 0; }
    return 1;
}
