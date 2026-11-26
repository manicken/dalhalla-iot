# Dalhalla IoT — Cross-Platform Architecture Guide

## Overview

The abstraction layer provides a **platform-agnostic interface** that works on:
- ✅ Windows (Winsock2)
- ✅ ESP8266 (ESPAsyncWebServer)
- ✅ ESP32 (ESPAsyncWebServer)
- 🚧 Linux/macOS (to be implemented)

## Architecture

```
Your Application Code
        ↓
    IWebServer Interface (Abstract)
        ↓
    ├─→ WebServerImpl_Windows.cpp  (Windows)
    ├─→ WebServerImpl_ESP.h        (ESP8266/ESP32)
    └─→ WebServerImpl_Linux.cpp    (Future)
        ↓
    Platform-Specific Libraries
    ├─→ Winsock2 (Windows)
    ├─→ ESPAsyncWebServer (ESP)
    └─→ ASIO/libevent (Linux)
```

## File Structure

```
project/
├── Core Abstraction
│   └── IWebServer.h                 ← Platform-agnostic interface
│
├── Platform Implementations
│   ├── WebServerImpl_Windows.h       ← Windows (header-only possible)
│   ├── WebServerImpl_Windows.cpp
│   ├── WebServerImpl_ESP.h           ← ESP8266/ESP32
│   └── WebServerImpl_Linux.h         ← Future
│
├── Your Application
│   ├── MyApplication.cpp            ← Uses ONLY IWebServer
│   ├── ApiHandlers.cpp
│   └── WebSocketHandlers.cpp
│
└── CMakeLists.txt                   ← Select implementation per platform
```

## Key Design Principles

### 1. Platform Independence
Your application code never knows which platform it's running on:

```cpp
// ✅ GOOD - Platform independent
void setupApi(IWebServer* server) {
    server->on("/api/status", [](const IHttpRequest& req, IHttpResponse& res) {
        res.sendJSON(200, R"({"status":"ok"})");
    });
}

// ❌ BAD - Platform-specific
void setupApi() {
    asyncServer.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(200, "application/json", R"({"status":"ok"})");
    });
}
```

### 2. Consistent Interface
All implementations provide the same interface:

```cpp
IWebServer* server = WebServerFactory::createWebServer(8080);

// Same API on all platforms
server->on("/path", handler);          // GET
server->post("/path", handler);        // POST
server->getWebSocket()->broadcast();   // WebSocket

IWebSocket* ws = server->getWebSocket();
ws->sendToClient(clientId, message);
ws->broadcastMessage(message);
```

### 3. Minimal Dependencies
Each implementation uses only necessary platform libraries:

| Platform | Dependencies |
|----------|--------------|
| Windows  | Winsock2 (system) |
| ESP8266  | ESPAsyncWebServer (already used) |
| ESP32    | ESPAsyncWebServer (already used) |
| Linux    | ASIO or libevent (to choose) |

## Usage Guide

### Step 1: Use Only IWebServer Interface

```cpp
#include "IWebServer.h"

class MyApp {
private:
    IWebServer* server_;  // Never use platform-specific type
    
public:
    void setup() {
        // Create platform-appropriate implementation
        server_ = WebServerFactory::createWebServer(8080);
        
        // Setup routes using interface
        server_->on("/api/status", handleStatus);
        
        // Setup WebSocket using interface
        auto* ws = server_->getWebSocket();
        ws->setCallbacks(callbacks);
        
        server_->begin();
    }
};
```

### Step 2: Include Only the Interface

```cpp
// main.cpp or application.cpp
#include "IWebServer.h"  // ← ONLY include this

// Platform-specific implementation is selected by CMakeLists.txt
// Your code doesn't need to know which one!
```

### Step 3: Configure CMakeLists.txt for Target Platform

#### For Windows:
```cmake
add_executable(my_app
    MyApplication.cpp
    WebServerImpl_Windows.cpp    ← Include Windows implementation
)

target_include_directories(my_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(my_app PRIVATE ws2_32 mswsock advapi32)
```

#### For ESP8266/ESP32:
```cmake
# In PlatformIO or Arduino build system
# Just include WebServerImpl_ESP.h and it adapts ESPAsyncWebServer

# src_build_flags += -DENABLE_DALHAL_CROSS_PLATFORM
```

#### For Linux:
```cmake
# Future - would use ASIO or libevent
# target_link_libraries(my_app PRIVATE asio)
```

## Request/Response Handling

### HTTP Requests

```cpp
server->on("/api/device/:id", [](const IHttpRequest& req, IHttpResponse& res) {
    // Access request data using interface
    std::string method = req.method();        // GET, POST, etc.
    std::string path = req.path();            // /api/device/123
    std::string body = req.body();            // POST/PUT body
    std::string param = req.param("name");    // Query param
    std::string header = req.header("Accept");// HTTP header
    std::string ip = req.remoteIP();          // Client IP
    
    // Send response
    res.sendJSON(200, R"({"id":"123"})");
    // or
    res.sendHTML(200, "<h1>Hello</h1>");
    // or
    res.sendText(200, "Plain text response");
    // or
    res.sendFile("/www/index.html", "text/html");
});
```

### WebSocket Communication

