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

#include "ArduinoOTA.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <strings.h>
#include <fcntl.h>

#include <Arduino.h>
#include <LittleFS.h>
#include <WiFiClient.h>

#include "lwip/sockets.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_random.h"
#include "esp_rom_md5.h"
#include "esp_system.h"

ArduinoOTAClass ArduinoOTA;

// ---------------- MD5 helpers ----------------

class Md5 {
public:
    Md5() { esp_rom_md5_init(&ctx); }
    void add(const void* data, size_t len) { esp_rom_md5_update(&ctx, static_cast<const uint8_t*>(data), len); }
    void add(const std::string& s) { add(s.data(), s.size()); }
    std::string hex() {
        uint8_t digest[16];
        esp_rom_md5_final(digest, &ctx);
        char out[33];
        for (int i = 0; i < 16; i++) snprintf(out + i * 2, 3, "%02x", digest[i]);
        return std::string(out, 32);
    }
private:
    md5_context_t ctx;
};

static std::string Md5Hex(const std::string& s) {
    Md5 m;
    m.add(s);
    return m.hex();
}

// ---------------- ArduinoOTAClass ----------------

void ArduinoOTAClass::setPassword(const char* password) {
    passwordMd5 = (password && *password) ? Md5Hex(password) : "";
}

void ArduinoOTAClass::begin() {
    if (udpFd >= 0) return;
    udpFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpFd < 0) { printf("OTA: no UDP socket\r\n"); return; }
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(otaPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(udpFd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        printf("OTA: bind to port %u failed\r\n", otaPort);
        close(udpFd);
        udpFd = -1;
        return;
    }
    fcntl(udpFd, F_SETFL, fcntl(udpFd, F_GETFL, 0) | O_NONBLOCK);
    state = State::Idle;
    printf("OTA: ready on port %u\r\n", otaPort);
}

void ArduinoOTAClass::end() {
    if (udpFd >= 0) { close(udpFd); udpFd = -1; }
    state = State::Idle;
}

void ArduinoOTAClass::udpReply(uint32_t ip, uint16_t port, const char* text) {
    struct sockaddr_in to = {};
    to.sin_family = AF_INET;
    to.sin_port = port; // already network order
    to.sin_addr.s_addr = ip;
    sendto(udpFd, text, strlen(text), 0, reinterpret_cast<struct sockaddr*>(&to), sizeof(to));
}

void ArduinoOTAClass::error(ota_error_t e, const char* text) {
    printf("OTA Error[%d]: %s\r\n", (int)e, text);
    if (errorCb) errorCb(e);
    state = State::Idle;
}

void ArduinoOTAClass::handle() {
    if (udpFd < 0) return;
    char buf[128];
    struct sockaddr_in from = {};
    socklen_t fromLen = sizeof(from);
    int n = recvfrom(udpFd, buf, sizeof(buf) - 1, 0, reinterpret_cast<struct sockaddr*>(&from), &fromLen);
    if (n <= 0) return;
    buf[n] = '\0';

    if (state == State::Idle) {
        // invitation: "<cmd> <host tcp port> <size> <md5>\n"
        int c = 0; unsigned port = 0; unsigned long size = 0; char md5[33] = {};
        if (sscanf(buf, "%d %u %lu %32s", &c, &port, &size, md5) != 4 || (c != U_FLASH && c != U_SPIFFS)) return;
        cmd = c;
        hostTcpPort = (uint16_t)port;
        imageSize = (uint32_t)size;
        imageMd5 = md5;
        if (passwordMd5.empty()) {
            udpReply(from.sin_addr.s_addr, from.sin_port, "OK");
            runUpdate(from.sin_addr.s_addr);
        } else {
            nonce = Md5Hex(std::to_string(esp_random()) + std::to_string(micros()));
            std::string msg = "AUTH " + nonce;
            udpReply(from.sin_addr.s_addr, from.sin_port, msg.c_str());
            state = State::WaitAuth;
        }
    } else if (state == State::WaitAuth) {
        // "200 <cnonce> <response>\n", response = md5(md5(password) ":" nonce ":" cnonce)
        int c = 0; char cnonce[33] = {}; char response[33] = {};
        if (sscanf(buf, "%d %32s %32s", &c, cnonce, response) != 3 || c != U_AUTH) return;
        std::string expected = Md5Hex(passwordMd5 + ":" + nonce + ":" + cnonce);
        if (strcasecmp(expected.c_str(), response) != 0) {
            udpReply(from.sin_addr.s_addr, from.sin_port, "Authentication Failed");
            error(OTA_AUTH_ERROR, "Auth Failed");
            return;
        }
        udpReply(from.sin_addr.s_addr, from.sin_port, "OK");
        runUpdate(from.sin_addr.s_addr);
    }
}

