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
		return newEl("div", { cls: "form-row" }, newEl("label", { cls: "form-label", text: labelText }), contentNode);
	}

	function buildBody() {
		// Network select + Scan
		dom.ssid = newEl("select", {}, newEl("option", { value: "", text: "— press Scan —" }));
		dom.scanBtn = newEl("button", { type: "button", cls: "btn btn-secondary", text: "\ud83d\udd0d Scan", onclick: () => WifiModal.scan() });
		// Password + visibility toggle
		dom.password = newEl("input", { type: "password", placeholder: "Enter password", autocomplete: "current-password" });
		dom.pwToggle = newEl("button", { type: "button", cls: "btn btn-secondary", "aria-label": "Toggle password visibility",
			text: "\ud83d\udc41\ufe0f", onclick: () => WifiModal._togglePw() });
		// Status line
		dom.status = newEl("div", { cls: "form-status" });
		return newEl("div", {},
			formRow("Network", newEl("div", { cls: "form-input-wrap" }, dom.ssid, dom.scanBtn)),
			formRow("Password", newEl("div", { cls: "form-input-wrap" }, dom.password, dom.pwToggle)),
			dom.status);
	}

	function buildFooter() {
		dom.sendBtn = newEl("button", { type: "button", cls: "btn btn-primary", text: "Send & Restart", onclick: () => WifiModal.send() });
		return newEl("div", {},
			newEl("button", { type: "button", cls: "btn btn-secondary", text: "Cancel", onclick: () => WifiModal.close() }),
			dom.sendBtn);
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
				dom.ssid.append(newEl("option", { value: jsonItem.ssid,
					text: `${ssidDecoded} (ch${jsonItem.ch}, ${jsonItem.freq}MHz, ${jsonItem.rssi}dBm, ${jsonItem.encryption})` }));
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