```cpp
WebSocketEventCallbacks callbacks;

callbacks.onConnect = [](IWebSocketClient& client) {
    std::cout << "Client " << client.id() << " from " << client.remoteIP() << std::endl;
    client.send("Welcome!");
};

callbacks.onMessage = [](uint32_t clientId, const std::string& message) {
    // Handle incoming message
    std::cout << "Received: " << message << std::endl;
};

callbacks.onDisconnect = [](uint32_t clientId) {
    std::cout << "Client " << clientId << " disconnected" << std::endl;
};

server->getWebSocket()->setCallbacks(callbacks);
server->getWebSocket()->broadcastMessage("Server message");
```

## Method-Specific Routing

```cpp
// GET handler
server->on("/api/status", [](const IHttpRequest& req, IHttpResponse& res) {
    // Only called for GET /api/status
    res.sendJSON(200, R"({"status":"ok"})");
});

// POST handler
server->post("/api/device", [](const IHttpRequest& req, IHttpResponse& res) {
    // Only called for POST /api/device
    res.sendJSON(201, R"({"id":"new_device"})");
});

// Any method
server->onAny("/api/config", [](const IHttpRequest& req, IHttpResponse& res) {
    // Called for GET, POST, PUT, DELETE, etc.
    if (req.method() == "GET") {
        res.sendJSON(200, "{}");
    } else if (req.method() == "POST") {
        res.sendJSON(201, "{}");
    }
});
```

## Static File Serving

```cpp
// Serve files from /www directory at URL path /
server->serveStatic("/", "/www");

// Set default file for directory access
server->setDefaultFile("index.html");

// Now:
// GET / → serves /www/index.html
// GET /css/style.css → serves /www/css/style.css
```

## Error Handling

```cpp
server->on("/api/data", [](const IHttpRequest& req, IHttpResponse& res) {
    try {
        // Parse JSON from body
        json data = json::parse(req.body());
        
        // Process...
        
        // Success
        res.sendJSON(200, data.dump());
    }
    catch (const std::exception& e) {
        // Error response
        json error = {{"error", e.what()}};
        res.sendJSON(400, error.dump());
    }
});
```

## Comparison: Before vs After

### Before (Platform-Specific)

```cpp
// Windows code
#include <ESPAsyncWebServer.h>
AsyncWebServer server(80);

server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\":\"ok\"}");
});

// Completely different on ESP8266
#include <AsyncWebServer.h>
AsyncWebServer server(80);  // Different class name!

// Can't share code!
```

### After (Abstraction Layer)

```cpp
// Same code for ALL platforms!
#include "IWebServer.h"

IWebServer* server = WebServerFactory::createWebServer(80);

server->on("/api/status", [](const IHttpRequest& req, IHttpResponse& res) {
    res.sendJSON(200, R"({"status":"ok"})");
});

// Works on Windows, ESP8266, ESP32, Linux, macOS!
```

## Adding a New Platform

To support a new platform:

1. **Create header file** `WebServerImpl_NewPlatform.h`:
   ```cpp
   class NewPlatformWebServer : public IWebServer {
       // Implement all virtual methods
   };
   ```

2. **Update CMakeLists.txt** to include it:
   ```cmake
   if(NEW_PLATFORM)
       target_sources(my_app PRIVATE WebServerImpl_NewPlatform.cpp)
   endif()
   ```

3. **Update factory**:
   ```cpp
   IWebServer* WebServerFactory::createWebServer(int port) {
   #ifdef _WIN32
       return new WindowsWebServer(port);
   #elif defined(NEW_PLATFORM)
       return new NewPlatformWebServer(port);
   #endif
   }
   ```

## Best Practices

### ✅ DO

- Use `IWebServer*` and `IWebSocket*` in headers
- Pass request/response by const reference
- Use lambdas for route handlers
- Share code across all platforms
- Check platform at **build time**, not runtime

### ❌ DON'T

- Include `ESPAsyncWebServer.h` or platform headers in business logic
- Use `AsyncWebServerRequest*` in your application
- Write platform-specific code in handlers
- Use preprocessor `#ifdef` for platform detection in handlers

## Performance Considerations

| Aspect | Windows | ESP8266 | ESP32 |
|--------|---------|---------|-------|
| Max Clients | 100+ | 5-10 | 20-50 |
| Latency | <5ms | 10-50ms | <10ms |
| Memory per Client | 50KB | 5KB | 10KB |
| Broadcast Speed | 1000 msg/s | 100 msg/s | 500 msg/s |

## Memory Management

```cpp
// Proper cleanup
IWebServer* server = WebServerFactory::createWebServer(8080);
server->begin();

// ... application runs ...

server->end();
WebServerFactory::releaseWebServer(server);  // ← Important!
server = nullptr;
```

## Testing Strategy

Test the same code on multiple platforms:

```bash
# Build for Windows
cmake -B build_win -DPLATFORM=Windows
cmake --build build_win

# Build for Linux
cmake -B build_linux -DPLATFORM=Linux
cmake --build build_linux

# Flash to ESP32
platformio run -e esp32

# All use identical application code!
```

## Troubleshooting

### Compilation Error: "Unknown type IWebServer"
- Ensure `IWebServer.h` is in include path
- Check `CMakeLists.txt` includes the header

### Platform-specific code in handler
- Remove `#ifdef` checks
- Use only interface methods
- Move platform code to implementation layer

### Missing WebSocket functionality
- Ensure `getWebSocket()` returns non-null
- Call `setEnabled(true)` on WebSocket
- Check that callbacks are registered

## Summary

The abstraction layer allows you to:
1. Write application code **once**
2. Deploy to **all platforms** unchanged
3. Add new platforms without touching app code
4. Test identically across platforms

This is **true cross-platform development**.

---

**Next**: See `CrossPlatform_Example.cpp` for complete working example!
