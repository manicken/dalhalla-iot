# Dalhalla IoT — Migration Guide
## Converting Your Existing Code to Cross-Platform Architecture

---

## 📋 Overview

This guide walks you through migrating your existing code from platform-specific implementations to the unified abstraction layer.

**Time Estimate**: 2-4 hours for moderate-sized application

---

## Phase 1: Preparation (15 minutes)

### Step 1.1: Backup Your Code
```bash
git commit -m "Checkpoint before abstraction migration"
git branch abstraction-migration
```

### Step 1.2: Review Current Structure
Identify all platform-specific code:
```bash
grep -r "AsyncWebServer" --include="*.cpp" --include="*.h"
grep -r "#ifdef ESP" --include="*.cpp" --include="*.h"
grep -r "request->send" --include="*.cpp" --include="*.h"
grep -r "ws.textAll\|ws.text" --include="*.cpp" --include="*.h"
```

### Step 1.3: Document Current API Usage
List all patterns you use:
- [ ] HTTP GET handlers
- [ ] HTTP POST handlers
- [ ] WebSocket broadcast
- [ ] WebSocket targeted messages
- [ ] Static file serving
- [ ] Request parameters/headers
- [ ] Response headers

---

## Phase 2: Include Integration (30 minutes)

### Step 2.1: Copy Header Files
```bash
# Copy interface definition
cp IWebServer.h project/include/

# Copy platform implementation (choose based on target)
cp WebServerImpl_Windows.h project/include/  # For Windows
cp WebServerImpl_Windows.cpp project/src/    # For Windows
cp WebServerImpl_ESP.h project/include/      # For ESP
```

### Step 2.2: Update CMakeLists.txt
See `CMakeLists_CrossPlatform.txt` for examples.

**Replace existing web server setup with:**
```cmake
# Platform-specific implementation
if(WIN32)
    set(WEB_IMPL_SOURCES WebServerImpl_Windows.cpp)
    set(WEB_IMPL_LIBS ws2_32 mswsock advapi32)
elseif(ESP32)
    # ESPAsyncWebServer already handled
    set(WEB_IMPL_SOURCES WebServerImpl_ESP.h)
endif()

# Add to your executable
add_executable(your_app
    your_sources.cpp
    ${WEB_IMPL_SOURCES}
)

target_include_directories(your_app PRIVATE include/)
target_link_libraries(your_app PRIVATE ${WEB_IMPL_LIBS})
```

### Step 2.3: Update Include Paths
```cpp
// OLD
#ifdef ESP32
#include <ESPAsyncWebServer.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#endif

// NEW
#include "IWebServer.h"  // ← ONLY this!
```

---

## Phase 3: Handler Migration (2-3 hours)

### Step 3.1: Migrate Main Setup

**BEFORE:**
```cpp
void setup() {
#ifdef ESP32
    AsyncWebServer server(80);
    server.begin();
#elif _WIN32
    // Custom Winsock setup
#endif
}
```

**AFTER:**
```cpp
class MyApp {
private:
    IWebServer* server_;
    
public:
    void setup() {
        // Create platform-appropriate server
        server_ = WebServerFactory::createWebServer(80);
        
        // Setup routes (below)
        setupRoutes();
        
        // Start
        server_->begin();
    }
};
```

### Step 3.2: Migrate HTTP GET Handlers

**BEFORE (ESP32):**
```cpp
server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(1024);
    doc["status"] = "ok";
    doc["uptime"] = millis();
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
});
```

**AFTER (All Platforms):**
```cpp
server_->on("/api/status", [](const IHttpRequest& req, IHttpResponse& res) {
    json doc = {
        {"status", "ok"},
        {"uptime", getUptime()}
    };
    
    res.sendJSON(200, doc.dump());
});
```

### Step 3.3: Migrate HTTP POST Handlers

**BEFORE (ESP32):**
```cpp
server.on("/api/device/control", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("plain", true)) {
        request->send(400, "text/plain", "Missing body");
        return;
    }
    
    String body = request->getParam("plain", true)->value();
    
    // Parse JSON...
    request->send(200, "application/json", "{\"ok\":true}");
});
```

