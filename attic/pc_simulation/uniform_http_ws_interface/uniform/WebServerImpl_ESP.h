/*
  Dalhalla IoT — ESP8266/ESP32 Implementation of IWebServer Interface
  
  Wraps ESPAsyncWebServer to provide platform-agnostic interface.
  
  This allows the same code to run on both embedded and desktop platforms.

  Copyright (C) 2026 Jannik Svensson
  GNU General Public License v3.0 or later
*/

#pragma once

#if defined(ESP8266) || defined(ESP32)

#include "IWebServer.h"
#include <ESPAsyncWebServer.h>
#include <map>
#include <memory>

namespace DALHAL {

    // ============================================================================
    // ESP8266/ESP32 HTTP Request Adapter
    // ============================================================================

    class ESPHttpRequest : public IHttpRequest {
    public:
        explicit ESPHttpRequest(AsyncWebServerRequest* req) : espRequest_(req) {}

        std::string method() const override {
            switch (espRequest_->method()) {
                case HTTP_GET: return "GET";
                case HTTP_POST: return "POST";
                case HTTP_PUT: return "PUT";
                case HTTP_DELETE: return "DELETE";
                case HTTP_PATCH: return "PATCH";
                case HTTP_HEAD: return "HEAD";
                case HTTP_OPTIONS: return "OPTIONS";
                default: return "UNKNOWN";
            }
        }

        std::string path() const override {
            return espRequest_->url().c_str();
        }

        std::string body() const override {
            if (espRequest_->hasParam("plain", true)) {
                return espRequest_->getParam("plain", true)->value().c_str();
            }
            return "";
        }

        std::string param(const std::string& name) const override {
            if (espRequest_->hasParam(name.c_str())) {
                return espRequest_->getParam(name.c_str())->value().c_str();
            }
            return "";
        }

        std::string header(const std::string& name) const override {
            if (espRequest_->hasHeader(name.c_str())) {
                return espRequest_->getHeader(name.c_str()).c_str();
            }
            return "";
        }

        std::string remoteIP() const override {
            return espRequest_->client()->remoteIP().toString().c_str();
        }

    private:
        AsyncWebServerRequest* espRequest_;
    };

    // ============================================================================
    // ESP8266/ESP32 HTTP Response Adapter
    // ============================================================================

    class ESPHttpResponse : public IHttpResponse {
    public:
        explicit ESPHttpResponse(AsyncWebServerRequest* req) : espRequest_(req) {}

        void send(int status, const std::string& contentType, const std::string& body) override {
            auto response = espRequest_->beginResponse(status, contentType.c_str(), body.c_str());
            
            for (const auto& h : customHeaders_) {
                response->addHeader(h.first.c_str(), h.second.c_str());
            }
            
            espRequest_->send(response);
        }

        void sendJSON(int status, const std::string& json) override {
            send(status, "application/json", json);
        }

        void sendText(int status, const std::string& text) override {
            send(status, "text/plain", text);
        }

        void sendHTML(int status, const std::string& html) override {
            send(status, "text/html", html);
        }

        void sendFile(const std::string& filepath, const std::string& contentType = "") override {
            String ct = contentType.empty() ? "application/octet-stream" : contentType.c_str();
            espRequest_->send(SPIFFS, filepath.c_str(), ct.c_str());
        }

        void setHeader(const std::string& name, const std::string& value) override {
            customHeaders_[name] = value;
        }

    private:
        AsyncWebServerRequest* espRequest_;
        std::map<std::string, std::string> customHeaders_;
    };

    // ============================================================================
    // ESP8266/ESP32 WebSocket Client Adapter
    // ============================================================================

    class ESPWebSocketClient : public IWebSocketClient {
    public:
        ESPWebSocketClient(uint32_t id, AsyncWebSocketClient* espClient)
            : id_(id), espClient_(espClient) {}

        uint32_t id() const override { return id_; }

        std::string remoteIP() const override {
            return espClient_->remoteIP().toString().c_str();
        }

        void send(const std::string& message) override {
            if (espClient_ && espClient_->canSend()) {
                espClient_->text(message.c_str());
            }
        }

        bool isConnected() const override {
            return espClient_ != nullptr && espClient_->status() == WS_CONNECTED;
        }

        void close() override {
            if (espClient_) {
                espClient_->close();
            }
        }

        AsyncWebSocketClient* getESPClient() const { return espClient_; }

    private:
        uint32_t id_;
        AsyncWebSocketClient* espClient_;
    };

    // ============================================================================
    // ESP8266/ESP32 WebSocket Adapter
    // ============================================================================

    class ESPWebSocket : public IWebSocket {
    public:
        ESPWebSocket(AsyncWebSocket* espWs) : espWs_(espWs) {}

