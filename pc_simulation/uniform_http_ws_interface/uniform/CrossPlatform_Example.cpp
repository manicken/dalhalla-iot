/*
  Dalhalla IoT — Cross-Platform Example
  
  This example code works identically on:
  - Windows (Winsock2)
  - ESP8266 (AsyncWebServer)
  - ESP32 (AsyncWebServer)
  - Linux/macOS (when implemented)
  
  No platform-specific includes or code needed!

  Copyright (C) 2026 Jannik Svensson
  GNU General Public License v3.0 or later
*/

#include "IWebServer.h"
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>  // For JSON (optional, use string if not available)

using json = nlohmann::json;
using namespace DALHAL;

class IoTApplication {
public:
    IoTApplication(int port = 80) : port_(port), server_(nullptr) {}

    ~IoTApplication() {
        if (server_) {
            WebServerFactory::releaseWebServer(server_);
        }
    }

    void setup() {
        // Create platform-appropriate server
        server_ = WebServerFactory::createWebServer(port_);
        if (!server_) {
            std::cerr << "Failed to create web server" << std::endl;
            return;
        }

        // Setup HTTP routes
        setupHttpRoutes();

        // Setup WebSocket
        setupWebSocket();

        // Start server
        server_->begin();
        std::cout << "IoT Application started on port " << port_ << std::endl;
    }

    void teardown() {
        if (server_) {
            server_->end();
        }
    }

private:
    int port_;
    IWebServer* server_;

    void setupHttpRoutes() {
        // GET /api/status - Get system status
        server_->on("/api/status", [this](const IHttpRequest& req, IHttpResponse& res) {
            handleGetStatus(req, res);
        });

        // GET /api/device - List devices
        server_->on("/api/device", [this](const IHttpRequest& req, IHttpResponse& res) {
            handleGetDevices(req, res);
        });

        // POST /api/device/control - Control a device
        server_->post("/api/device/control", [this](const IHttpRequest& req, IHttpResponse& res) {
            handleDeviceControl(req, res);
        });

        // GET /api/config - Get configuration
        server_->on("/api/config", [this](const IHttpRequest& req, IHttpResponse& res) {
            handleGetConfig(req, res);
        });

        // Serve static files
        server_->serveStatic("/", "/www");
        server_->setDefaultFile("index.html");

        // Set CORS headers
        server_->setDefaultHeader("Access-Control-Allow-Origin", "*");
        server_->setDefaultHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    }

    void setupWebSocket() {
        IWebSocket* ws = server_->getWebSocket();
        if (!ws) return;

        WebSocketEventCallbacks callbacks;

        callbacks.onConnect = [this](IWebSocketClient& client) {
            std::cout << "WebSocket client connected: " << client.id() 
                     << " from " << client.remoteIP() << std::endl;
            
            // Send welcome message
            client.send(R"({"type":"welcome","message":"Connected to IoT server"})");
        };

        callbacks.onDisconnect = [this](uint32_t clientId) {
            std::cout << "WebSocket client disconnected: " << clientId << std::endl;
        };

        callbacks.onMessage = [this](uint32_t clientId, const std::string& message) {
            std::cout << "WebSocket message from " << clientId << ": " << message << std::endl;
            handleWebSocketMessage(clientId, message);
        };

        callbacks.onError = [this](const std::string& error) {
            std::cerr << "WebSocket error: " << error << std::endl;
        };

        ws->setCallbacks(callbacks);
    }

    // ========================================================================
    // HTTP Request Handlers
    // ========================================================================

    void handleGetStatus(const IHttpRequest& req, IHttpResponse& res) {
        json status = {
            {"uptime", 12345},
            {"version", "1.0.0"},
            {"platform", getPlatformName()},
            {"clients", server_->getWebSocket()->getClientCount()},
            {"timestamp", getCurrentTimestamp()}
        };

        res.sendJSON(200, status.dump());
    }

    void handleGetDevices(const IHttpRequest& req, IHttpResponse& res) {
        json devices = json::array();
        
        devices.push_back({
            {"id", 1},
            {"name", "Living Room Light"},
            {"type", "light"},
            {"state", true},
            {"brightness", 80}
        });

        devices.push_back({
            {"id", 2},
            {"name", "Bedroom Fan"},
            {"type", "fan"},
            {"state", false},
            {"speed", 0}
        });

        json response = {
            {"devices", devices},
            {"count", devices.size()}
        };

        res.sendJSON(200, response.dump());
    }