**AFTER (All Platforms):**
```cpp
server_->post("/api/device/control", [](const IHttpRequest& req, IHttpResponse& res) {
    if (req.body().empty()) {
        res.sendText(400, "Missing body");
        return;
    }
    
    // Parse JSON...
    res.sendJSON(200, R"({"ok":true})");
});
```

### Step 3.4: Migrate Request Parameter Access

**BEFORE:**
```cpp
// ESP
String value = request->arg("key");
String header = request->getHeader("Accept").c_str();

// Windows
// Had to parse manually
```

**AFTER (All Platforms):**
```cpp
std::string value = req.param("key");
std::string header = req.header("Accept");
```

### Step 3.5: Migrate WebSocket Setup

**BEFORE (ESP32):**
```cpp
AsyncWebSocket ws("/ws");

ws.onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, 
               AwsEventType type, void * arg, uint8_t *data, size_t len) {
    
    if(type == WS_EVT_CONNECT){
        // connect
    } else if(type == WS_EVT_DATA){
        // data
    } else if(type == WS_EVT_DISCONNECT){
        // disconnect
    }
});

server.addHandler(&ws);
```

**AFTER (All Platforms):**
```cpp
auto* ws = server_->getWebSocket();

WebSocketEventCallbacks callbacks;

callbacks.onConnect = [](IWebSocketClient& client) {
    std::cout << "Connected: " << client.id() << std::endl;
};

callbacks.onMessage = [](uint32_t clientId, const std::string& message) {
    std::cout << "Message: " << message << std::endl;
};

callbacks.onDisconnect = [](uint32_t clientId) {
    std::cout << "Disconnected: " << clientId << std::endl;
};

ws->setCallbacks(callbacks);
```

### Step 3.6: Migrate WebSocket Broadcasting

**BEFORE:**
```cpp
ws.textAll(messageStr.c_str());
```

**AFTER:**
```cpp
server_->getWebSocket()->broadcastMessage(messageStr);
```

### Step 3.7: Migrate Targeted WebSocket Messages

**BEFORE:**
```cpp
AsyncWebSocketClient* client = ws.client(clientId);
if (client && client->canSend()) {
    client->text(message);
}
```

**AFTER:**
```cpp
server_->getWebSocket()->sendToClient(clientId, message);
```

### Step 3.8: Migrate Static File Serving

**BEFORE (ESP32):**
```cpp
server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
```

**AFTER (All Platforms):**
```cpp
server_->serveStatic("/", "/www");
server_->setDefaultFile("index.html");
```

---

## Phase 4: Testing (1 hour)

### Step 4.1: Compile for Windows
```bash
mkdir build_win && cd build_win
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### Step 4.2: Test HTTP Endpoints
```bash
curl http://localhost:8080/api/status
curl -X POST http://localhost:8080/api/device/control -d '{"device":"led"}'
```

### Step 4.3: Test WebSocket
Use `WebSocket_Console.html` or:
```javascript
// Browser console
const ws = new WebSocket('ws://localhost:8080/ws');
ws.onmessage = (e) => console.log(e.data);
ws.send('test message');
```

### Step 4.4: Compile for ESP32
```bash
platformio run -e esp32 -t upload
```

### Step 4.5: Verify Identical Behavior
- [ ] HTTP endpoints respond correctly
- [ ] WebSocket messages flow both directions
- [ ] Static files serve properly
- [ ] Error handling works
- [ ] Performance is acceptable

---

## Phase 5: Cleanup (15 minutes)

### Step 5.1: Remove Platform-Specific Code
```cpp
// DELETE all these
#ifdef ESP32
#ifdef _WIN32
#ifdef ESP8266
```

### Step 5.2: Remove Old Headers
Delete or archive:
- Old Winsock2 implementations
- Old ESPAsyncWebServer wrappers
- Platform-specific main functions

### Step 5.3: Consolidate to Single Source
Move all handlers to unified implementation:
```
src/
├── main.cpp              ← Platform-independent
├── api_handlers.cpp      ← Platform-independent
├── websocket_handlers.cpp ← Platform-independent
└── platform_impl/
    ├── WebServerImpl_Windows.cpp
    └── WebServerImpl_ESP.h
