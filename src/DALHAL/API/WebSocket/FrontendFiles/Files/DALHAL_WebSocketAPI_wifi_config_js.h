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

static const uint8_t WS_API_WIFI_CONFIG_JS[] PROGMEM = R"rawliteral(
/**
 * wifi-modal.js — WiFi setup dialog, built on the shared Modal component.
 *
 * The public API (setSendFn, receive, open, close, scan, send, _togglePw)
 * is unchanged from the original hand-rolled version — anything that
 * already calls WifiModal.* keeps working. What changed is the chrome:
 * backdrop, header, close button, Escape-to-close, and click-outside-to-close
 * all now come from Modal instead of a dedicated #wifi-modal-backdrop block
 * in the HTML. There's nothing left to paste into index.html for this modal
 * — the DOM is built here, the same way rego600-panel.js builds its panel.
 */
const WifiModal = (() => {
	let _sendFn = null;
	let _inScan = false;
	let modal = null;
	const dom = {}; // populated once, in _buildBody()/_buildFooter()

	// ── encoding helpers (unchanged) ──
	function b64urlEncode(str) {
		return btoa(unescape(encodeURIComponent(str)))
			.replace(/\+/g, "-").replace(/\//g, "_").replace(/=+$/, "");
	}
	function b64urlDecode(str) {
		str = str.replace(/-/g, "+").replace(/_/g, "/");
		while (str.length % 4) str += "=";
		return decodeURIComponent(escape(atob(str)));
	}

	function setStatus(msg, type = "") {
		dom.status.textContent = msg;
		dom.status.className = `form-status ${type}`.trim();
	}

	function send(cmd) {
		if (!_sendFn) {
			setStatus("No transport set — call WifiModal.setSendFn()", "err");
			return false;
		}
		_sendFn(cmd);
		return true;
	}

	function formRow(labelText, contentNode) {
		const row = document.createElement("div");
		row.className = "form-row";
		const label = document.createElement("label");
		label.className = "form-label";
		label.textContent = labelText;
		row.append(label, contentNode);
		return row;
	}

	function buildBody() {
		const body = document.createElement("div");

		// Network select + Scan
		const scanRow = document.createElement("div");
		scanRow.className = "form-input-wrap";
		dom.ssid = document.createElement("select");
		dom.ssid.innerHTML = '<option value="">— press Scan —</option>';
		dom.scanBtn = document.createElement("button");
		dom.scanBtn.type = "button";
		dom.scanBtn.className = "btn btn-secondary";
		dom.scanBtn.textContent = "\ud83d\udd0d Scan";
		dom.scanBtn.addEventListener("click", () => WifiModal.scan());
		scanRow.append(dom.ssid, dom.scanBtn);
		body.appendChild(formRow("Network", scanRow));

		// Password + visibility toggle
		const pwRow = document.createElement("div");
		pwRow.className = "form-input-wrap";
		dom.password = document.createElement("input");
		dom.password.type = "password";
		dom.password.placeholder = "Enter password";
		dom.password.autocomplete = "current-password";
		dom.pwToggle = document.createElement("button");
		dom.pwToggle.type = "button";
		dom.pwToggle.className = "btn btn-secondary";
		dom.pwToggle.setAttribute("aria-label", "Toggle password visibility");
		dom.pwToggle.textContent = "\ud83d\udc41\ufe0f";
		dom.pwToggle.addEventListener("click", () => WifiModal._togglePw());
		pwRow.append(dom.password, dom.pwToggle);
		body.appendChild(formRow("Password", pwRow));

		// Status line
		dom.status = document.createElement("div");
		dom.status.className = "form-status";
		body.appendChild(dom.status);

		return body;
	}

	function buildFooter() {
		const footer = document.createElement("div");

		const cancelBtn = document.createElement("button");
		cancelBtn.type = "button";
		cancelBtn.className = "btn btn-secondary";
		cancelBtn.textContent = "Cancel";
		cancelBtn.addEventListener("click", () => WifiModal.close());

		dom.sendBtn = document.createElement("button");
		dom.sendBtn.type = "button";
		dom.sendBtn.className = "btn btn-primary";
		dom.sendBtn.textContent = "Send & Restart";
		dom.sendBtn.addEventListener("click", () => WifiModal.send());

		footer.append(cancelBtn, dom.sendBtn);
		return footer;
	}

	/** Build the Modal + DOM lazily, on first open() — no markup needed in index.html. */
	function ensureModal() {
		if (modal) return modal;
		modal = new Modal({
			id: "wifi-modal",
			title: "\ud83d\udcf6 WiFi Setup",
			width: 360,
			backdrop: true, // dimmed overlay, click-outside + Escape close it
			draggable: false, // it's a one-shot settings dialog, not a tool window
		});
		modal.setBody(buildBody());
		modal.setFooter(buildFooter());
		modal.mount();
		return modal;
	}

	// ── public API (unchanged shape) ──
	return {
		/** Register the transport. fn(cmd: string) — do NOT add \n, modal handles it if needed */
		setSendFn(fn) {
			_sendFn = fn;
		},

		/** Feed scan results into the modal (call this from your rx handler) */
		receive(jsonData) {
			ensureModal();
			for (let i = 0; i < jsonData.length; i++) {
				const jsonItem = jsonData[i];
				let ssidDecoded;
				try { ssidDecoded = b64urlDecode(jsonItem.ssid); } catch { ssidDecoded = jsonItem.ssid; }
				const opt = document.createElement("option");
				opt.value = jsonItem.ssid;
				opt.textContent = `${ssidDecoded} (ch${jsonItem.ch}, ${jsonItem.freq}MHz, ${jsonItem.rssi}dBm, ${jsonItem.encryption})`;
				dom.ssid.appendChild(opt);
			}
		},

		open() {
			ensureModal();
			dom.password.value = "";
			setStatus("");
			dom.sendBtn.disabled = false;
			dom.scanBtn.disabled = false;
			modal.open();
			dom.password.focus();
		},

		close() {
			modal?.close();
			_inScan = false;
		},

		scan() {
			setStatus("Sending scan request…", "info");
			send("wifi/scan");
		},

		send() {
			const ssidB64 = dom.ssid.value;
			if (!ssidB64) {
				setStatus("Select a network first.", "err");
				return;
			}
			const pass = dom.password.value;
			const passB64 = b64urlEncode(pass);
			const cmd = `wifi/set_b64/${ssidB64}:${passB64}`;
			if (send(cmd)) {
				setStatus("Config sent — device will restart.", "ok");
				dom.sendBtn.disabled = true;
			}
		},

		_togglePw() {
			const isHidden = dom.password.type === "password";
			dom.password.type = isHidden ? "text" : "password";
			dom.pwToggle.textContent = isHidden ? "\ud83d\ude48" : "\ud83d\udc41\ufe0f";
		},
	};
})();

// Backdrop click-to-close and Escape-to-close are now handled by Modal itself
// (backdrop: true, closeOnEscape defaults to true when backdrop is true) —
// nothing to wire up here.

WifiModal.setSendFn((cmd) => ws.send(cmd)); // WebSocket transport
customParsers.push((tag, text) => {
    if (tag.startsWith('wifi')) {
      WifiModal.receive(JSON.parse(text));
      return true;
    }
    return false;
});
registerToolbarButton(
    "📶 WiFi Setup",
    () => WifiModal.open(),
    "open-wifi-modal-btn"
);
)rawliteral";

}