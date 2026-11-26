/*
  Dalhalla IoT — Platform-Agnostic WebServer & WebSocket Interface
  
  Defines the abstract interface for WebServer and WebSocket functionality.
  Implementations exist for:
  - ESP8266/ESP32 (ESPAsyncWebServer)
  - Windows (Winsock2)
  - Linux (socket/epoll or libevent)
  - macOS (similar to Linux)

  Copyright (C) 2026 Jannik Svensson
  GNU General Public License v3.0 or later
*/

#pragma once

#include <string>
#include <functional>
#include <cstdint>

namespace DALHAL {

    // ============================================================================
    // HTTP Request Handler
    // ============================================================================
    
    class IHttpRequest {
    public:
        virtual ~IHttpRequest() = default;

        // Get HTTP method (GET, POST, etc.)
        virtual std::string method() const = 0;

        // Get request path ("/api/status", etc.)
        virtual std::string path() const = 0;

        // Get request body/content
        virtual std::string body() const = 0;

        // Get query parameter by name
        virtual std::string param(const std::string& name) const = 0;

        // Get header by name
        virtual std::string header(const std::string& name) const = 0;

        // Get remote client IP
        virtual std::string remoteIP() const = 0;
    };

    // ============================================================================
    // HTTP Response Handler
    // ============================================================================

    class IHttpResponse {
    public:
        virtual ~IHttpResponse() = default;

        // Send response with status code, content type, and body
        virtual void send(int status, const std::string& contentType, const std::string& body) = 0;

        // Shorthand: send JSON response
        virtual void sendJSON(int status, const std::string& json) = 0;

        // Shorthand: send text/plain response
        virtual void sendText(int status, const std::string& text) = 0;

        // Shorthand: send HTML response
        virtual void sendHTML(int status, const std::string& html) = 0;

        // Send file from disk
        virtual void sendFile(const std::string& filepath, const std::string& contentType = "") = 0;

        // Add response header
        virtual void setHeader(const std::string& name, const std::string& value) = 0;
    };

    // ============================================================================
    // HTTP Route Handler Callback
    // ============================================================================

    using HttpRequestHandler = std::function<void(const IHttpRequest& request, IHttpResponse& response)>;

    // ============================================================================
    // WebSocket Client
    // ============================================================================

    class IWebSocketClient {
    public:
        virtual ~IWebSocketClient() = default;

        // Get unique client ID
        virtual uint32_t id() const = 0;

        // Get client's remote IP address
        virtual std::string remoteIP() const = 0;

        // Send text message to this client
        virtual void send(const std::string& message) = 0;

        // Check if this client is still connected
        virtual bool isConnected() const = 0;

        // Close connection
        virtual void close() = 0;
    };

    // ============================================================================
    // WebSocket Event Callbacks
    // ============================================================================

    struct WebSocketEventCallbacks {
        // Called when a client connects
        // client->id() and client->remoteIP() are available
        std::function<void(IWebSocketClient& client)> onConnect;

        // Called when a client disconnects
        std::function<void(uint32_t clientId)> onDisconnect;

        // Called when a client sends a message
        // message is UTF-8 text
        std::function<void(uint32_t clientId, const std::string& message)> onMessage;

        // Called if an error occurs
        std::function<void(const std::string& error)> onError;
    };

    // ============================================================================
    // WebSocket Interface
    // ============================================================================

    class IWebSocket {
    public:
        virtual ~IWebSocket() = default;

        // Register event callbacks
        virtual void setCallbacks(const WebSocketEventCallbacks& callbacks) = 0;

        // Send message to specific client
        virtual void sendToClient(uint32_t clientId, const std::string& message) = 0;

        // Broadcast message to all connected clients
        virtual void broadcastMessage(const std::string& message) = 0;

        // Get number of currently connected clients
        virtual int getClientCount() const = 0;

        // Disconnect a specific client
        virtual void disconnectClient(uint32_t clientId) = 0;

        // Enable/disable the WebSocket endpoint
        virtual void setEnabled(bool enabled) = 0;

        // Check if WebSocket is enabled and running
        virtual bool isEnabled() const = 0;
    };

    // ============================================================================
    // WebServer Interface
    // ============================================================================

    class IWebServer {
    public:
        virtual ~IWebServer() = default;

        // ---- Lifecycle ----

        // Start the web server
        virtual void begin() = 0;

        // Stop the web server
        virtual void end() = 0;

        // Check if server is running
        virtual bool isRunning() const = 0;

        // ---- HTTP Routes ----

        // Register GET handler
        virtual void on(const std::string& path, HttpRequestHandler handler) = 0;

        // Register POST handler
        virtual void post(const std::string& path, HttpRequestHandler handler) = 0;

        // Register handler for any method
        virtual void onAny(const std::string& path, HttpRequestHandler handler) = 0;

        // ---- Static Files ----

        // Serve directory of static files
        // e.g., serveStatic("/", "/data") -> access files from /data directory
        virtual void serveStatic(const std::string& urlPath, const std::string& fsPath) = 0;

        // Set default file when accessing directory
        // e.g., setDefaultFile("index.html")
        virtual void setDefaultFile(const std::string& filename) = 0;

        // ---- WebSocket ----

        // Get WebSocket instance for this server
        virtual IWebSocket* getWebSocket() = 0;

        // ---- Server Info ----

        // Get the port the server is listening on
        virtual int getPort() const = 0;

        // Set a custom header for all responses (e.g., CORS headers)
        virtual void setDefaultHeader(const std::string& name, const std::string& value) = 0;
    };

    // ============================================================================
    // Factory: Create appropriate implementation
    // ============================================================================

    class WebServerFactory {
    public:
        // Create appropriate IWebServer implementation for current platform
        // On Windows: Returns Winsock2 implementation
        // On ESP8266/ESP32: Returns ESPAsyncWebServer wrapper
        // On Linux: Returns ASIO or libevent implementation
        static IWebServer* createWebServer(int port = 80);

        // Cleanup/release resources
        static void releaseWebServer(IWebServer* server);
    };

}  // namespace DALHAL
