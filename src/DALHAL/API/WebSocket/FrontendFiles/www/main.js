// ── Theme ─────────────────────────────────────────────────────────────────────
let isDark = false;
function toggleTheme() {
  isDark = !isDark;
  document.body.className = isDark ? 'dark' : 'light';
  document.getElementById('theme-toggle').textContent = isDark ? '☀️ light' : '🌙 dark';
}

// ── WebSocket ─────────────────────────────────────────────────────────────────
let ws;
const RECONNECT_MS = 2000;
let chunks = [];
const decoder = new TextDecoder('utf-8');
const customParsers = [];

// ── wsRequest: a command whose answer is wanted by code (e.g. a tab), not shown in the console log ──
// The device answers only the client that asked, every chunked answer has a tag (BlockStreamer),
// an unknown/failed command answers {"error":..,"cmd":..} as one text frame.
const wsWaiters = []; // { cmd, tag, resolve, reject, timer }

/** sends cmd, resolves with the text of the next chunked answer with the given tag */
function wsRequest(cmd, tag, timeoutMs = 15000) {
  return new Promise((resolve, reject) => {
    if (!ws || ws.readyState !== WebSocket.OPEN) { reject(new Error('WebSocket not connected')); return; }
    const w = { cmd, tag, resolve, reject };
    w.timer = setTimeout(() => {
      wsWaiters.splice(wsWaiters.indexOf(w), 1);
      reject(new Error(`no answer to ${cmd}`));
    }, timeoutMs);
    wsWaiters.push(w);
    ws.send(cmd);
  });
}

/** true when a waiter took this answer */
function wsDeliver(match, settle) {
  const i = wsWaiters.findIndex(match);
  if (i < 0) return false;
  const w = wsWaiters.splice(i, 1)[0];
  clearTimeout(w.timer);
  settle(w);
  return true;
}

// same server as the page: port 80 on ESP-IDF, port 82 on the Arduino build (until it also moves to port 80)
const location_host = location.host;

function setWsStatus(connected, text) {
  const dot = document.getElementById('ws-indicator');
  dot.className = connected ? 'connected' : 'disconnected';
  document.getElementById('ws-status').textContent = text;
}

function connect() {
  ws = new WebSocket(`ws://${location_host}/ws`);
  ws.binaryType = 'arraybuffer';

  ws.onopen = () => {
    setWsStatus(true, `Connected to ws://${location_host}/ws`);
    logText('Connected\n');
    wsSend('help');
  };

  ws.onmessage = (evt) => {
    if (typeof evt.data === 'string') {
      let jsonData;
      try {
        jsonData = JSON.parse(evt.data);
        if (jsonData.type === 'start_chunked') {
          chunks = [];
        } else if (jsonData.type === 'end_chunked') {
          const total = chunks.reduce((s, c) => s + c.length, 0);
          const merged = new Uint8Array(total);
          let offset = 0;
          for (const c of chunks) { merged.set(c, offset); offset += c.length; }
          const text = decoder.decode(merged);
          if (wsDeliver((w) => w.tag === jsonData.tag, (w) => w.resolve(text))) return;
          if (jsonData.dataType === 'json') {
            try { 
              let parsedJson = JSON.parse(text);
              if (jsonData.tag == 'help') {
                helpTree = JSON.parse(text); // create copy

                renderAutoButtons(helpTree, document.getElementById('btn-container'), wsSend);
              } else {
                for (let i=0;i<customParsers.length;i++) {
                   if (customParsers[i](jsonData.tag, text)) {
                    break;
                   }
                }
              }
              logCollapsible(parsedJson, jsonData.tag);
            } catch(e) { console.log(e); logText('(chunk parse error)\n' + text); }
          } else {
            logText(text);
          }
        } else {
          if (jsonData.error && jsonData.cmd &&
              wsDeliver((w) => w.cmd === jsonData.cmd, (w) => w.reject(new Error(`${jsonData.error}: ${jsonData.cmd}`)))) return;
          logCollapsible(jsonData);
        }
      } catch(e) {
        logText(evt.data);
      }
    } else {
      chunks.push(new Uint8Array(evt.data));
    }
  };

  ws.onclose = () => {
    for (const w of wsWaiters.splice(0)) { clearTimeout(w.timer); w.reject(new Error('WebSocket closed')); }
    setWsStatus(false, `Disconnected — reconnecting in ${RECONNECT_MS/1000}s…`);
    logText('Disconnected, reconnecting…\n');
    setTimeout(connect, RECONNECT_MS);
  };

  ws.onerror = () => ws.close();
}

function wsSend(cmd) {
  const trimmed = cmd.trim().replace(/\/$/, '');
  if (!trimmed) return;
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(trimmed);
    termPrintLine('> ' + trimmed, 'line-prompt');
  } else {
    termPrintLine('Not connected', 'line-err');
  }
}



function collectRows(node, pathParts) {
  const rows = [];
  const siblings = [];
  for (const child of (node.children || [])) {
    const childPath = pathParts.concat(child.name);
    if (child.autogenbutton) {
      siblings.push({
        path: childPath.join('/'),
        name: child.name,
        danger: child.autogendanger,
        parent: pathParts[pathParts.length - 1] || null
      });
    }
    rows.push(...collectRows(child, childPath));
  }
  if (siblings.length > 0) {
    rows.unshift({ groupParent: pathParts[pathParts.length - 1] || 'root', buttons: siblings });
  }
  return rows;
}

function renderAutoButtons(tree, container, wsSend) {
  container.innerHTML = '';
  const rows = collectRows(tree.root, []);
  for (const row of rows) {
    container.append(newEl('div', { cls: 'btn-row' }, ...row.buttons.map((btn) =>
      newEl('button', {
        cls: 'cmd-btn' + (btn.danger ? ' danger' : ''),
        text: btn.parent ? `${btn.parent}/${btn.name}` : btn.name,
        title: btn.path, // full path as tooltip
        onclick: () => wsSend(btn.path),
      }))));
  }
}

const ext_toolbar = document.getElementById("extensions_toolbar");

function registerToolbarButton(text, onclick, id = "") {
    const btn = newEl('button', { text, id: id || null, onclick });
    ext_toolbar.append(btn);
    return btn;
}

document.addEventListener("DOMContentLoaded", () => {

  connect();
});