    void handleDeviceControl(const IHttpRequest& req, IHttpResponse& res) {
        try {
            json payload = json::parse(req.body());
            
            int deviceId = payload["device_id"];
            std::string command = payload["command"];

            std::cout << "Controlling device " << deviceId << " with command: " << command << std::endl;

            // Broadcast to WebSocket clients
            json notification = {
                {"type", "device_control"},
                {"device_id", deviceId},
                {"command", command},
                {"timestamp", getCurrentTimestamp()}
            };

            server_->getWebSocket()->broadcastMessage(notification.dump());

            // Send response
            json response = {
                {"success", true},
                {"message", "Command executed"},
                {"device_id", deviceId}
            };

            res.sendJSON(200, response.dump());
        }
        catch (const std::exception& e) {
            json error = {
                {"success", false},
                {"error", e.what()}
            };
            res.sendJSON(400, error.dump());
        }
    }

    void handleGetConfig(const IHttpRequest& req, IHttpResponse& res) {
        json config = {
            {"name", "Dalhalla IoT Hub"},
            {"version", "1.0.0"},
            {"features", {
                "http_api",
                "websocket",
                "mqtt",
                "device_control"
            }},
            {"limits", {
                {"max_devices", 100},
                {"max_websocket_clients", 50}
            }}
        };

        res.sendJSON(200, config.dump());
    }

    // ========================================================================
    // WebSocket Message Handler
    // ========================================================================

    void handleWebSocketMessage(uint32_t clientId, const std::string& message) {
        try {
            json msg = json::parse(message);
            std::string type = msg.value("type", "");

            if (type == "ping") {
                json response = {
                    {"type", "pong"},
                    {"timestamp", getCurrentTimestamp()}
                };
                server_->getWebSocket()->sendToClient(clientId, response.dump());
            }
            else if (type == "subscribe") {
                std::string topic = msg.value("topic", "");
                std::cout << "Client " << clientId << " subscribed to: " << topic << std::endl;

                json response = {
                    {"type", "subscribed"},
                    {"topic", topic}
                };
                server_->getWebSocket()->sendToClient(clientId, response.dump());
            }
            else if (type == "device_command") {
                int deviceId = msg.value("device_id", -1);
                std::string command = msg.value("command", "");

                handleWebSocketDeviceCommand(clientId, deviceId, command);
            }
            else {
                std::cout << "Unknown message type: " << type << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing WebSocket message: " << e.what() << std::endl;

            json error = {
                {"type", "error"},
                {"message", e.what()}
            };
            server_->getWebSocket()->sendToClient(clientId, error.dump());
        }
    }

    void handleWebSocketDeviceCommand(uint32_t clientId, int deviceId, const std::string& command) {
        std::cout << "Device command from client " << clientId 
                 << " - Device: " << deviceId 
                 << " Command: " << command << std::endl;

        // Broadcast to all clients
        json notification = {
            {"type", "device_update"},
            {"device_id", deviceId},
            {"command", command},
            {"from_client", clientId}
        };

        server_->getWebSocket()->broadcastMessage(notification.dump());
    }

    // ========================================================================
    // Utility Functions
    // ========================================================================

    std::string getPlatformName() const {
#if defined(_WIN32)
        return "Windows";
#elif defined(ESP32)
        return "ESP32";
#elif defined(ESP8266)
        return "ESP8266";
#elif defined(__linux__)
        return "Linux";
#elif defined(__APPLE__)
        return "macOS";
#else
        return "Unknown";
#endif
    }

    std::string getCurrentTimestamp() const {
        // Simple timestamp - could use std::time for more details
        return "2026-04-03T10:30:00Z";
    }
};

// ============================================================================
// Main / Setup Entry Point
// ============================================================================

#ifdef ESP8266
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Connect to WiFi (ESP-specific code)
    // WiFi.begin(ssid, password);
    // while (WiFi.status() != WL_CONNECTED) delay(500);
    
    IoTApplication app(80);
    app.setup();
    
    // Note: On ESP, setup runs once, then loop() is called repeatedly
    // You would put WebSocket message handling in loop()
}

void loop() {
    delay(100);
}

#else
// Windows/Linux main()
int main() {
    IoTApplication app(8080);
    app.setup();

    std::cout << "\nServer running. Press any key to exit..." << std::endl;
    std::cin.get();

    app.teardown();
    return 0;
}
#endif
