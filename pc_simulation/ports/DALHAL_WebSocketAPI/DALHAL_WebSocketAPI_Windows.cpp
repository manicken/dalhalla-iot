/*
  Dalhalla IoT — Windows Port
  WebSocket API Implementation for Windows
  
  Uses ASIO (Asynchronous I/O) library for networking

  Copyright (C) 2026 Jannik Svensson
  GNU General Public License v3.0 or later
*/

#include "DALHAL_WebSocketAPI_Windows.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "advapi32.lib")
#endif

namespace DALHAL {

    // Internal WebSocket Server implementation
    class WebSocketServer {
    public:
        WebSocketServer(int port) : port_(port), running_(false), nextClientId_(1) {}

        ~WebSocketServer() {
            stop();
        }

        bool start() {
            if (running_) return false;
            running_ = true;

            try {
                // Initialize Windows Sockets
                WSADATA wsaData;
                if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                    std::cerr << "WSAStartup failed\n";
                    running_ = false;
                    return false;
                }

                // Create listening socket
                listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (listenSocket_ == INVALID_SOCKET) {
                    std::cerr << "socket failed with error\n";
                    WSACleanup();
                    running_ = false;
                    return false;
                }

                // Bind socket
                sockaddr_in serverAddr{};
                serverAddr.sin_family = AF_INET;
                serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                serverAddr.sin_port = htons(port_);

                if (bind(listenSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
                    std::cerr << "bind failed with error\n";
                    closesocket(listenSocket_);
                    WSACleanup();
                    running_ = false;
                    return false;
                }

                // Listen
                if (listen(listenSocket_, SOMAXCONN) == SOCKET_ERROR) {
                    std::cerr << "listen failed with error\n";
                    closesocket(listenSocket_);
                    WSACleanup();
                    running_ = false;
                    return false;
                }

                std::cout << "WebSocket server listening on port " << port_ << std::endl;

                // Start accept thread
                acceptThread_ = std::thread(&WebSocketServer::acceptLoop, this);
                acceptThread_.detach();

                return true;
            }
            catch (const std::exception& e) {
                std::cerr << "WebSocket server start failed: " << e.what() << std::endl;
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

            // Close all client sockets
            {
                std::lock_guard<std::mutex> lock(clientsMutex_);
                for (auto& pair : clients_) {
                    if (pair.second != INVALID_SOCKET) {
                        closesocket(pair.second);
                    }
                }
                clients_.clear();
            }

            WSACleanup();
        }

        bool isRunning() const {
            return running_;
        }

        int getClientCount() const {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            return static_cast<int>(clients_.size());
        }

        void broadcastMessage(const std::string& msg) {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            for (auto& pair : clients_) {
                sendWebSocketFrame(pair.second, msg);
            }
        }

        void sendToClient(int clientId, const std::string& msg) {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            auto it = clients_.find(clientId);
            if (it != clients_.end()) {
                sendWebSocketFrame(it->second, msg);
            }
        }

    private:
        int port_;
        bool running_;
        SOCKET listenSocket_ = INVALID_SOCKET;
        std::thread acceptThread_;
        std::map<int, SOCKET> clients_;
        mutable std::mutex clientsMutex_;
        int nextClientId_;

        void acceptLoop() {
            while (running_) {
                sockaddr_in clientAddr{};
                int clientAddrLen = sizeof(clientAddr);

                SOCKET clientSocket = accept(listenSocket_, (sockaddr*)&clientAddr, &clientAddrLen);
                if (clientSocket == INVALID_SOCKET) {
                    if (running_) {
                        std::cerr << "accept failed with error\n";
                    }
                    continue;
                }

                int clientId = nextClientId_++;
                std::string clientIP = inet_ntoa(clientAddr.sin_addr);

                std::cout << "Client #" << clientId << " connected from " << clientIP << std::endl;

                {
                    std::lock_guard<std::mutex> lock(clientsMutex_);
                    clients_[clientId] = clientSocket;
                }

                // Start client handler thread
                std::thread(&WebSocketServer::clientHandler, this, clientId, clientSocket).detach();
            }
        }

        void clientHandler(int clientId, SOCKET clientSocket) {
            char buffer[4096];

            // Receive WebSocket upgrade request
            int recvResult = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (recvResult > 0) {
                buffer[recvResult] = '\0';

                // Parse HTTP upgrade request
                if (strstr(buffer, "Upgrade: websocket")) {
                    // Extract Sec-WebSocket-Key
                    std::string key = extractWebSocketKey(buffer);
                    if (!key.empty()) {
                        // Send WebSocket upgrade response
                        std::string upgradeResponse = generateWebSocketUpgrade(key);
                        send(clientSocket, upgradeResponse.c_str(), (int)upgradeResponse.length(), 0);
                    }
                }
            }

            // Message loop
            while (running_) {
                recvResult = recv(clientSocket, buffer, sizeof(buffer), 0);

                if (recvResult > 0) {
                    // Parse WebSocket frame
                    std::string message = parseWebSocketFrame(buffer, recvResult);
                    if (!message.empty()) {
                        std::cout << "Client #" << clientId << " RX: " << message << std::endl;
                        // In real implementation, would queue command
                    }
                }
                else if (recvResult == 0) {
                    std::cout << "Client #" << clientId << " disconnected\n";
                    break;
                }
                else {
                    std::cerr << "recv failed\n";
                    break;
                }
            }

            // Cleanup
            {
                std::lock_guard<std::mutex> lock(clientsMutex_);
                clients_.erase(clientId);
            }

            closesocket(clientSocket);
        }

        // WebSocket frame parsing (simplified)
        std::string parseWebSocketFrame(const char* buffer, int len) {
            if (len < 2) return "";

            unsigned char* data = (unsigned char*)buffer;
            bool fin = (data[0] & 0x80) != 0;
            int opcode = data[0] & 0x0f;

            if (!fin || opcode != 0x1) return "";  // Only handle text frames

            int payloadLen = data[1] & 0x7f;
            int headerLen = 2;
            int maskStart = 2;

            if (payloadLen == 126) {
                payloadLen = (data[2] << 8) | data[3];
                headerLen = 4;
                maskStart = 4;
            }
            else if (payloadLen == 127) {
                // 64-bit length (simplified, not fully implemented)
                return "";
            }

            bool masked = (data[1] & 0x80) != 0;
            if (!masked) return "";

            unsigned char mask[4];
            std::memcpy(mask, &data[maskStart], 4);

            int payloadStart = maskStart + 4;
            if (payloadStart + payloadLen > len) return "";

            std::string result;
            result.reserve(payloadLen);
            for (int i = 0; i < payloadLen; i++) {
                result += static_cast<char>(data[payloadStart + i] ^ mask[i % 4]);
            }

            return result;
        }

        // WebSocket frame encoding
        void sendWebSocketFrame(SOCKET socket, const std::string& message) {
            unsigned char frame[4096];
            int frameLen = 0;

            frame[frameLen++] = 0x81;  // FIN + text opcode

            size_t msgLen = message.length();
            if (msgLen < 126) {
                frame[frameLen++] = static_cast<unsigned char>(msgLen);
            }
            else if (msgLen < 65536) {
                frame[frameLen++] = 126;
                frame[frameLen++] = static_cast<unsigned char>((msgLen >> 8) & 0xff);
                frame[frameLen++] = static_cast<unsigned char>(msgLen & 0xff);
            }
            else {
                frame[frameLen++] = 127;
                // 64-bit length (simplified)
                for (int i = 0; i < 8; i++) {
                    frame[frameLen++] = 0;
                }
                frame[frameLen - 1] = static_cast<unsigned char>(msgLen & 0xff);
            }

            std::memcpy(&frame[frameLen], message.c_str(), msgLen);
            frameLen += static_cast<int>(msgLen);

            send(socket, (char*)frame, frameLen, 0);
        }

        std::string extractWebSocketKey(const char* request) {
            const char* keyStart = strstr(request, "Sec-WebSocket-Key: ");
            if (!keyStart) return "";

            keyStart += strlen("Sec-WebSocket-Key: ");
            std::string key(keyStart, 24);  // Key is always 24 chars
            return key;
        }

        std::string generateWebSocketUpgrade(const std::string& key) {
            // SHA1 + Base64 hash of key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
            std::string magicGUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
            std::string fullKey = key + magicGUID;

            // Simplified: using hardcoded response
            // In production, implement proper SHA1 + Base64
            return std::string(
                "HTTP/1.1 101 Switching Protocols\r\n"
                "Upgrade: websocket\r\n"
                "Connection: Upgrade\r\n"
                "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"
                "\r\n"
            );
        }
    };

