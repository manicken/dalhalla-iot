/*
  Dalhalla IoT — Windows WebServer Implementation
  
  Copyright (C) 2026 Jannik Svensson
  GNU General Public License v3.0 or later
*/

#ifndef _WIN32

#include "WebServerImpl_Windows.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>

namespace DALHAL {

    // ============================================================================
    // WindowsHttpRequest Implementation
    // ============================================================================

    WindowsHttpRequest::WindowsHttpRequest(const std::string& rawRequest, const std::string& clientIP)
        : clientIP_(clientIP) {
        parseRequest(rawRequest);
    }

    void WindowsHttpRequest::parseRequest(const std::string& raw) {
        std::istringstream stream(raw);
        std::string line;

        // Parse request line: GET /path HTTP/1.1
        if (std::getline(stream, line)) {
            std::istringstream requestLine(line);
            requestLine >> method_ >> path_;
            
            // Remove query string from path
            size_t queryPos = path_.find('?');
            if (queryPos != std::string::npos) {
                std::string queryString = path_.substr(queryPos + 1);
                path_ = path_.substr(0, queryPos);

                // Parse query parameters
                std::istringstream queryStream(queryString);
                std::string pair;
                while (std::getline(queryStream, pair, '&')) {
                    size_t eqPos = pair.find('=');
                    if (eqPos != std::string::npos) {
                        std::string key = pair.substr(0, eqPos);
                        std::string value = pair.substr(eqPos + 1);
                        params_[key] = value;
                    }
                }
            }
        }

        // Parse headers
        while (std::getline(stream, line)) {
            if (line == "\r" || line.empty()) break;

            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string name = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 2);
                
                // Remove trailing \r
                if (!value.empty() && value.back() == '\r') {
                    value.pop_back();
                }
                
                headers_[name] = value;
            }
        }

        // Body is remaining content
        std::string remaining;
        while (std::getline(stream, remaining)) {
            body_ += remaining + "\n";
        }
        if (!body_.empty() && body_.back() == '\n') {
            body_.pop_back();
        }
    }

    std::string WindowsHttpRequest::method() const { return method_; }
    std::string WindowsHttpRequest::path() const { return path_; }
    std::string WindowsHttpRequest::body() const { return body_; }
    std::string WindowsHttpRequest::remoteIP() const { return clientIP_; }

    std::string WindowsHttpRequest::param(const std::string& name) const {
        auto it = params_.find(name);
        return (it != params_.end()) ? it->second : "";
    }

    std::string WindowsHttpRequest::header(const std::string& name) const {
        auto it = headers_.find(name);
        return (it != headers_.end()) ? it->second : "";
    }

    // ============================================================================
    // WindowsHttpResponse Implementation
    // ============================================================================

    WindowsHttpResponse::WindowsHttpResponse(SOCKET socket) : socket_(socket) {}

    std::string WindowsHttpResponse::getStatusMessage(int status) const {
        switch (status) {
            case 200: return "OK";
            case 201: return "Created";
            case 204: return "No Content";
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 500: return "Internal Server Error";
            case 503: return "Service Unavailable";
            default: return "Unknown";
        }
    }

    std::string WindowsHttpResponse::buildHttpHeader(int status, const std::string& contentType, size_t contentLength) const {
        std::ostringstream header;
        header << "HTTP/1.1 " << status << " " << getStatusMessage(status) << "\r\n";
        header << "Content-Type: " << contentType << "\r\n";
        header << "Content-Length: " << contentLength << "\r\n";
        header << "Connection: close\r\n";

        for (const auto& h : headers_) {
            header << h.first << ": " << h.second << "\r\n";
        }

        header << "\r\n";
        return header.str();
    }

    void WindowsHttpResponse::send(int status, const std::string& contentType, const std::string& body) {
        std::string httpHeader = buildHttpHeader(status, contentType, body.length());
        std::string response = httpHeader + body;
        ::send(socket_, response.c_str(), (int)response.length(), 0);
    }

    void WindowsHttpResponse::sendJSON(int status, const std::string& json) {
        send(status, "application/json", json);
    }

    void WindowsHttpResponse::sendText(int status, const std::string& text) {
        send(status, "text/plain", text);
    }

    void WindowsHttpResponse::sendHTML(int status, const std::string& html) {
        send(status, "text/html", html);
    }

    void WindowsHttpResponse::sendFile(const std::string& filepath, const std::string& contentType) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            sendText(404, "File not found");
            return;
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string type = contentType.empty() ? "application/octet-stream" : contentType;
        send(200, type, content);
    }

    void WindowsHttpResponse::setHeader(const std::string& name, const std::string& value) {
        headers_[name] = value;
    }

    // ============================================================================
    // WindowsWebSocketClient Implementation
    // ============================================================================

    WindowsWebSocketClient::WindowsWebSocketClient(uint32_t id, SOCKET socket, const std::string& remoteIP)
        : id_(id), socket_(socket), remoteIP_(remoteIP), connected_(true) {}

    WindowsWebSocketClient::~WindowsWebSocketClient() {
        if (socket_ != INVALID_SOCKET) {
            closesocket(socket_);
        }
    }

    void WindowsWebSocketClient::send(const std::string& message) {
        if (!connected_) return;
        sendWebSocketFrame(message);
    }

    void WindowsWebSocketClient::sendWebSocketFrame(const std::string& message) {
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

        memcpy(&frame[frameLen], message.c_str(), msgLen);
        frameLen += static_cast<int>(msgLen);

        ::send(socket_, (char*)frame, frameLen, 0);
    }

    void WindowsWebSocketClient::close() {
        connected_ = false;
        if (socket_ != INVALID_SOCKET) {
            closesocket(socket_);
            socket_ = INVALID_SOCKET;
        }
    }

    // ============================================================================
    // WindowsWebSocket Implementation
    // ============================================================================

    WindowsWebSocket::WindowsWebSocket() : enabled_(false), nextClientId_(1) {}

    WindowsWebSocket::~WindowsWebSocket() {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        for (auto& pair : clients_) {
            pair.second->close();
        }
        clients_.clear();
    }

    void WindowsWebSocket::setCallbacks(const WebSocketEventCallbacks& callbacks) {
        callbacks_ = callbacks;
    }

    void WindowsWebSocket::sendToClient(uint32_t clientId, const std::string& message) {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        auto it = clients_.find(clientId);
        if (it != clients_.end()) {
            it->second->send(message);
        }
    }

    void WindowsWebSocket::broadcastMessage(const std::string& message) {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        for (auto& pair : clients_) {
            pair.second->send(message);
        }
    }

    int WindowsWebSocket::getClientCount() const {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        return static_cast<int>(clients_.size());
    }

    void WindowsWebSocket::disconnectClient(uint32_t clientId) {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        auto it = clients_.find(clientId);
        if (it != clients_.end()) {
            it->second->close();
            clients_.erase(it);
        }
    }

    void WindowsWebSocket::setEnabled(bool enabled) {
        enabled_ = enabled;
    }

    void WindowsWebSocket::handleNewConnection(SOCKET socket, const std::string& remoteIP) {
        if (!enabled_) return;

        uint32_t clientId = nextClientId_++;
        auto client = std::make_shared<WindowsWebSocketClient>(clientId, socket, remoteIP);

        {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            clients_[clientId] = client;
        }

        if (callbacks_.onConnect) {
            callbacks_.onConnect(*client);
        }
    }

    void WindowsWebSocket::handleMessage(uint32_t clientId, const std::string& message) {
        if (callbacks_.onMessage) {
            callbacks_.onMessage(clientId, message);
        }
    }

    void WindowsWebSocket::handleDisconnection(uint32_t clientId) {
        {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            auto it = clients_.find(clientId);
            if (it != clients_.end()) {
                it->second->close();
                clients_.erase(it);
            }
        }

        if (callbacks_.onDisconnect) {
            callbacks_.onDisconnect(clientId);
        }
    }

    std::string WindowsWebSocket::parseWebSocketFrame(const char* buf, int len) {
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
            return "";  // 64-bit not implemented
        }

        if (maskStart + 4 + payloadLen > len) return "";

        unsigned char mask[4];
        memcpy(mask, &data[maskStart], 4);

        int payloadStart = maskStart + 4;
        std::string result;
        result.reserve(payloadLen);

        for (int i = 0; i < payloadLen; i++) {
            result += static_cast<char>(data[payloadStart + i] ^ mask[i % 4]);
        }

        return result;
    }

    // ============================================================================
    // WindowsWebServer Implementation
    // ============================================================================

    WindowsWebServer::WindowsWebServer(int port)
        : port_(port), running_(false), listenSocket_(INVALID_SOCKET) {
        webSocket_ = std::make_unique<WindowsWebSocket>();
    }

    WindowsWebServer::~WindowsWebServer() {
        end();
    }

    void WindowsWebServer::begin() {
        if (running_) return;
        running_ = true;

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            running_ = false;
            return;
        }

        listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket_ == INVALID_SOCKET) {
            WSACleanup();
            running_ = false;
            return;
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddr.sin_port = htons(port_);

        if (bind(listenSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(listenSocket_);
            WSACleanup();
            running_ = false;
            return;
        }

        if (listen(listenSocket_, SOMAXCONN) == SOCKET_ERROR) {
            closesocket(listenSocket_);
            WSACleanup();
            running_ = false;
            return;
        }

        std::cout << "Web server listening on port " << port_ << std::endl;
        webSocket_->setEnabled(true);

        acceptThread_ = std::thread(&WindowsWebServer::acceptLoop, this);
        acceptThread_.detach();
    }

    void WindowsWebServer::end() {
        running_ = false;

        if (listenSocket_ != INVALID_SOCKET) {
            closesocket(listenSocket_);
            listenSocket_ = INVALID_SOCKET;
        }

        WSACleanup();
    }

    bool WindowsWebServer::isRunning() const {
        return running_;
    }

    void WindowsWebServer::on(const std::string& path, HttpRequestHandler handler) {
        std::lock_guard<std::mutex> lock(handlersMutex_);
        getHandlers_[path] = handler;
    }

    void WindowsWebServer::post(const std::string& path, HttpRequestHandler handler) {
        std::lock_guard<std::mutex> lock(handlersMutex_);
        postHandlers_[path] = handler;
    }

    void WindowsWebServer::onAny(const std::string& path, HttpRequestHandler handler) {
        std::lock_guard<std::mutex> lock(handlersMutex_);
        anyHandlers_[path] = handler;
    }

    void WindowsWebServer::serveStatic(const std::string& urlPath, const std::string& fsPath) {
        std::lock_guard<std::mutex> lock(handlersMutex_);
        staticPaths_[urlPath] = fsPath;
    }

    void WindowsWebServer::setDefaultFile(const std::string& filename) {
        defaultFile_ = filename;
    }

    IWebSocket* WindowsWebServer::getWebSocket() {
        return webSocket_.get();
    }

    void WindowsWebServer::setDefaultHeader(const std::string& name, const std::string& value) {
        std::lock_guard<std::mutex> lock(handlersMutex_);
        defaultHeaders_[name] = value;
    }

    void WindowsWebServer::acceptLoop() {
        while (running_) {
            sockaddr_in clientAddr{};
            int clientAddrLen = sizeof(clientAddr);

            SOCKET clientSocket = accept(listenSocket_, (sockaddr*)&clientAddr, &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                if (running_) continue;
                break;
            }

            std::string clientIP = inet_ntoa(clientAddr.sin_addr);
            std::thread(&WindowsWebServer::handleClient, this, clientSocket).detach();
        }
    }

    void WindowsWebServer::handleClient(SOCKET clientSocket) {
        char buffer[8192];
        int recvLen = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (recvLen > 0) {
            buffer[recvLen] = '\0';
            std::string rawRequest(buffer);
            std::string clientIP = "127.0.0.1";  // TODO: Get actual client IP

            // Check if WebSocket upgrade
            if (rawRequest.find("Upgrade: websocket") != std::string::npos ||
                rawRequest.find("Upgrade: WebSocket") != std::string::npos) {
                handleWebSocketUpgrade(rawRequest, clientSocket, clientIP);
            }
            else {
                // Regular HTTP request
                WindowsHttpRequest httpRequest(rawRequest, clientIP);
                WindowsHttpResponse httpResponse(clientSocket);

                if (!handleHttpRequest(httpRequest, httpResponse)) {
                    httpResponse.sendText(404, "Not Found");
                }
            }
        }

        closesocket(clientSocket);
    }

    bool WindowsWebServer::handleHttpRequest(const WindowsHttpRequest& request, WindowsHttpResponse& response) {
        std::lock_guard<std::mutex> lock(handlersMutex_);

        // Try exact path match
        if (request.method() == "GET") {
            auto it = getHandlers_.find(request.path());
            if (it != getHandlers_.end()) {
                it->second(request, response);
                return true;
            }
        }
        else if (request.method() == "POST") {
            auto it = postHandlers_.find(request.path());
            if (it != postHandlers_.end()) {
                it->second(request, response);
                return true;
            }
        }

        // Try any method handler
        auto it = anyHandlers_.find(request.path());
        if (it != anyHandlers_.end()) {
            it->second(request, response);
            return true;
        }

        return false;
    }

    uint32_t WindowsWebSocket::getNextClientId() {
        return nextClientId_;
    }

    bool WindowsWebServer::handleWebSocketUpgrade(const std::string& rawRequest, SOCKET socket, const std::string& clientIP) {
        // Extract Sec-WebSocket-Key
        size_t keyPos = rawRequest.find("Sec-WebSocket-Key: ");
        if (keyPos == std::string::npos) return false;

        std::string key = rawRequest.substr(keyPos + 19, 24);

        // Send upgrade response
        std::string upgrade = "HTTP/1.1 101 Switching Protocols\r\n"
                             "Upgrade: websocket\r\n"
                             "Connection: Upgrade\r\n"
                             "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"
                             "\r\n";

        ::send(socket, upgrade.c_str(), (int)upgrade.length(), 0);

        // Notify WebSocket of new connection
        webSocket_->handleNewConnection(socket, clientIP);

        // Message loop for this WebSocket client
        char buffer[4096];
        uint32_t clientId = webSocket_->getNextClientId() - 1;

        while (running_) {
            int recvLen = recv(socket, buffer, sizeof(buffer), 0);
            if (recvLen <= 0) break;

            std::string message = webSocket_->parseWebSocketFrame(buffer, recvLen);
            if (!message.empty()) {
                webSocket_->handleMessage(clientId, message);
            }
        }

        webSocket_->handleDisconnection(clientId);
        return true;
    }

    // ============================================================================
    // Factory Implementation
    // ============================================================================

    IWebServer* WebServerFactory::createWebServer(int port) {
#ifdef _WIN32
        return new WindowsWebServer(port);
#else
        // TODO: Implement for other platforms
        return nullptr;
#endif
    }

    void WebServerFactory::releaseWebServer(IWebServer* server) {
        delete server;
    }

}  // namespace DALHAL

#endif  // _WIN32
