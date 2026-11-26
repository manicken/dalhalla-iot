# Dalhalla IoT — Quick Reference Card

## Architecture Comparison

### Without Abstraction (Your Old Code)
```cpp
// Windows Code
#ifdef _WIN32
#include <winsock2.h>
// 500+ lines of socket management...
#endif

// ESP Code
#ifdef ESP8266
#include <ESPAsyncWebServer.h>
AsyncWebServer server(80);
server.on("/api", HTTP_GET, handler);
#endif

// Result: Two completely different code bases!
```

### With Abstraction Layer (New Code)
```cpp
#include "IWebServer.h"

IWebServer* server = WebServerFactory::createWebServer(80);
server->on("/api", handler);  // ← Same on ALL platforms!
```

---

## Common Tasks

### Initialize Server

```cpp
// Old (Platform-specific)
#ifdef _WIN32
    AsyncWebServer server(80);
#elif ESP8266
    AsyncWebServer server(80);
#endif

// New (Platform-agnostic)
IWebServer* server = WebServerFactory::createWebServer(80);
server->begin();
```

### Register HTTP Route

```cpp
// Old
#ifdef _WIN32
    // Windows Winsock code...
#elif ESP8266
    asyncServer.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{...}");
    });
#endif

// New
server->on("/api/status", [](const IHttpRequest& req, IHttpResponse& res) {
    res.sendJSON(200, "{...}");
});
```

### Handle POST Requests

```cpp
// Old
#ifdef ESP8266
    server.on("/api/device", HTTP_POST, [](AsyncWebServerRequest *request) {
        String body = request->arg("plain");
        // process...
        request->send(200, "application/json", "{...}");
    });
#endif

// New
server->post("/api/device", [](const IHttpRequest& req, IHttpResponse& res) {
    std::string body = req.body();
    // process...
    res.sendJSON(200, "{...}");
});
```

### WebSocket Broadcasting

```cpp
// Old
#ifdef ESP8266
    ws.textAll("message");
#elif _WIN32
    // Custom loop through sockets...
#endif

// New
server->getWebSocket()->broadcastMessage("message");
```

### Send to Specific Client

```cpp
// Old
AsyncWebSocketClient* client = ws.client(clientId);
if (client && client->canSend()) {
    client->text("message");
}

// New
server->getWebSocket()->sendToClient(clientId, "message");
```

### Handle WebSocket Events

```cpp
// Old
ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
              AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        // handle connect
    }
});

// New
WebSocketEventCallbacks callbacks;
callbacks.onConnect = [](IWebSocketClient& client) {
    std::cout << "Connected: " << client.id() << std::endl;
};
callbacks.onMessage = [](uint32_t clientId, const std::string& msg) {
    std::cout << "Message: " << msg << std::endl;
};
callbacks.onDisconnect = [](uint32_t clientId) {
    std::cout << "Disconnected: " << clientId << std::endl;
};
server->getWebSocket()->setCallbacks(callbacks);
```

### Get Request Data

```cpp
// Old (ESP)
const char* method = request->methodToString();
String url = request->url();
String param = request->arg("key");
String body = request->arg("plain");

// Old (Windows)
// Parse from raw socket buffer...

// New (Same on all platforms!)
std::string method = request.method();
std::string path = request.path();
std::string param = request.param("key");
std::string body = request.body();
```

### Send Response

```cpp
// Old
request->send(200, "application/json", "{\"status\":\"ok\"}");
request->send(404, "text/plain", "Not found");

// New
res.sendJSON(200, "{\"status\":\"ok\"}");
res.sendText(404, "Not found");
res.sendHTML(200, "<h1>Hello</h1>");
res.sendFile("index.html", "text/html");
```

---

## Data Type Mapping

