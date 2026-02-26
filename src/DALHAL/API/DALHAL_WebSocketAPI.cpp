/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "DALHAL_WebSocketAPI.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

//#include "../Support/DALHAL_Logger.h"
//#include "../Core/Types/DALHAL_ZeroCopyString.h"
//#include "../Core/DALHAL_Manager.h"
#include "DALHAL_CommandExecutor.h"

namespace DALHAL {

    AsyncWebServer* WebSocketAPI::asyncWebserver = nullptr;
    AsyncWebSocket* WebSocketAPI::asyncWebSocket = nullptr;

    static const char HTML_WS_CONSOLE[] = R"rawliteral(
<!DOCTYPE html>
<html>
  <style>
  body { font-family: 'Courier New', monospace; margin: 1em; }
  input, button { font-family: 'Courier New', monospace; margin: 0.5em 0; display: block; width: 200px; padding: 0.5em; }
  pre { font-family: 'Courier New', monospace; background: #f0f0f0; padding: 1em; margin-top: 1em; flex: 1; overflow-y: auto; }
</style>
<body style="margin:3; height:100vh; display:flex; flex-direction:column;">
<div style="flex:0 0 auto;">
  <button onclick="ws.send('hal/reloadcfg');">reload cfg</button>
  <button onclick="ws.send('help');">help</button>
  <div style="display:flex; flex-direction:row; gap: 0.5em;">
    <button onclick="ws.send('hal/printDevices');">Print Devices</button>
    <button onclick="ws.send('hal/getAvailableGPIOs');">get Available GPIOs</button>
    <button onclick="ws.send('hal/printlog');">print log</button>
  </div>
  <div style="display:flex; flex-direction:row; gap: 0.5em;">
    <button onclick="ws.send('hal/scripts/reload');">scripts reload</button>
    <button onclick="ws.send('hal/scripts/stop');">scripts stop</button>
    <button onclick="ws.send('hal/scripts/start');">scripts start</button>
  </div>
  <div style="margin-top:1em;">
    <input type="text" id="cmd" placeholder="Enter command" style="width:70%;" />
    <button onclick="sendCmd()">Send</button>
</div>
 </div>
<pre id="log" style="margin:0; margin-top:1em; flex:1 1 auto; overflow-y:auto; background:#f0f0f0; padding:1em; "></pre>
<script>
let location_host = location.host;
if (!location_host.endsWith(":82")) location_host += ":82";

let ws;
let reconnectInterval = 2000; // 2 seconds

function connect() {
    ws = new WebSocket(`ws://${location_host}/ws`);

    ws.onopen = () => {
        console.log("WebSocket connected");
        document.getElementById('log').textContent += "Connected\n";
    };

    ws.onmessage = (evt) => {
        document.getElementById('log').textContent += evt.data + '\n';
    };

    ws.onclose = () => {
        console.log("WebSocket disconnected, retrying in", reconnectInterval, "ms");
        document.getElementById('log').textContent += "Disconnected, reconnecting...\n";
        setTimeout(connect, reconnectInterval);
    };

    ws.onerror = (err) => {
        console.error("WebSocket error", err);
        ws.close(); // triggers onclose
    };
}

function sendCmd() {
    const cmd = document.getElementById('cmd').value;
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(cmd);
    } else {
        console.log("WebSocket not connected, retrying...");
        document.getElementById('log').textContent += "Not connected, retrying...\n";
        setTimeout(sendCmd, reconnectInterval);
    }
}

// Start connection
connect();

// Handle Enter key
document.getElementById('cmd').addEventListener('keydown', (e) => {
    if (e.key === "Enter") sendCmd();
});
</script>
</body>
</html>
)rawliteral";


    void WebSocketAPI::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
    {
        //Serial.println("onWsEvent");

        switch (type) {

        case WS_EVT_CONNECT:
            Serial.printf("WS client #%u connected from %s\r\n",
                        client->id(),
                        client->remoteIP().toString().c_str());
            break;

        case WS_EVT_DISCONNECT:
            Serial.printf("WS client #%u disconnected\r\n", client->id());
            break;

        case WS_EVT_DATA: {
            AwsFrameInfo *info = (AwsFrameInfo*)arg;

            if (!info->final || info->index != 0 || info->len != len) {
                Serial.println("WS EVT_DATA rx error");
                return;
            }
            if (info->opcode != WS_TEXT) {
                Serial.println("info->opcode != WS_TEXT");
                return;
            }

            std::string cmd((char*)data, len);
            //Serial.printf("WS RX: %s\n", cmd.c_str());

            //client->text("ACK");
            uint32_t clientId = client->id();
            CommandExecutor_LOCK_QUEUE();
            CommandExecutor::g_pending.push({
                cmd,
                [clientId](const std::string& body) {
                    
                    AsyncWebSocketClient* c = asyncWebSocket->client(clientId);

                    if (!c) return;                 // client gone
                    if (!c->canSend()) return;      // TCP buffer full / closing

                    c->text(body.c_str());
                }
            });
            CommandExecutor_UNLOCK_QUEUE();

            break;
        }

        case WS_EVT_ERROR:
            Serial.println("WS error");
            break;

        case WS_EVT_PONG:
            Serial.println("WS_EVT_PONG");
            break;

        default:
            Serial.println("WS unknown type");
            break;
        }
    }

    void WebSocketAPI::GetRootPage_Handler(AsyncWebServerRequest* request) {
        request->send(200, "text/html", HTML_WS_CONSOLE);
    }

    void WebSocketAPI::setup() {
        asyncWebserver = new AsyncWebServer(82);
        asyncWebSocket = new AsyncWebSocket("/ws");
        asyncWebSocket->onEvent(onWsEvent);
        asyncWebserver->addHandler(asyncWebSocket);
        asyncWebserver->on("/", GetRootPage_Handler);
        asyncWebserver->begin();
        asyncWebSocket->enable(true);
    }

    void WebSocketAPI::SendMessage(std::string &msg) {
        if (asyncWebSocket->availableForWriteAll()) {
            asyncWebSocket->textAll(msg.c_str());
        }
    }
    void WebSocketAPI::SendMessage(const char* msg) {
        if (asyncWebSocket->availableForWriteAll()) {
            asyncWebSocket->textAll(msg);
        }
    }
    void WebSocketAPI::SendMessage(const char* source, const char* msg) {
        if (asyncWebSocket->availableForWriteAll()) {
            std::string msgStr;
            msgStr.reserve(strlen(source) + strlen(msg) + 2);
            msgStr.append(source);
            msgStr.append(msg);
            asyncWebSocket->textAll(msg);
        }
    }
/*
    void WebSocketAPI::SendMessage(const char* fmt, ...) {
        if (!asyncWebSocket) return;

        va_list args;
        va_start(args, fmt);

        if (asyncWebSocket->availableForWriteAll()) {
            asyncWebSocket->printfAll(fmt, args);  // forward var args
        }

        va_end(args);
    }*/
}
