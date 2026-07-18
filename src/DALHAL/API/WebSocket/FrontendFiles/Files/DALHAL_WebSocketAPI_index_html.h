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
<link rel="stylesheet" href="css/main.css">
<link rel="stylesheet" href="css/wifi-config.css">
<link rel="stylesheet" href="css/modal.css">
</head>
<body class="light">

<script src="main.js"></script>
<script src="log-helpers.js"></script>
<script src="autocomplete.js"></script>
<script src="wifi-config.js"></script>
<script src="modal.js"></script>
<script src="control-panel.js"></script>

<!-- ── Toolbar ── -->
<div id="toolbar">
  <div id="toolbar-top">
    <h1>DALHAL WebSocket Terminal</h1>
    <button id="theme-toggle" onclick="toggleTheme()">🌙 dark</button>
  </div>
  <div class="btn-row" id="extensions_toolbar"></div>
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
</body>
</html>
)rawliteral";

}