        void setCallbacks(const WebSocketEventCallbacks& callbacks) override {
            callbacks_ = callbacks;

            // Setup ESP AsyncWebSocket callbacks
            espWs_->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client,
                                   AwsEventType type, void* arg, uint8_t* data, size_t len) {
                onESPWebSocketEvent(server, client, type, arg, data, len);
            });
        }

        void sendToClient(uint32_t clientId, const std::string& message) override {
            AsyncWebSocketClient* client = espWs_->client(clientId);
            if (client && client->canSend()) {
                client->text(message.c_str());
            }
        }

        void broadcastMessage(const std::string& message) override {
            espWs_->textAll(message.c_str());
        }

        int getClientCount() const override {
            return espWs_->count();
        }

        void disconnectClient(uint32_t clientId) override {
            AsyncWebSocketClient* client = espWs_->client(clientId);
            if (client) {
                client->close();
            }
        }

        void setEnabled(bool enabled) override {
            if (enabled) {
                espWs_->enable(true);
            } else {
                espWs_->enable(false);
            }
        }

        bool isEnabled() const override {
            return espWs_->enabled();
        }

    private:
        AsyncWebSocket* espWs_;
        WebSocketEventCallbacks callbacks_;
        std::map<uint32_t, std::shared_ptr<ESPWebSocketClient>> clients_;

        void onESPWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                               AwsEventType type, void* arg, uint8_t* data, size_t len) {
            uint32_t clientId = client->id();

            switch (type) {
                case WS_EVT_CONNECT: {
                    auto wsClient = std::make_shared<ESPWebSocketClient>(clientId, client);
                    clients_[clientId] = wsClient;
                    if (callbacks_.onConnect) {
                        callbacks_.onConnect(*wsClient);
                    }
                    break;
                }

                case WS_EVT_DISCONNECT: {
                    clients_.erase(clientId);
                    if (callbacks_.onDisconnect) {
                        callbacks_.onDisconnect(clientId);
                    }
                    break;
                }

                case WS_EVT_DATA: {
                    AwsFrameInfo* info = (AwsFrameInfo*)arg;

                    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
                        std::string message((char*)data, len);
                        if (callbacks_.onMessage) {
                            callbacks_.onMessage(clientId, message);
                        }
                    }
                    break;
                }

                case WS_EVT_ERROR: {
                    if (callbacks_.onError) {
                        callbacks_.onError("WebSocket error");
                    }
                    break;
                }

                default:
                    break;
            }
        }
    };

    // ============================================================================
    // ESP8266/ESP32 WebServer Adapter
    // ============================================================================

    class ESPWebServer : public IWebServer {
    public:
        explicit ESPWebServer(int port = 80)
            : port_(port), asyncServer_(std::make_unique<AsyncWebServer>(port)) {
            asyncWs_ = std::make_unique<AsyncWebSocket>("/ws");
            espWebSocket_ = std::make_unique<ESPWebSocket>(asyncWs_.get());
        }

        void begin() override {
            asyncServer_->addHandler(asyncWs_.get());
            asyncServer_->begin();
        }

        void end() override {
            // AsyncWebServer doesn't provide a proper stop method
            // This is a limitation of the ESP library
        }

        bool isRunning() const override {
            return true;  // Always running on ESP if begin() was called
        }

        void on(const std::string& path, HttpRequestHandler handler) override {
            asyncServer_->on(path.c_str(), HTTP_GET, [handler](AsyncWebServerRequest* request) {
                ESPHttpRequest httpRequest(request);
                ESPHttpResponse httpResponse(request);
                handler(httpRequest, httpResponse);
            });
        }

        void post(const std::string& path, HttpRequestHandler handler) override {
            asyncServer_->on(path.c_str(), HTTP_POST, [handler](AsyncWebServerRequest* request) {
                ESPHttpRequest httpRequest(request);
                ESPHttpResponse httpResponse(request);
                handler(httpRequest, httpResponse);
            });
        }

        void onAny(const std::string& path, HttpRequestHandler handler) override {
            asyncServer_->onNotFound([path, handler](AsyncWebServerRequest* request) {
                if (request->url() == path) {
                    ESPHttpRequest httpRequest(request);
                    ESPHttpResponse httpResponse(request);
                    handler(httpRequest, httpResponse);
                } else {
                    request->send(404);
                }
            });
        }

        void serveStatic(const std::string& urlPath, const std::string& fsPath) override {
            asyncServer_->serveStatic(urlPath.c_str(), SPIFFS, fsPath.c_str());
        }

        void setDefaultFile(const std::string& filename) override {
            // ESPAsyncWebServer handles this automatically in serveStatic
        }

        IWebSocket* getWebSocket() override {
            return espWebSocket_.get();
        }

        int getPort() const override {
            return port_;
        }

        void setDefaultHeader(const std::string& name, const std::string& value) override {
            asyncServer_->setDefaultHeader(name.c_str(), value.c_str());
        }

    private:
        int port_;
        std::unique_ptr<AsyncWebServer> asyncServer_;
        std::unique_ptr<AsyncWebSocket> asyncWs_;
        std::unique_ptr<ESPWebSocket> espWebSocket_;
    };

    // ============================================================================
    // Factory Implementation for ESP
    // ============================================================================

    // This gets included in the factory definition
    // to provide platform-specific implementation

}  // namespace DALHAL

#endif  // ESP8266 || ESP32