| Task | IWebServer | Windows | ESP8266/32 |
|------|-----------|---------|-----------|
| Request | `IHttpRequest` | `WindowsHttpRequest` | `ESPHttpRequest` |
| Response | `IHttpResponse` | `WindowsHttpResponse` | `ESPHttpResponse` |
| WS Client | `IWebSocketClient` | `WindowsWebSocketClient` | `ESPWebSocketClient` |
| WebSocket | `IWebSocket` | `WindowsWebSocket` | `ESPWebSocket` |
| Server | `IWebServer` | `WindowsWebServer` | `ESPWebServer` |

---

## File Structure Changes

### Before
```
src/
├── main_windows.cpp        (Different!)
├── main_esp.cpp            (Different!)
├── api_handlers_windows.cpp (Different!)
└── api_handlers_esp.cpp    (Different!)
```

### After
```
src/
├── main.cpp                ← ONE main file
├── api_handlers.cpp        ← ONE handler file
└── platform_impl/
    ├── WebServerImpl_Windows.cpp
    ├── WebServerImpl_Windows.h
    ├── WebServerImpl_ESP.h
    └── WebServerImpl_Linux.h  (future)
```

---

## Compilation Commands

### Windows
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
.\CrossPlatform_Example.exe
```

### ESP32 (PlatformIO)
```bash
platformio run -e esp32 -t upload
# Uses WebServerImpl_ESP.h automatically
```

### Linux
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
./CrossPlatform_Example
```

**Same application code on all platforms!**

---

## Key Benefits

| Aspect | Before | After |
|--------|--------|-------|
| Code Duplication | ❌ 100% | ✅ 0% |
| Time to Add Platform | ⏱️ Days | ✅ Hours |
| Testing | ❌ Different tests per platform | ✅ Same tests everywhere |
| Maintenance | ❌ Bug fixes in multiple places | ✅ One fix for all |
| Learning Curve | ❌ Learn Winsock + ESPAsync + ... | ✅ Learn one interface |

---

## Migration Checklist

- [ ] Replace `#include <AsyncWebServer.h>` with `#include "IWebServer.h"`
- [ ] Replace `AsyncWebServer server(80)` with `IWebServer* server = WebServerFactory::createWebServer(80)`
- [ ] Replace `AsyncWebServerRequest*` with `const IHttpRequest&`
- [ ] Replace `AsyncWebSocketClient*` with `IWebSocketClient&`
- [ ] Update route handlers to use interface methods
- [ ] Test on target platforms
- [ ] Remove platform-specific `#ifdef` checks from handlers

---

## Platform Support Matrix

| Feature | Windows | ESP8266 | ESP32 | Linux |
|---------|---------|---------|-------|-------|
| HTTP GET/POST | ✅ | ✅ | ✅ | 🚧 |
| WebSocket | ✅ | ✅ | ✅ | 🚧 |
| Static Files | ✅ | ✅ | ✅ | 🚧 |
| JSON | ✅ | ✅ | ✅ | 🚧 |
| CORS Headers | ✅ | ✅ | ✅ | 🚧 |

---

## Examples

### Simple Status Endpoint
```cpp
server->on("/status", [](const IHttpRequest& req, IHttpResponse& res) {
    res.sendJSON(200, R"({"status":"ok","version":"1.0"})");
});
// Works on Windows, ESP32, ESP8266, Linux...
```

### Device Control WebSocket
```cpp
auto* ws = server->getWebSocket();
ws->setCallbacks({
    .onMessage = [](uint32_t id, const std::string& msg) {
        // Parse and execute command
    }
});
// Works everywhere!
```

### Full Application
See `CrossPlatform_Example.cpp` for complete example with:
- HTTP API routes
- WebSocket messaging
- Device control
- Status monitoring

---

## Getting Started

1. **Include** `IWebServer.h` in your code
2. **Create** server with `WebServerFactory::createWebServer(port)`
3. **Register** routes using interface methods
4. **Build** with appropriate CMakeLists.txt
5. **Deploy** to any platform without changes

That's it! 🎉

---

**Remember**: Your application code should **never know** which platform it's running on.
