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

static const uint8_t WS_API_WIFI_CONFIG_CSS[] PROGMEM = R"rawliteral(

/* Backdrop */
#wifi-modal-backdrop {
  display: none;
  position: fixed;
  inset: 0;
  background: rgba(0,0,0,.45);
  z-index: 1000;
  align-items: center;
  justify-content: center;
}
#wifi-modal-backdrop.open { display: flex; }

/* Dialog box */
#wifi-modal {
  background: #fff;
  border-radius: 10px;
  box-shadow: 0 8px 32px rgba(0,0,0,.22);
  width: 420px;
  max-width: 95vw;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  font-family: sans-serif;
  font-size: 14px;
}

/* Header */
#wifi-modal-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 14px 18px 12px;
  border-bottom: 1px solid #e0e0e0;
}
#wifi-modal-header h2 { font-size: 15px; font-weight: 600; color: #1a1a1a; }
#wifi-modal-close {
  background: none; border: none; font-size: 20px;
  cursor: pointer; color: #888; line-height: 1;
}
#wifi-modal-close:hover { color: #333; }

/* Body */
#wifi-modal-body { padding: 16px 18px; display: flex; flex-direction: column; gap: 12px; }

.wm-row { display: flex; flex-direction: column; gap: 4px; }
.wm-label { font-size: 12px; color: #555; font-weight: 500; }

.wm-row select,
.wm-row input[type="password"],
.wm-row input[type="text"] {
  width: 100%;
  padding: 8px 10px;
  border: 1px solid #ccc;
  border-radius: 6px;
  font-size: 13px;
  font-family: inherit;
  background: #fafafa;
  color: #1a1a1a;
}
.wm-row select:focus,
.wm-row input:focus { outline: 2px solid #4a90d9; border-color: transparent; background: #fff; }

/* Password row with toggle */
.wm-pw-wrap { position: relative; }
.wm-pw-wrap input { padding-right: 38px; }
.wm-pw-toggle {
  position: absolute;
  right: 8px; top: 50%;
  transform: translateY(-50%);
  background: none; border: none;
  font-size: 16px; cursor: pointer;
  line-height: 1;
}

/* Scan button inline with select */
.wm-scan-row { display: flex; gap: 8px; align-items: flex-end; }
.wm-scan-row select { flex: 1; }
.wm-scan-btn {
  flex-shrink: 0;
  padding: 8px 14px;
  font-size: 13px;
  border: 1px solid #bbb;
  border-radius: 6px;
  cursor: pointer;
  background: #f0f0f0;
  white-space: nowrap;
}
.wm-scan-btn:hover { background: #e0e0e0; }
.wm-scan-btn:disabled { opacity: .5; cursor: default; }

/* Status line */
#wm-status {
  font-size: 12px;
  min-height: 18px;
  color: #555;
  padding: 0 2px;
}
#wm-status.err  { color: #c0392b; }
#wm-status.ok   { color: #1a7a4a; }
#wm-status.info { color: #1a5fa0; }

/* Footer */
#wifi-modal-footer {
  padding: 12px 18px 16px;
  border-top: 1px solid #e0e0e0;
  display: flex;
  justify-content: flex-end;
  gap: 8px;
}
.wm-btn-cancel {
  padding: 8px 16px;
  font-size: 13px;
  border: 1px solid #bbb;
  border-radius: 6px;
  cursor: pointer;
  background: #f4f4f4;
}
.wm-btn-cancel:hover { background: #e8e8e8; }
.wm-btn-send {
  padding: 8px 18px;
  font-size: 13px;
  border: none;
  border-radius: 6px;
  cursor: pointer;
  background: #1a5fa0;
  color: #fff;
  font-weight: 500;
}
.wm-btn-send:hover { background: #154d84; }
.wm-btn-send:disabled { opacity: .5; cursor: default; }
)rawliteral";

}