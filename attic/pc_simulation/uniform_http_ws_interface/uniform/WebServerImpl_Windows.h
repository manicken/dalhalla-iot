/*
  Dalhalla IoT — Windows Implementation of IWebServer Interface
  
  Uses Winsock2 for HTTP and WebSocket functionality.

  Copyright (C) 2026 Jannik Svensson
  GNU General Public License v3.0 or later
*/

#pragma once

#ifdef _WIN32

#include "IWebServer.h"
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#endif

namespace DALHAL {

    // ============================================================================
    // Windows HTTP Request Implementation
    // ============================================================================

    class WindowsHttpRequest : public IHttpRequest {
    public:
        WindowsHttpRequest(const std::string& rawRequest, const std::string& clientIP);

        std::string method() const override;
        std::string path() const override;
        std::string body() const override;
        std::string param(const std::string& name) const override;
        std::string header(const std::string& name) const override;
        std::string remoteIP() const override;

    private:
        std::string method_;
        std::string path_;
        std::string body_;
        std::string clientIP_;
        std::map<std::string, std::string> headers_;
        std::map<std::string, std::string> params_;

        void parseRequest(const std::string& raw);
    };

    // ============================================================================
    // Windows HTTP Response Implementation
    // ============================================================================

    class WindowsHttpResponse : public IHttpResponse {
    public:
        WindowsHttpResponse(SOCKET socket);

        void send(int status, const std::string& contentType, const std::string& body) override;
        void sendJSON(int status, const std::string& json) override;
        void sendText(int status, const std::string& text) override;
        void sendHTML(int status, const std::string& html) override;
        void sendFile(const std::string& filepath, const std::string& contentType = "") override;
        void setHeader(const std::string& name, const std::string& value) override;

    private:
        SOCKET socket_;
        std::map<std::string, std::string> headers_;

        std::string getStatusMessage(int status) const;
        std::string buildHttpHeader(int status, const std::string& contentType, size_t contentLength) const;
    };

    // ============================================================================
    // Windows WebSocket Client Implementation
    // ============================================================================

    class WindowsWebSocketClient : public IWebSocketClient {
    public:
        WindowsWebSocketClient(uint32_t id, SOCKET socket, const std::string& remoteIP);
        ~WindowsWebSocketClient();

        uint32_t id() const override { return id_; }
        std::string remoteIP() const override { return remoteIP_; }
        void send(const std::string& message) override;
        bool isConnected() const override { return connected_; }
        void close() override;

    private:
        uint32_t id_;
        SOCKET socket_;
        std::string remoteIP_;
        bool connected_;

        void sendWebSocketFrame(const std::string& message);
    };

    // ============================================================================
    // Windows WebSocket Implementation
    // ============================================================================

    class WindowsWebSocket : public IWebSocket {
    public:
        WindowsWebSocket();
        ~WindowsWebSocket();

        void setCallbacks(const WebSocketEventCallbacks& callbacks) override;
        void sendToClient(uint32_t clientId, const std::string& message) override;
        void broadcastMessage(const std::string& message) override;
        int getClientCount() const override;
        void disconnectClient(uint32_t clientId) override;
        void setEnabled(bool enabled) override;
        bool isEnabled() const override { return enabled_; }

        // Internal use
        void handleNewConnection(SOCKET socket, const std::string& remoteIP);
        void handleMessage(uint32_t clientId, const std::string& message);
        void handleDisconnection(uint32_t clientId);
        uint32_t getNextClientId();

    private:
        bool enabled_;
        std::map<uint32_t, std::shared_ptr<WindowsWebSocketClient>> clients_;
        mutable std::mutex clientsMutex_;
        uint32_t nextClientId_;
        WebSocketEventCallbacks callbacks_;

        std::string parseWebSocketFrame(const char* buffer, int len);
        std::string createWebSocketFrame(const std::string& message);
    };

    // ============================================================================
    // Windows WebServer Implementation
    // ============================================================================

    class WindowsWebServer : public IWebServer {
    public:
        explicit WindowsWebServer(int port = 80);
        ~WindowsWebServer();

        void begin() override;
        void end() override;
        bool isRunning() const override;

        void on(const std::string& path, HttpRequestHandler handler) override;
        void post(const std::string& path, HttpRequestHandler handler) override;
        void onAny(const std::string& path, HttpRequestHandler handler) override;

        void serveStatic(const std::string& urlPath, const std::string& fsPath) override;
        void setDefaultFile(const std::string& filename) override;

        IWebSocket* getWebSocket() override;

        int getPort() const override { return port_; }
        void setDefaultHeader(const std::string& name, const std::string& value) override;

    private:
        int port_;
        bool running_;
        SOCKET listenSocket_;
        std::thread acceptThread_;

        std::map<std::string, HttpRequestHandler> getHandlers_;
        std::map<std::string, HttpRequestHandler> postHandlers_;
        std::map<std::string, HttpRequestHandler> anyHandlers_;
        std::map<std::string, std::string> staticPaths_;
        std::map<std::string, std::string> defaultHeaders_;
        std::string defaultFile_;

        std::unique_ptr<WindowsWebSocket> webSocket_;
        mutable std::mutex handlersMutex_;

        void acceptLoop();
        void handleClient(SOCKET clientSocket);
        bool handleHttpRequest(const WindowsHttpRequest& request, WindowsHttpResponse& response);
        bool handleWebSocketUpgrade(const std::string& rawRequest, SOCKET socket, const std::string& clientIP);
    };

}  // namespace DALHAL

#endif  // _WIN32
