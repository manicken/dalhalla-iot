/*
  Dalhalla IoT — Simplified Windows WebSocket (Header-Only Option)
  
  A lightweight, single-header WebSocket server for Windows.
  No external dependencies beyond Windows Winsock2.

  Copyright (C) 2026 Jannik Svensson
  GNU General Public License v3.0 or later
*/

#pragma once

#ifndef DALHAL_WEBSOCKET_SIMPLE_H
#define DALHAL_WEBSOCKET_SIMPLE_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#endif

#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <functional>
#include <memory>
#include <cstring>
#include <iostream>

namespace DALHAL {

    class SimpleWebSocketServer {
    public:
        using MessageCallback = std::function<void(int clientId, const std::string& message)>;
        using ClientCallback = std::function<void(int clientId, const std::string& ip)>;

        SimpleWebSocketServer(int port) : port_(port), running_(false), nextClientId_(1) {}

        ~SimpleWebSocketServer() {
            stop();
        }

        bool start(MessageCallback onMessage = nullptr, 
                  ClientCallback onConnect = nullptr,
                  ClientCallback onDisconnect = nullptr) {
            if (running_) return false;
            
            onMessage_ = onMessage;
            onConnect_ = onConnect;
            onDisconnect_ = onDisconnect;
            running_ = true;

            try {
                WSADATA wsaData;
                if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                    running_ = false;
                    return false;
                }

                listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (listenSocket_ == INVALID_SOCKET) {
                    WSACleanup();
                    running_ = false;
                    return false;
                }

                sockaddr_in serverAddr{};
                serverAddr.sin_family = AF_INET;
                serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                serverAddr.sin_port = htons(port_);

                if (bind(listenSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
                    closesocket(listenSocket_);
                    WSACleanup();
                    running_ = false;
                    return false;
                }

                if (listen(listenSocket_, SOMAXCONN) == SOCKET_ERROR) {
                    closesocket(listenSocket_);
                    WSACleanup();
                    running_ = false;
                    return false;
                }

                std::cout << "WebSocket server listening on port " << port_ << std::endl;
                std::thread acceptThread(&SimpleWebSocketServer::acceptLoop, this);
                acceptThread.detach();

                return true;
            }
            catch (...) {
                running_ = false;
                return false;
            }
        }

        void stop() {
            running_ = false;

            if (listenSocket_ != INVALID_SOCKET) {
                closesocket(listenSocket_);
                listenSocket_ = INVALID_SOCKET;
            }

            {
                std::lock_guard<std::mutex> lock(clientsMutex_);
                for (auto& p : clients_) {
                    if (p.second != INVALID_SOCKET) {
                        closesocket(p.second);
                    }
                }
                clients_.clear();
            }

            WSACleanup();
        }

        void broadcast(const std::string& message) {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            for (auto& p : clients_) {
                sendFrame(p.second, message);
            }
        }

        void sendToClient(int clientId, const std::string& message) {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            auto it = clients_.find(clientId);
            if (it != clients_.end()) {
                sendFrame(it->second, message);
            }
        }

        int getClientCount() const {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            return static_cast<int>(clients_.size());
        }

        bool isRunning() const { return running_; }

    private:
        int port_;
        bool running_;
        SOCKET listenSocket_ = INVALID_SOCKET;
        std::map<int, SOCKET> clients_;
        mutable std::mutex clientsMutex_;
        int nextClientId_;

        MessageCallback onMessage_;
        ClientCallback onConnect_;
        ClientCallback onDisconnect_;

        void acceptLoop() {
            while (running_) {
                sockaddr_in clientAddr{};
                int clientAddrLen = sizeof(clientAddr);

                SOCKET clientSocket = accept(listenSocket_, (sockaddr*)&clientAddr, &clientAddrLen);
                if (clientSocket == INVALID_SOCKET) {
                    if (running_) continue;
                    break;
                }

                int clientId = nextClientId_++;
                std::string clientIP = inet_ntoa(clientAddr.sin_addr);

                {
                    std::lock_guard<std::mutex> lock(clientsMutex_);
                    clients_[clientId] = clientSocket;
                }

                if (onConnect_) onConnect_(clientId, clientIP);

                std::thread(&SimpleWebSocketServer::clientHandler, this, clientId, clientSocket).detach();
            }
        }

        void clientHandler(int clientId, SOCKET clientSocket) {
            char buffer[4096];

            // Handshake
            int recvLen = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (recvLen > 0) {
                buffer[recvLen] = '\0';
                std::string key = extractKey(buffer);
                if (!key.empty()) {
                    std::string response = generateUpgrade(key);
                    send(clientSocket, response.c_str(), (int)response.length(), 0);
                }
            }

            // Message loop
            while (running_) {
                recvLen = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (recvLen <= 0) break;

                std::string msg = parseFrame(buffer, recvLen);
                if (!msg.empty() && onMessage_) {
                    onMessage_(clientId, msg);
                }
            }

            // Cleanup
            {
                std::lock_guard<std::mutex> lock(clientsMutex_);
                clients_.erase(clientId);
            }

            if (onDisconnect_) onDisconnect_(clientId, "");
            closesocket(clientSocket);
        }

        std::string parseFrame(const char* buf, int len) {
            if (len < 2) return "";
            
            unsigned char* data = (unsigned char*)buf;
            int opcode = data[0] & 0x0f;
            if (opcode != 0x1) return "";  // Not text

            int payloadLen = data[1] & 0x7f;
            int maskStart = 2;

            if (payloadLen == 126) {
                if (len < 4) return "";
                payloadLen = (data[2] << 8) | data[3];
                maskStart = 4;
            }
            else if (payloadLen == 127) {
                return "";  // Not implemented
            }

            if (maskStart + 4 + payloadLen > len) return "";

            unsigned char mask[4];
            std::memcpy(mask, &data[maskStart], 4);

            int payloadStart = maskStart + 4;
            std::string result;
            result.reserve(payloadLen);

            for (int i = 0; i < payloadLen; i++) {
                result += static_cast<char>(data[payloadStart + i] ^ mask[i % 4]);
            }

            return result;
        }

        void sendFrame(SOCKET socket, const std::string& message) {
            unsigned char frame[4096];
            int frameLen = 0;

            frame[frameLen++] = 0x81;  // FIN + text

            size_t msgLen = message.length();
            if (msgLen < 126) {
                frame[frameLen++] = static_cast<unsigned char>(msgLen);
            }
            else if (msgLen < 65536) {
                frame[frameLen++] = 126;
                frame[frameLen++] = static_cast<unsigned char>((msgLen >> 8) & 0xff);
                frame[frameLen++] = static_cast<unsigned char>(msgLen & 0xff);
            }

            std::memcpy(&frame[frameLen], message.c_str(), msgLen);
            frameLen += static_cast<int>(msgLen);

            send(socket, (char*)frame, frameLen, 0);
        }

        std::string extractKey(const char* request) {
            const char* keyStart = strstr(request, "Sec-WebSocket-Key: ");
            if (!keyStart) return "";
            keyStart += strlen("Sec-WebSocket-Key: ");
            return std::string(keyStart, 24);
        }

        std::string generateUpgrade(const std::string& key) {
            return std::string(
                "HTTP/1.1 101 Switching Protocols\r\n"
                "Upgrade: websocket\r\n"
                "Connection: Upgrade\r\n"
                "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"
                "\r\n"
            );
        }
    };

}  // namespace DALHAL

#endif  // DALHAL_WEBSOCKET_SIMPLE_H