void ArduinoOTAClass::runUpdate(uint32_t hostIp) {
    state = State::Idle;
    if (startCb) startCb();
    printf("OTA Start: %s, %lu bytes\r\n", cmd == U_FLASH ? "firmware" : "filesystem", (unsigned long)imageSize);

    // target
    const esp_partition_t* part = nullptr;
    esp_ota_handle_t ota = 0;
    if (cmd == U_FLASH) {
        part = esp_ota_get_next_update_partition(nullptr);
        if (part == nullptr || imageSize > part->size || esp_ota_begin(part, OTA_WITH_SEQUENTIAL_WRITES, &ota) != ESP_OK) { // erase per sector while writing, as Arduino Update
            error(OTA_BEGIN_ERROR, "Begin Failed (no OTA partition or image too large)");
            return;
        }
    } else {
        part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "spiffs");
        if (part == nullptr || imageSize > part->size) {
            error(OTA_BEGIN_ERROR, "Begin Failed (no spiffs partition or image too large)");
            return;
        }
        LittleFS.end(); // the image replaces the whole filesystem
        uint32_t eraseSize = (imageSize + 4095) & ~4095u;
        if (esp_partition_erase_range(part, 0, eraseSize) != ESP_OK) {
            error(OTA_BEGIN_ERROR, "Begin Failed (erase)");
            return;
        }
    }

    // the host waits for us to connect back
    WiFiClient client;
    int tries = 0;
    while (!client.connect(IPAddress(hostIp), hostTcpPort)) {
        if (++tries > 10) {
            if (cmd == U_FLASH) esp_ota_abort(ota);
            error(OTA_CONNECT_ERROR, "Connect Failed");
            return;
        }
        delay(100);
    }

    Md5 md5;
    uint8_t buf[1460];
    uint32_t received = 0;
    unsigned long lastData = millis();
    while (received < imageSize) {
        int avail = client.available();
        if (avail <= 0) {
            if (!client.connected() || millis() - lastData > 10000) {
                if (cmd == U_FLASH) esp_ota_abort(ota);
                error(OTA_RECEIVE_ERROR, "Receive Failed");
                return;
            }
            delay(1);
            continue;
        }
        size_t want = imageSize - received;
        if (want > sizeof(buf)) want = sizeof(buf);
        if ((size_t)avail < want) want = (size_t)avail;
        int r = client.read(buf, want);
        if (r <= 0) continue;

        esp_err_t err = (cmd == U_FLASH) ? esp_ota_write(ota, buf, (size_t)r)
                                         : esp_partition_write(part, received, buf, (size_t)r);
        if (err != ESP_OK) {
            if (cmd == U_FLASH) esp_ota_abort(ota);
            error(OTA_RECEIVE_ERROR, "Write Failed");
            return;
        }
        md5.add(buf, (size_t)r);
        received += (uint32_t)r;
        lastData = millis();
        client.print(std::to_string(r).c_str()); // ack with the number of bytes written, as arduino-esp32
        if (progressCb) progressCb(received, imageSize);
    }

    // verify before anything is activated
    std::string got = md5.hex();
    if (strcasecmp(got.c_str(), imageMd5.c_str()) != 0) {
        if (cmd == U_FLASH) esp_ota_abort(ota);
        client.print("MD5 Check Failed");
        error(OTA_END_ERROR, "End Failed (MD5 mismatch)");
        return;
    }
    if (cmd == U_FLASH) {
        if (esp_ota_end(ota) != ESP_OK || esp_ota_set_boot_partition(part) != ESP_OK) {
            client.print("Image Validation Failed");
            error(OTA_END_ERROR, "End Failed (image validation)");
            return;
        }
    }

    client.print("OK");
    client.stop();
    printf("OTA End: OK\r\n");
    if (endCb) endCb();
    if (rebootOnSuccess) {
        delay(100);
        esp_restart();
    }
}
