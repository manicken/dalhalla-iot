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

#include "stdint.h"
#include <pgmspace.h>

namespace DALHAL {

static const uint8_t WS_API_INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>DALHAL Terminal</title>
<link rel="stylesheet" href="main.css">
<link rel="stylesheet" href="wifi_config.css">
<style>

</style>
</head>
<body class="light">

<!-- ── WiFi Setup Modal ── -->

<div id="wifi-modal-backdrop" role="dialog" aria-modal="true" aria-labelledby="wifi-modal-title">
  <div id="wifi-modal">

    <div id="wifi-modal-header">
      <h2 id="wifi-modal-title">📶 WiFi Setup</h2>
      <button id="wifi-modal-close" aria-label="Close" onclick="WifiModal.close()">×</button>
    </div>

    <div id="wifi-modal-body">

      <!-- Network select + Scan -->
      <div class="wm-row">
        <label class="wm-label">Network</label>
        <div class="wm-scan-row">
          <select id="wm-ssid"><option value="">— press Scan —</option></select>
          <button class="wm-scan-btn" id="wm-scan-btn" onclick="WifiModal.scan()">🔍 Scan</button>
        </div>
      </div>

      <!-- Password -->
      <div class="wm-row">
        <label class="wm-label">Password</label>
        <div class="wm-pw-wrap">
          <input type="password" id="wm-password" placeholder="Enter password" autocomplete="current-password"/>
          <button class="wm-pw-toggle" id="wm-pw-toggle" type="button" aria-label="Toggle password visibility"
                  onclick="WifiModal._togglePw()">👁️</button>
        </div>
      </div>

      <!-- Status -->
      <div id="wm-status"></div>

    </div>

    <div id="wifi-modal-footer">
      <button class="wm-btn-cancel" onclick="WifiModal.close()">Cancel</button>
      <button class="wm-btn-send" id="wm-send-btn" onclick="WifiModal.send()">Send &amp; Restart</button>
    </div>

  </div>
</div>

<!-- ── Toolbar ── -->
<div id="toolbar">
  <div id="toolbar-top">
    <h1>DALHAL WebSocket Terminal</h1>
    <button id="theme-toggle" onclick="toggleTheme()">🌙 dark</button>
  </div>
  <div class="btn-row">
    <button id="open-wifi-modal-btn" onclick="WifiModal.open()">📶 WiFi Setup</button>
  </div>
  <div id="btn-container"></div>
  
</div>

<!-- ── Status bar ── -->
<div id="status-bar">
  <span id="ws-indicator" class="disconnected"></span>
  <span id="ws-status">Disconnected</span>
</div>

<!-- ── Main area ── -->
<div id="main">

  <!-- Autocomplete terminal -->
  <div id="terminal-wrap">
    <div id="terminal-label">autocomplete terminal &nbsp;·&nbsp; delimiter: /</div>
    <div id="autocomplete-output">
      <div class="line-hint">Type commands with / delimiter, e.g. hal/read/string/deviceId. Enter sends via WebSocket.</div>
    </div>
    <div id="autocomplete-area">
      <div id="suggestions" role="listbox"></div>
    </div>
    <div id="input-row">
      <span id="prompt-lbl">&gt;&nbsp;</span>
      <input id="cmd-input" type="text" autocomplete="off" spellcheck="false"
             placeholder="hal/read/…"
             aria-label="Command input" aria-autocomplete="list" aria-haspopup="listbox"/>
    </div>
    <div id="key-hint">Tab — complete &nbsp;·&nbsp; ↑↓ — select / history &nbsp;·&nbsp; Esc — dismiss &nbsp;·&nbsp; Enter — send</div>
  </div>

  <!-- Log pane -->
  <div id="log-wrap">
    <div id="log-label">
      <span>WebSocket log</span>
      <button onclick="document.getElementById('log').innerHTML=''">Clear</button>
    </div>
    <div id="log"></div>
  </div>

</div>

<script src="log_helpers.js"></script>
<script src="autocomplete.js"></script>
<script src="wifi_config.js"></script>
<script src="mail.js"></script>

</body>
</html>
)rawliteral";

}