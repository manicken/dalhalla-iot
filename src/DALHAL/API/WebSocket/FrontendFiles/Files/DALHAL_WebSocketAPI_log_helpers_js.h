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

static const uint8_t WS_API_LOG_HELPERS_JS[] PROGMEM = R"rawliteral(
// ── Log helpers ───────────────────────────────────────────────────────────────
const log = document.getElementById('log');

function logText(text) {
  const div = document.createElement('div');
  div.style.whiteSpace = 'pre-wrap';
  div.textContent = text;
  log.appendChild(div);
  log.scrollTop = log.scrollHeight;
}

function logCollapsible(jsonData, titleStr) {
  const details = document.createElement("details");

  const summary = document.createElement("summary");
  const oneLine = JSON.stringify(jsonData);

  summary.textContent =
    `${titleStr ?? ""} ` +
    (oneLine.length > 150 ? oneLine.slice(0, 150-3) + "…" : oneLine);

  const pre = document.createElement("pre");
  pre.textContent = JSON.stringify(jsonData, null, 2);

  details.appendChild(summary);
  details.appendChild(pre);

  pre.addEventListener("mousedown", (e) => {
    if (e.ctrlKey == false) { return;}
    e.preventDefault(); // stops native selection behavior
    const range = document.createRange();
    range.selectNodeContents(pre);
    const sel = window.getSelection();
    sel.removeAllRanges();
    sel.addRange(range);
  });

  log.appendChild(details);
  log.scrollTop = log.scrollHeight;
}
)rawliteral";

}