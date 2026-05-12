/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2026 Jannik Svensson

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

#pragma once

namespace DALHAL {

static const char HTML_WS_CONSOLE[] PROGMEM = R"rawliteral(
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
    <button onclick="ws.send('hal/printRegistry');">Print Device Registry</button>
    <button onclick="ws.send('hal/printJsonSchemas');">Print Json Schemas</button>
    <button onclick="ws.send('hal/getAvailableGPIOs');">get Available GPIOs</button>
  </div>
  <div style="display:flex; flex-direction:row; gap: 0.5em;">
    <button onclick="ws.send('hal/printDevices');">Print Devices</button>
    <button onclick="ws.send('hal/printlog');">print log</button>
    <button onclick="ws.send('system/info');">get Info</button>
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
 <div style="display:flex; justify-content:flex-end; margin-bottom:0.5em;">
    <button onclick="document.getElementById('log').textContent='';">Clear log</button>
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
        const log = document.getElementById('log');

        let jsonData;
        try {
            jsonData = JSON.parse(evt.data);
            // JSON detected — make it collapsible
            const details = document.createElement('details');
            const summary = document.createElement('summary');
            const oneLine = JSON.stringify(jsonData);
            summary.textContent = oneLine.length > 80 ? oneLine.substr(0, 77) + "  ..." : oneLine;
            details.appendChild(summary);

            const pre = document.createElement('pre');
            pre.textContent = JSON.stringify(jsonData, null, 2); // nicely indented
            details.appendChild(pre);

            log.appendChild(details);
        } catch(e) {
            // Not JSON, just print normally
            const div = document.createElement('div');
            div.style.whiteSpace = "pre-wrap";  // preserves newlines
            div.style.fontFamily = "monospace"; // optional, for readability
            div.textContent = evt.data;
            log.appendChild(div);
        }
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

}