```

---

## Migration Checklist

### Preparation
- [ ] Backed up code (git)
- [ ] Reviewed all platform-specific code
- [ ] Documented API usage patterns

### File Integration
- [ ] Copied `IWebServer.h` to include
- [ ] Copied platform implementations
- [ ] Updated CMakeLists.txt
- [ ] Updated include paths

### Code Migration
- [ ] Migrated server setup
- [ ] Migrated GET handlers
- [ ] Migrated POST handlers
- [ ] Migrated request access patterns
- [ ] Migrated WebSocket setup
- [ ] Migrated WebSocket broadcast
- [ ] Migrated WebSocket targeted messages
- [ ] Migrated static file serving

### Testing
- [ ] Tested on Windows
- [ ] Tested on ESP32
- [ ] Verified HTTP endpoints
- [ ] Verified WebSocket
- [ ] Verified static files
- [ ] Verified error handling

### Cleanup
- [ ] Removed platform ifdefs
- [ ] Removed old implementations
- [ ] Consolidated source structure
- [ ] Updated documentation

---

## Common Issues & Solutions

### Issue: "Include file not found: IWebServer.h"
**Solution:**
```cmake
target_include_directories(your_app PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

### Issue: "Undefined reference to WebServerFactory"
**Solution:**
Ensure platform implementation is compiled:
```cmake
add_executable(your_app
    your_code.cpp
    ${PLATFORM_IMPL_SOURCE}  ← This must be included!
)
```

### Issue: "AsyncWebSocket is not a member of..."
**Solution:**
Remove all direct references to AsyncWebSocket:
```cpp
// Don't do this:
AsyncWebSocket* ws = server_.getWS();  // ❌

// Do this instead:
IWebSocket* ws = server_->getWebSocket();  // ✅
```

### Issue: "Cannot convert from IHttpRequest to AsyncWebServerRequest"
**Solution:**
Use interface methods only:
```cpp
// Don't mix:
std::string body = req.body();           // ✅ Use interface
// String body = req->arg("plain");      // ❌ Don't use platform-specific

// Don't cast:
// auto* espReq = (AsyncWebServerRequest*)(&req);  // ❌ Never!
```

---

## Performance Migration Notes

### Memory Usage
- **ESP32**: No change (same library underneath)
- **Windows**: Expected usage ~1-50MB depending on concurrent clients

### Latency
- **HTTP**: No performance change (same libraries)
- **WebSocket**: No performance change

### Throughput
- **Message Rate**: Same as direct implementation

---

## Rollback Plan

If you need to rollback:

```bash
# Option 1: Git branch
git checkout your-previous-branch

# Option 2: Revert specific commits
git revert <commit-hash>
```

The abstraction layer doesn't modify your data structures, so rollback is safe and simple.

---

## Gradual Migration Strategy

If you can't migrate all at once:

### Phase A: Setup Infrastructure
1. Add new files (IWebServer.h, implementations)
2. Update CMakeLists.txt
3. Don't modify existing code yet

### Phase B: Parallel Implementation
1. Create new handlers using IWebServer interface
2. Keep old handlers running
3. Route old paths to old handlers, new paths to new handlers

### Phase C: Gradual Handler Migration
1. Migrate one handler at a time
2. Test each individually
3. Monitor for regressions

### Phase D: Cleanup
1. Remove old implementations
2. Verify full functionality
3. Deploy to production

---

## Success Criteria

Your migration is complete when:

- ✅ All HTTP handlers use `IHttpRequest` and `IHttpResponse`
- ✅ All WebSocket code uses `IWebSocket` interface
- ✅ Zero platform-specific `#ifdef` in application code
- ✅ Code compiles identically on Windows and ESP32
- ✅ Identical behavior on all platforms
- ✅ No performance regression
- ✅ Tests pass on all platforms

---

## Next Steps

1. Follow Phase 1-5 above
2. Refer to `CrossPlatform_Example.cpp` for patterns
3. Check `QUICK_REFERENCE.md` for API mappings
4. Review `ARCHITECTURE_GUIDE.md` for design details

---

## Support

If you get stuck:
1. Check `QUICK_REFERENCE.md` for similar patterns
2. Review `CrossPlatform_Example.cpp` for working code
3. Consult `ARCHITECTURE_GUIDE.md` for concepts

Good luck! 🚀