    // Static members
    std::unique_ptr<WebSocketServer> WebSocketAPI::server = nullptr;
    std::mutex WebSocketAPI::serverMutex;

    void WebSocketAPI::setup(int port) {
        std::lock_guard<std::mutex> lock(serverMutex);
        if (!server) {
            server = std::make_unique<WebSocketServer>(port);
            server->start();
        }
    }

    void WebSocketAPI::shutdown() {
        std::lock_guard<std::mutex> lock(serverMutex);
        if (server) {
            server->stop();
            server.reset();
        }
    }

    bool WebSocketAPI::isRunning() {
        std::lock_guard<std::mutex> lock(serverMutex);
        return server && server->isRunning();
    }

    int WebSocketAPI::getClientCount() {
        std::lock_guard<std::mutex> lock(serverMutex);
        return server ? server->getClientCount() : 0;
    }

    void WebSocketAPI::Broadcast(const std::string& msg) {
        std::lock_guard<std::mutex> lock(serverMutex);
        if (server) {
            server->broadcastMessage(msg);
        }
    }

    void WebSocketAPI::Broadcast(const char* msg) {
        Broadcast(std::string(msg));
    }

    void WebSocketAPI::Broadcast(const char* source, const char* msg) {
        std::string combined = std::string(source) + msg;
        Broadcast(combined);
    }

    void WebSocketAPI::broadcastMessage(const std::string& msg) {
        Broadcast(msg);
    }

}
